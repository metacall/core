/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

 /* -- Headers -- */

#include <metacall/metacall.h>
#include <metacall/metacall_fork.h>

#include <detour/detour.h>

#include <log/log.h>

#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

 /* -- Headers -- */

#define _WIN32_WINNT 0x0600
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* -- Definitions -- */

#define metacall_fork_pid _getpid

/* -- Type Definitions -- */

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

typedef NTSTATUS (*RtlCloneUserProcessPtr)(ULONG ProcessFlags,
	PSECURITY_DESCRIPTOR ProcessSecurityDescriptor,
	PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
	HANDLE DebugPort,
	PRTL_USER_PROCESS_INFORMATION ProcessInformation);

/* -- Methods -- */

void (*metacall_fork_func(void))(void);

NTSTATUS metacall_fork_hook(ULONG ProcessFlags,
	PSECURITY_DESCRIPTOR ProcessSecurityDescriptor,
	PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
	HANDLE DebugPort,
	PRTL_USER_PROCESS_INFORMATION ProcessInformation);

#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

 /* -- Methods -- */

void (*metacall_fork_func(void))(void);

pid_t metacall_fork_hook(void);

#else
#	error "Unknown metacall fork safety platform"
#endif

/* -- Private Variables -- */

static const char metacall_fork_detour_name[] = "funchook";

static detour metacall_detour = NULL;

static detour_handle metacall_detour_handle = NULL;

static metacall_fork_callback_ptr metacall_callback = NULL;

/* -- Methods -- */

#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

void (*metacall_fork_func(void))(void)
{
	HMODULE module;
	RtlCloneUserProcessPtr clone_ptr;

	module = GetModuleHandle("ntdll.dll");

	if (!module)
	{
		return NULL;
	}

	clone_ptr = (RtlCloneUserProcessPtr)GetProcAddress(module, "RtlCloneUserProcess");

	return (void(*)(void))clone_ptr;
}

NTSTATUS metacall_fork_hook(ULONG ProcessFlags,
	PSECURITY_DESCRIPTOR ProcessSecurityDescriptor,
	PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
	HANDLE DebugPort,
	PRTL_USER_PROCESS_INFORMATION ProcessInformation)
{
	RtlCloneUserProcessPtr metacall_fork_trampoline = (RtlCloneUserProcessPtr)detour_trampoline(metacall_detour_handle);

	metacall_fork_callback_ptr callback = metacall_callback;

	NTSTATUS result;

	metacall_fork_prepare();

	if (metacall_destroy() != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall fork auto destruction");
	}

	result = metacall_fork_trampoline(ProcessFlags, ProcessSecurityDescriptor, ThreadSecurityDescriptor, DebugPort, ProcessInformation);

	if (result != ((NTSTATUS)0x00000000L))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall fork trampoline invocation");
	}

	if (metacall_initialize() != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall fork auto initialization");
	}

	metacall_fork_callback(callback);

	if (callback != NULL)
	{
		/* TODO: Context */
		if (callback(_getpid(), NULL) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour callback invocation");
		}
	}

	return result;
}

#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

void (*metacall_fork_func(void))(void)
{
	return (void(*)(void))&fork;
}

pid_t metacall_fork_hook()
{
	pid_t (*metacall_fork_trampoline)(void) = (pid_t(*)(void))detour_trampoline(metacall_detour_handle);

	metacall_fork_callback_ptr callback = metacall_callback;

	pid_t pid;

	metacall_fork_prepare();

	if (metacall_destroy() != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall fork auto destruction");
	}

	pid = metacall_fork_trampoline();

	if (metacall_initialize() != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall fork auto initialization");
	}

	metacall_fork_callback(callback);

	if (callback != NULL)
	{
		/* TODO: Context */
		if (callback(pid, NULL) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour callback invocation");
		}
	}

	return pid;
}

#else
#	error "Unknown metacall fork safety platform"
#endif

int metacall_fork_initialize()
{
	void (*fork_func)(void) = metacall_fork_func();

	if (fork_func == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid fork function pointer");

		return 1;
	}

	if (detour_initialize() != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour initialization");

		return 1;
	}

	if (metacall_detour == NULL)
	{
		metacall_detour = detour_create(metacall_fork_detour_name);

		if (metacall_detour == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour creation");

			metacall_fork_destroy();

			return 1;
		}
	}

	if (metacall_detour_handle == NULL)
	{
		metacall_detour_handle = detour_install(metacall_detour, (void(*)(void))fork_func, (void(*)(void))&metacall_fork_hook);

		if (metacall_detour_handle == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour installation");

			metacall_fork_destroy();

			return 1;
		}
	}

	return 0;
}

void metacall_fork_callback(metacall_fork_callback_ptr callback)
{
	metacall_callback = callback;
}

int metacall_fork_destroy()
{
	int result = 0;

	if (metacall_detour_handle != NULL)
	{
		if (detour_uninstall(metacall_detour, metacall_detour_handle) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour uninstall");

			result = 1;
		}

		metacall_detour_handle = NULL;
	}

	if (metacall_detour != NULL)
	{
		if (detour_clear(metacall_detour) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour clear");

			result = 1;
		}

		metacall_detour = NULL;
	}

	detour_destroy();

	metacall_callback = NULL;

	return result;
}
