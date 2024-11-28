// /*
//  *	MetaCall Library by Parra Studios
//  *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
//  *
//  *	A library for providing a foreign function interface calls.
//  *
//  */

// /* -- Headers -- */

#include <metacall/metacall.h>
#include <metacall/metacall_link.h>

#include <detour/detour.h>

#include <log/log.h>

#include <stdlib.h>

/* -- Private Variables -- */

static detour_handle detour_link_handle = NULL;

#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)

	#include <windows.h>

void (*metacall_link_func(void))(void)
{
	return (void (*)(void))(&GetProcAddress);
}

FARPROC metacall_link_hook(HMODULE handle, LPCSTR symbol)
{
	typedef FARPROC (*metacall_link_func_ptr)(HMODULE, LPCSTR);

	metacall_link_func_ptr metacall_link_trampoline = (metacall_link_func_ptr)detour_trampoline(detour_link_handle);

	// TODO: Intercept if any, iterate hash map elements and invoke them

	return metacall_link_trampoline(handle, symbol);
}

#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

	#include <dlfcn.h>

void (*metacall_link_func(void))(void)
{
	return (void (*)(void))(&dlsym);
}

void *metacall_link_hook(void *handle, const char *symbol)
{
	typedef void *(*metacall_link_func_ptr)(void *, const char *);

	metacall_link_func_ptr metacall_link_trampoline = (metacall_link_func_ptr)detour_trampoline(detour_link_handle);

	// TODO: Intercept if any, iterate hash map elements and invoke them

	return metacall_link_trampoline(handle, symbol);
}

#else
	#error "Unknown metacall link platform"
#endif

/* -- Methods -- */

int metacall_link_initialize(void)
{
	detour d = detour_create(metacall_detour());

	if (detour_link_handle == NULL)
	{
		detour_link_handle = detour_install(d, (void (*)(void))metacall_link_func(), (void (*)(void))(&metacall_link_hook));

		if (detour_link_handle == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour link installation");

			metacall_link_destroy();

			return 1;
		}
	}

	return 0;
}

int metacall_link_destroy(void)
{
	int result = 0;

	if (detour_link_handle != NULL)
	{
		detour d = detour_create(metacall_detour());

		if (detour_uninstall(d, detour_link_handle) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall invalid detour fork uninstall");

			result = 1;
		}

		detour_link_handle = NULL;
	}

	// TODO: Destroy hash map

	return result;
}
