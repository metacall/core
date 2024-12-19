/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
#include <metacall/metacall_link.h>

#include <detour/detour.h>

#include <dynlink/dynlink_type.h>

#include <log/log.h>

#include <adt/adt_set.h>

#include <stdlib.h>

/* -- Private Variables -- */

static detour_handle detour_link_handle = NULL;

static set metacall_link_table = NULL;

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

	/* Intercept if any */
	void *ptr = set_get(metacall_link_table, (set_key)symbol);

	if (ptr != NULL)
	{
		dynlink_symbol_addr addr;

		dynlink_symbol_cast(void *, ptr, addr);

		return (FARPROC)addr;
	}

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

	/* Intercept function if any */
	void *ptr = set_get(metacall_link_table, (set_key)symbol);

	/* TODO: Disable logs here until log is completely thread safe and async signal safe */
	/* log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall detour link interception: %s -> %p", symbol, ptr); */

	if (ptr != NULL)
	{
		return ptr;
	}

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

	if (metacall_link_table == NULL)
	{
		metacall_link_table = set_create(&hash_callback_str, &comparable_callback_str);

		if (metacall_link_table == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "MetaCall failed to create link table");

			metacall_link_destroy();

			return 1;
		}
	}

	return 0;
}

int metacall_link_register(const char *symbol, void (*fn)(void))
{
	void *ptr;

	if (metacall_link_table == NULL)
	{
		return 1;
	}

	dynlink_symbol_uncast(fn, ptr);

	return set_insert(metacall_link_table, (set_key)symbol, ptr);
}

int metacall_link_unregister(const char *symbol)
{
	if (metacall_link_table == NULL)
	{
		return 1;
	}

	return (set_remove(metacall_link_table, (set_key)symbol) == NULL);
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

	if (metacall_link_table != NULL)
	{
		set_destroy(metacall_link_table);

		metacall_link_table = NULL;
	}

	return result;
}
