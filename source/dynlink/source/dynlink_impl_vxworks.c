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

#if defined(__has_include)
	#if __has_include(<errnoLib.h>)
		#include <errnoLib.h>
	#else
		#define errnoGet() 0
		#define ERROR	   (-1)
		#define OK		   (0)
typedef int STATUS;
	#endif
	#if __has_include(<fcntl.h>)
		#include <fcntl.h>
	#else
		#define O_RDONLY 0
	#endif
	#if __has_include(<ioLib.h>)
		#include <ioLib.h>
	#else
		#define open(path, flags, mode) (-1)
		#define close(fd)				(-1)
	#endif
	#if __has_include(<loadLib.h>)
		#include <loadLib.h>
	#else
typedef void *MODULE_ID;
		#define LOAD_GLOBAL_SYMBOLS	  0
		#define LOAD_LOCAL_SYMBOLS	  1
		#define loadModule(fd, flags) NULL
	#endif
	#if __has_include(<symLib.h>)
		#include <symLib.h>
	#else
typedef int SYM_TYPE;
	#endif
	#if __has_include(<sysSymTbl.h>)
		#include <sysSymTbl.h>
	#else
		#define sysSymTbl NULL
	#endif
	#if __has_include(<unldLib.h>)
		#include <unldLib.h>
	#else
		#define unldByModuleId(mid, flags) ERROR
	#endif
#else
	/* Fallback for older compilers without __has_include */
	#include <errnoLib.h>
	#include <fcntl.h>
	#include <ioLib.h>
	#include <loadLib.h>
	#include <symLib.h>
	#include <sysSymTbl.h>
	#include <unldLib.h>
#endif

#ifndef symFindByName
	#define symFindByName(symTbl, name, pVal, pType) ERROR
#endif

/* -- Methods -- */

const char *dynlink_impl_interface_prefix_vxworks(void)
{
	/* VxWorks DKM (Downloadable Kernel Module) mode does not use lib prefix.
	 * Files are loaded as mymodule.so or mymodule.out directly. */
	static const char prefix_vxworks[] = "";

	return prefix_vxworks;
}

const char *dynlink_impl_interface_extension_vxworks(void)
{
	/* VxWorks uses different extensions:
	 * - .out for downloadable kernel modules (DKMs)
	 * - .so for Real-Time Processes (RTPs)
	 * We default to .so as it's more common in modern VxWorks
	 */
	static const char extension_vxworks[] = "so";

	return extension_vxworks;
}

dynlink_impl dynlink_impl_interface_load_vxworks(dynlink handle)
{
	dynlink_flags flags = dynlink_get_flags(handle);
	MODULE_ID impl;
	int flags_impl;
	int fd;

	/* VxWorks loadModule flags:
	 * LOAD_GLOBAL_SYMBOLS - make symbols globally available
	 * LOAD_LOCAL_SYMBOLS - keep symbols local to the module
	 */
	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_GLOBAL))
	{
		DYNLINK_FLAGS_SET(flags_impl, LOAD_GLOBAL_SYMBOLS);
	}
	else if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_LOCAL))
	{
		DYNLINK_FLAGS_SET(flags_impl, LOAD_LOCAL_SYMBOLS);
	}
	else
	{
		/* Default to global symbols for compatibility */
		DYNLINK_FLAGS_SET(flags_impl, LOAD_GLOBAL_SYMBOLS);
	}

	/* Check if we're loading symbols from the current executable */
	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_SELF))
	{
		/* Return a special marker to indicate system symbol table usage
		 * We use (MODULE_ID)-1 as a special handle for system symbols */
		impl = (MODULE_ID)-1;
	}
	else
	{
		fd = open(dynlink_get_path(handle), O_RDONLY, 0);

		if (fd < 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR,
				"DynLink VxWorks error: Failed to open module '%s' (errno: %d)",
				dynlink_get_path(handle), errnoGet());

			return NULL;
		}

		impl = loadModule(fd, flags_impl);

		if (close(fd) == ERROR)
		{
			log_write("metacall", LOG_LEVEL_DEBUG,
				"DynLink VxWorks warning: Failed to close module fd for '%s' (errno: %d)",
				dynlink_get_path(handle), errnoGet());
		}

		if (impl == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR,
				"DynLink VxWorks error: Failed to load module '%s' (errno: %d)",
				dynlink_get_path(handle), errnoGet());

			return NULL;
		}
	}

	return (dynlink_impl)impl;
}

int dynlink_impl_interface_symbol_vxworks(dynlink handle, dynlink_impl impl, const char *name, dynlink_symbol_addr *addr)
{
	SYM_TYPE symType;
	void *symbol = NULL;
	STATUS status;

	(void)handle;
	(void)impl;

	/* All symbols in VxWorks are stored in the system symbol table (sysSymTbl).
	 * There is no per-module symbol table API in the vxWorks loadModule interface. */
	status = symFindByName(sysSymTbl, (char *)name, (char **)&symbol, &symType);

	if (status != OK || symbol == NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG,
			"DynLink VxWorks: Symbol '%s' not found (errno: %d)", name, errnoGet());
		*addr = NULL;
		return 1;
	}

	dynlink_symbol_cast(void *, symbol, *addr);

	return 0;
}

int dynlink_impl_interface_unload_vxworks(dynlink handle, dynlink_impl impl)
{
	dynlink_flags flags = dynlink_get_flags(handle);

	/* Skip unload when using system symbol table for the current process */
	if (DYNLINK_FLAGS_CHECK(flags, DYNLINK_FLAGS_BIND_SELF))
	{
		return 0;
	}

#if defined(__MEMORYCHECK__) || defined(__ADDRESS_SANITIZER__) || defined(__THREAD_SANITIZER__) || defined(__MEMORY_SANITIZER__)
	/* Disable unload when running with sanitizers in order to maintain stacktraces */
	(void)impl;
	return 0;
#else
	/* VxWorks unldByModuleId returns OK (0) on success, ERROR (-1) on failure */
	{
		STATUS status = unldByModuleId((MODULE_ID)impl, 0);
		return (status == OK) ? 0 : 1;
	}
#endif
}

dynlink_impl_interface dynlink_impl_interface_singleton(void)
{
	static struct dynlink_impl_interface_type impl_interface_vxworks = {
		&dynlink_impl_interface_prefix_vxworks,
		&dynlink_impl_interface_extension_vxworks,
		&dynlink_impl_interface_load_vxworks,
		&dynlink_impl_interface_symbol_vxworks,
		&dynlink_impl_interface_unload_vxworks,
	};

	return &impl_interface_vxworks;
}
