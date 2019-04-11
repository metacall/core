/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>

#include <log/log.h>

class metacall_fork_test : public testing::Test
{
public:
};

#if defined(WIN32) || defined(_WIN32) || \
		defined(__CYGWIN__) || defined(__CYGWIN32__) || \
		defined(__MINGW32__) || defined(__MINGW64__)

#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define RTL_CLONE_PROCESS_FLAGS_CREATE_SUSPENDED 0x00000001
#define RTL_CLONE_PROCESS_FLAGS_INHERIT_HANDLES 0x00000002
#define RTL_CLONE_PROCESS_FLAGS_NO_SYNCHRONIZE 0x00000004

#define RTL_CLONE_PARENT 0
#define RTL_CLONE_CHILD 297

typedef long NTSTATUS;

typedef struct _CLIENT_ID {
	PVOID UniqueProcess;
	PVOID UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _SECTION_IMAGE_INFORMATION {
	PVOID EntryPoint;
	ULONG StackZeroBits;
	ULONG StackReserved;
	ULONG StackCommit;
	ULONG ImageSubsystem;
	WORD SubSystemVersionLow;
	WORD SubSystemVersionHigh;
	ULONG Unknown1;
	ULONG ImageCharacteristics;
	ULONG ImageMachineType;
	ULONG Unknown2[3];
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;

typedef struct _RTL_USER_PROCESS_INFORMATION {
	ULONG Size;
	HANDLE Process;
	HANDLE Thread;
	CLIENT_ID ClientId;
	SECTION_IMAGE_INFORMATION ImageInformation;
} RTL_USER_PROCESS_INFORMATION, *PRTL_USER_PROCESS_INFORMATION;

typedef NTSTATUS(NTAPI * RtlCloneUserProcessPtr)(ULONG ProcessFlags,
	PSECURITY_DESCRIPTOR ProcessSecurityDescriptor,
	PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
	HANDLE DebugPort,
	PRTL_USER_PROCESS_INFORMATION ProcessInformation);

typedef long pid_t;

pid_t fork(void);

pid_t fork()
{
	HMODULE module;
	RtlCloneUserProcessPtr clone_ptr;
	RTL_USER_PROCESS_INFORMATION process_info;
	NTSTATUS result;

	module = GetModuleHandle("ntdll.dll");

	if (!module)
	{
		return -ENOSYS;
	}

	clone_ptr = (RtlCloneUserProcessPtr)GetProcAddress(module, "RtlCloneUserProcess");

	if (clone_ptr == NULL)
	{
		return -ENOSYS;
	}

	result = clone_ptr(RTL_CLONE_PROCESS_FLAGS_CREATE_SUSPENDED | RTL_CLONE_PROCESS_FLAGS_INHERIT_HANDLES, NULL, NULL, NULL, &process_info);

	if (result == RTL_CLONE_PARENT)
	{
		HANDLE me = GetCurrentProcess();
		pid_t child_pid;

		child_pid = GetProcessId(process_info.Process);

		ResumeThread(process_info.Thread);
		CloseHandle(process_info.Process);
		CloseHandle(process_info.Thread);

		return child_pid;
	}
	else if (result == RTL_CLONE_CHILD)
	{
		/* fix stdio */
		AllocConsole();
		return 0;
	}

	return -1;
}

#endif

int callback_test(metacall_pid pid, void * ctx)
{
	(void)ctx;

	log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall fork callback test %d", (int)pid);

	return 0;
}

TEST_F(metacall_fork_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	metacall_fork(&callback_test);

	if (fork() == 0)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall fork child");
	}
	else
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall fork parent");
	}

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
