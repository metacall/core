/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

/* -- Headers -- */

#include <metacall/metacall.h>
#include <metacall/metacall_fork.h>

#include <detour/detour.h>

#include <log/log.h>

#include <stdlib.h>

/* -- Methods -- */

#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

/* -- Headers -- */

	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0600
	#endif
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>

/* -- Type Definitions -- */

typedef long NTSTATUS;

typedef struct _CLIENT_ID
{
	PVOID UniqueProcess;
	PVOID UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _SECTION_IMAGE_INFORMATION
{
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

typedef struct _RTL_USER_PROCESS_INFORMATION
{
	ULONG Size;
	HANDLE Process;
	HANDLE Thread;
	CLIENT_ID ClientId;
	SECTION_IMAGE_INFORMATION ImageInformation;
} RTL_USER_PROCESS_INFORMATION, *PRTL_USER_PROCESS_INFORMATION;

typedef NTSTATUS(NTAPI *RtlCloneUserProcessPtr)(ULONG ProcessFlags,
	PSECURITY_DESCRIPTOR ProcessSecurityDescriptor,
	PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
	HANDLE DebugPort,
	PRTL_USER_PROCESS_INFORMATION ProcessInformation);

/* -- Methods -- */

NTSTATUS NTAPI metacall_fork_hook(ULONG ProcessFlags,
	PSECURITY_DESCRIPTOR ProcessSecurityDescriptor,
	PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
	HANDLE DebugPort,
	PRTL_USER_PROCESS_INFORMATION ProcessInformation);

#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

/* -- Methods -- */

pid_t metacall_fork_hook(void);

#else
	#error "Unknown metacall fork safety platform"
#endif

/* -- Private Variables -- */

static detour_handle detour_fork_handle = NULL;

static metacall_pre_fork_callback_ptr metacall_pre_fork_callback = NULL;
static metacall_post_fork_callback_ptr metacall_post_fork_callback = NULL;

/* -- Methods -- */

#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

typedef RtlCloneUserProcessPtr metacall_fork_trampoline_type;

static const char metacall_fork_func_name[] = "RtlCloneUserProcess";
static metacall_fork_trampoline_type metacall_fork_trampoline = NULL;

static detour_handle metacall_fork_handle(detour d)
{
	return detour_load_file(d, "ntdll.dll");
}

NTSTATUS NTAPI metacall_fork_hook(ULONG ProcessFlags,
	PSECURITY_DESCRIPTOR ProcessSecurityDescriptor,
	PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
	HANDLE DebugPort,
	PRTL_USER_PROCESS_INFORMATION ProcessInformation)
{
	metacall_pre_fork_callback_ptr pre_callback = metacall_pre_fork_callback;
	metacall_post_fork_callback_ptr post_callback = metacall_post_fork_callback;

	NTSTATUS result;

	log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall process forked");

	/* Execute pre fork callback */
	if (pre_callback != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall process pre fork callback execution");

		/* TODO: Context */
		if (pre_callback(NULL) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour pre fork callback invocation");
		}
	}

	log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall process fork auto destroy");

	/* Destroy metacall before the fork */
	metacall_destroy();

	/* Execute the real fork */
	result = metacall_fork_trampoline(ProcessFlags, ProcessSecurityDescriptor, ThreadSecurityDescriptor, DebugPort, ProcessInformation);

	if (result != ((NTSTATUS)0x00000000L))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall fork trampoline invocation");
	}

	log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall process fork re-initialize");

	/* Initialize metacall again */
	if (metacall_initialize() != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall fork auto initialization");
	}

	/* Set again the callbacks in the new process */
	metacall_fork(pre_callback, post_callback);

	/* Execute post fork callback */
	if (post_callback != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall process post fork callback execution");

		/* TODO: Context */
		if (post_callback(_getpid(), NULL) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour post fork callback invocation");
		}
	}

	return result;
}

#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

typedef pid_t (*metacall_fork_trampoline_type)(void);

static const char metacall_fork_func_name[] = "fork";
static metacall_fork_trampoline_type metacall_fork_trampoline = NULL;

static detour_handle metacall_fork_handle(detour d)
{
	return detour_load_file(d, NULL);
}

pid_t metacall_fork_hook(void)
{
	metacall_pre_fork_callback_ptr pre_callback = metacall_pre_fork_callback;
	metacall_post_fork_callback_ptr post_callback = metacall_post_fork_callback;

	pid_t pid;

	log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall process forked");

	/* Execute pre fork callback */
	if (pre_callback != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall process pre fork callback execution");

		/* TODO: Context */
		if (pre_callback(NULL) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour pre fork callback invocation");
		}
	}

	log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall process fork auto destroy");

	/* Destroy metacall before the fork */
	metacall_destroy();

	/* Execute the real fork */
	pid = metacall_fork_trampoline();

	log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall process fork re-initialize");

	/* Initialize metacall again */
	if (metacall_initialize() != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall fork auto initialization");
	}

	/* Set again the callbacks in the new process */
	metacall_fork(pre_callback, post_callback);

	/* Execute post fork callback */
	if (post_callback != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall process post fork callback execution");

		/* TODO: Context */
		if (post_callback(pid, NULL) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour post fork callback invocation");
		}
	}

	return pid;
}

#else
	#error "Unknown metacall fork safety platform"
#endif

int metacall_fork_initialize(void)
{
	detour d = detour_create(metacall_detour());

	if (detour_fork_handle == NULL)
	{
		/* Casting for getting the original function */
		union
		{
			metacall_fork_trampoline_type *trampoline;
			void (**ptr)(void);
		} cast = { &metacall_fork_trampoline };

		detour_fork_handle = metacall_fork_handle(d);

		if (detour_fork_handle == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour fork installation");

			metacall_fork_destroy();

			return 1;
		}

		if (detour_replace(d, detour_fork_handle, metacall_fork_func_name, (void (*)(void))(&metacall_fork_hook), cast.ptr) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour fork replacement");

			metacall_link_destroy();

			return 1;
		}
	}

	return 0;
}

void metacall_fork(metacall_pre_fork_callback_ptr pre_callback, metacall_post_fork_callback_ptr post_callback)
{
	metacall_pre_fork_callback = pre_callback;
	metacall_post_fork_callback = post_callback;
}

void metacall_fork_destroy(void)
{
	if (detour_fork_handle != NULL)
	{
		detour d = detour_create(metacall_detour());

		/* TODO: Restore the hook? We need support for this on the detour API */

		detour_unload(d, detour_fork_handle);

		detour_fork_handle = NULL;
	}

	metacall_pre_fork_callback = NULL;
	metacall_post_fork_callback = NULL;
}
