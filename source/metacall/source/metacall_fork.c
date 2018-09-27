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

/* -- Definitions -- */

#define metacall_fork_pid _getpid

#ifdef UNICODE
#	define metacall_fork_func CreateProcessW

#	define METACALL_FORK_HOOK_ARGS \
		LPCWSTR lpApplicationName, \
		LPWSTR lpCommandLine, \
		LPSECURITY_ATTRIBUTES lpProcessAttributes, \
		LPSECURITY_ATTRIBUTES lpThreadAttributes, \
		BOOL bInheritHandles, \
		DWORD dwCreationFlags, \
		LPVOID lpEnvironment, \
		LPCWSTR lpCurrentDirectory, \
		LPSTARTUPINFOW lpStartupInfo, \
		LPPROCESS_INFORMATION lpProcessInformation

#	define METACALL_FORK_HOOK_TYPE \
		LPCWSTR, \
		LPWSTR, \
		LPSECURITY_ATTRIBUTES, \
		LPSECURITY_ATTRIBUTES, \
		BOOL, \
		DWORD, \
		LPVOID, \
		LPCWSTR, \
		LPSTARTUPINFOW, \
		LPPROCESS_INFORMATION
#else
#	define metacall_fork_func CreateProcessA

#	define METACALL_FORK_HOOK_ARGS \
		LPCSTR lpApplicationName, \
		LPSTR lpCommandLine, \
		LPSECURITY_ATTRIBUTES lpProcessAttributes, \
		LPSECURITY_ATTRIBUTES lpThreadAttributes, \
		BOOL bInheritHandles, \
		DWORD dwCreationFlags, \
		LPVOID lpEnvironment, \
		LPCSTR lpCurrentDirectory, \
		LPSTARTUPINFOA lpStartupInfo, \
		LPPROCESS_INFORMATION lpProcessInformation

#	define METACALL_FORK_HOOK_TYPE \
		LPCSTR, \
		LPSTR, \
		LPSECURITY_ATTRIBUTES, \
		LPSECURITY_ATTRIBUTES, \
		BOOL, \
		DWORD, \
		LPVOID, \
		LPCSTR, \
		LPSTARTUPINFOA, \
		LPPROCESS_INFORMATION
#endif

/* -- Methods -- */


BOOL metacall_fork_hook(METACALL_FORK_HOOK_ARGS);

#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

/* -- Definitions -- */

#define metacall_fork_func fork

/* -- Methods -- */

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

BOOL metacall_fork_hook(METACALL_FORK_HOOK_ARGS)
{
	BOOL (*metacall_fork_trampoline)(METACALL_FORK_HOOK_TYPE) = (BOOL(*)(METACALL_FORK_HOOK_TYPE))detour_trampoline(metacall_detour_handle);

	metacall_fork_callback_ptr callback = metacall_callback;

	BOOL result;

	if (metacall_destroy() != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall fork auto destruction");
	}

	result = metacall_fork_trampoline(
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation);

	if (result == FALSE)
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

pid_t metacall_fork_hook()
{
	pid_t (*metacall_fork_trampoline)(void) = (pid_t(*)(void))detour_trampoline(metacall_detour_handle);

	metacall_fork_callback_ptr callback = metacall_callback;

	pid_t pid;

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
	if (detour_initialize() != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour initialization");

		return 1;
	}

	metacall_detour = detour_create(metacall_fork_detour_name);

	if (metacall_detour == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour creation");

		metacall_fork_destroy();

		return 1;
	}

	metacall_detour_handle = detour_install(metacall_detour, (void(*)(void))&metacall_fork_func, (void(*)(void))&metacall_fork_hook);

	if (metacall_detour_handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour installation");

		metacall_fork_destroy();

		return 1;
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
