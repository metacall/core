/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <dynlink/dynlink.h>

#include <dynlink/dynlink_impl.h>

#include <log/log.h>

#include <string.h>

#include <dl.h>
#include <errno.h>

/* -- Methods -- */

const char *dynlink_impl_interface_prefix_hpux(void)
{
	static const char prefix_hpux[] = "lib";

	return prefix_hpux;
}

const char *dynlink_impl_interface_extension_hpux(void)
{
	/* HP-UX uses different extensions based on architecture:
	 * - PA-RISC systems use .sl (shared library)
	 * - Itanium (IA-64) systems use .so (shared object)
	 */
#if defined(__ia64) || defined(__ia64__)
	static const char extension_hpux[] = "so";
#else
	/* PA-RISC or unknown - default to .sl */
	static const char extension_hpux[] = "sl";
#endif

	return extension_hpux;
}

dynlink_impl dynlink_impl_interface_load_hpux(dynlink handle)
{
	dynlink_flags flags = dynlink_get_flags(handle);
	int flags_impl;
	shl_t impl;

	/* Default is BIND_IMMEDIATE (0x0) which resolves all symbols at load time.
	 * We start with BIND_NONFATAL to allow loading even if some symbols are unresolved. */
	DYNLINK_FLAGS_SET(flags_impl, BIND_NONFATAL);

	/* Convert generic flags to HP-UX specific flags.
	 * Note: BIND_IMMEDIATE is 0x0, so it's the default when BIND_DEFERRED is not set. */
	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_LAZY))
	{
		DYNLINK_FLAGS_ADD(flags_impl, BIND_DEFERRED);
	}

	/* Note: HP-UX shl_load does not have direct equivalents for LOCAL/GLOBAL visibility
	 * like Unix's RTLD_LOCAL/RTLD_GLOBAL. These flags are silently ignored on HP-UX.
	 * BIND_FIRST could be used for different symbol lookup order, but it's not the same concept. */

	/* Check if we're loading the current executable itself */
	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_SELF))
	{
		/* PROG_HANDLE ((void *)-1) is a special handle representing the main program.
		 * It allows shl_findsym to search for symbols in the running executable. */
		impl = PROG_HANDLE;
	}
	else
	{
		impl = shl_load(dynlink_get_path(handle), flags_impl, 0L);

		if (impl == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "DynLink HP-UX error: Failed to load library '%s' (errno: %d - %s)",
				dynlink_get_path(handle), errno, strerror(errno));

			return NULL;
		}
	}

	return (dynlink_impl)impl;
}

int dynlink_impl_interface_symbol_hpux(dynlink handle, dynlink_impl impl, const char *name, dynlink_symbol_addr *addr)
{
	void *symbol = NULL;
	int result;

	(void)handle;

	/* shl_findsym searches for a symbol in the loaded library
	 * TYPE_PROCEDURE is used for function symbols
	 * TYPE_DATA would be used for data symbols
	 * TYPE_UNDEFINED searches for any type */
	result = shl_findsym((shl_t *)&impl, name, TYPE_UNDEFINED, &symbol);

	if (result != 0 || symbol == NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "DynLink HP-UX: Symbol '%s' not found (errno: %d)", name, errno);
		*addr = NULL;
		return 1;
	}

	dynlink_symbol_cast(void *, symbol, *addr);

	return 0;
}

int dynlink_impl_interface_unload_hpux(dynlink handle, dynlink_impl impl)
{
	dynlink_flags flags = dynlink_get_flags(handle);

	/* Skip unlink when using PROG_HANDLE for the current process */
	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_SELF))
	{
		return 0;
	}

#if defined(__MEMORYCHECK__) || defined(__ADDRESS_SANITIZER__) || defined(__THREAD_SANITIZER__) || defined(__MEMORY_SANITIZER__)
	/* Disable unload when running with valgrind or sanitizers in order to maintain stacktraces */
	(void)impl;
	return 0;
#else
	return shl_unload((shl_t)impl);
#endif
}

dynlink_impl_interface dynlink_impl_interface_singleton(void)
{
	static struct dynlink_impl_interface_type impl_interface_hpux = {
		&dynlink_impl_interface_prefix_hpux,
		&dynlink_impl_interface_extension_hpux,
		&dynlink_impl_interface_load_hpux,
		&dynlink_impl_interface_symbol_hpux,
		&dynlink_impl_interface_unload_hpux,
	};

	return &impl_interface_hpux;
}
