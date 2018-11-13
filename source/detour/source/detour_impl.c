/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

/* -- Headers -- */

#include <detour/detour_impl.h>
#include <detour/detour_interface.h>
#include <detour/detour_host.h>

#include <dynlink/dynlink.h>

#include <log/log.h>

#include <memory/memory.h>

#include <string.h>

/* -- Definitions -- */

#define DETOUR_DYNLINK_NAME_SIZE		0x40
#define DETOUR_DYNLINK_SUFFIX			"_detour"

/* -- Member Data -- */

struct detour_impl_type
{
	dynlink handle;
	detour_interface iface;
	detour_host host;
};

/* -- Private Methods -- */

static dynlink detour_impl_load_dynlink(const char * path, const char * name);

static int detour_impl_load_symbol(dynlink handle, const char * name, dynlink_symbol_addr * singleton_addr_ptr);

/* -- Methods -- */

dynlink detour_impl_load_dynlink(const char * path, const char * name)
{
	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		const char detour_dynlink_suffix[] = DETOUR_DYNLINK_SUFFIX "d";
	#else
		const char detour_dynlink_suffix[] = DETOUR_DYNLINK_SUFFIX;
	#endif

	#define DETOUR_DYNLINK_NAME_FULL_SIZE \
		(sizeof(detour_dynlink_suffix) + DETOUR_DYNLINK_NAME_SIZE)

	char detour_dynlink_name[DETOUR_DYNLINK_NAME_FULL_SIZE];

	strncpy(detour_dynlink_name, name, DETOUR_DYNLINK_NAME_FULL_SIZE);

	strncat(detour_dynlink_name, detour_dynlink_suffix,
		DETOUR_DYNLINK_NAME_FULL_SIZE - strnlen(detour_dynlink_name, DETOUR_DYNLINK_NAME_FULL_SIZE - 1) - 1);

	#undef DETOUR_DYNLINK_NAME_FULL_SIZE

	log_write("metacall", LOG_LEVEL_DEBUG, "Loading detour plugin: %s", detour_dynlink_name);

	return dynlink_load(path, detour_dynlink_name, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
}

int detour_impl_load_symbol(dynlink handle, const char * name, dynlink_symbol_addr * singleton_addr_ptr)
{
	const char detour_dynlink_symbol_prefix[] = DYNLINK_SYMBOL_STR("");
	const char detour_dynlink_symbol_suffix[] = "_detour_impl_interface_singleton";

	#define DETOUR_DYNLINK_SYMBOL_SIZE \
		(sizeof(detour_dynlink_symbol_prefix) + DETOUR_DYNLINK_NAME_SIZE + sizeof(detour_dynlink_symbol_suffix))

	char detour_dynlink_symbol[DETOUR_DYNLINK_SYMBOL_SIZE];

	strncpy(detour_dynlink_symbol, detour_dynlink_symbol_prefix, DETOUR_DYNLINK_SYMBOL_SIZE);

	strncat(detour_dynlink_symbol, name,
		DETOUR_DYNLINK_SYMBOL_SIZE - strnlen(detour_dynlink_symbol, DETOUR_DYNLINK_SYMBOL_SIZE - 1) - 1);

	strncat(detour_dynlink_symbol, detour_dynlink_symbol_suffix,
		DETOUR_DYNLINK_SYMBOL_SIZE - strnlen(detour_dynlink_symbol, DETOUR_DYNLINK_SYMBOL_SIZE - 1) - 1);

	#undef DETOUR_DYNLINK_SYMBOL_SIZE

	log_write("metacall", LOG_LEVEL_DEBUG, "Loading detour symbol: %s", detour_dynlink_symbol);

	return dynlink_symbol(handle, detour_dynlink_symbol, singleton_addr_ptr);
}

detour_impl detour_impl_create()
{
	detour_impl impl = malloc(sizeof(struct detour_impl_type));

	if (impl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation allocation");

		return NULL;
	}

	impl->host = (detour_host)malloc(sizeof(struct detour_host_type));

	if (impl->host == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation host allocation");

		free(impl);

		return NULL;
	}

	impl->host->log = log_instance();

	impl->handle = NULL;
	impl->iface = NULL;

	return impl;
}

int detour_impl_load(detour_impl impl, const char * path, const char * name)
{
	dynlink_symbol_addr singleton_addr;

	detour_interface_singleton iface_singleton;

	impl->handle = detour_impl_load_dynlink(path, name);

	if (impl->handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour dynlink loading");

		return 1;
	}

	if (detour_impl_load_symbol(impl->handle, name, &singleton_addr) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour dynlink symbol loading");

		dynlink_unload(impl->handle);

		impl->handle = NULL;

		return 1;
	}

	iface_singleton = (detour_interface_singleton)DYNLINK_SYMBOL_GET(singleton_addr);

	if (iface_singleton == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour interface singleton access");

		dynlink_unload(impl->handle);

		impl->handle = NULL;

		return 1;
	}

	impl->iface = iface_singleton();

	return 0;
}

detour_impl_handle detour_impl_install(detour_impl impl, void(**target)(void), void(*hook)(void))
{
	detour_impl_handle handle = impl->iface->initialize(impl->host);

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation handle initialization");

		return NULL;
	}

	if (impl->iface->install(handle, target, hook) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation handle installation");

		if (impl->iface->destroy(handle) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation handle destruction");
		}

		return NULL;
	}

	return handle;
}

int detour_impl_uninstall(detour_impl impl, detour_impl_handle handle)
{
	if (impl->iface->uninstall(handle) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation handle uninstallation");

		return 1;
	}

	return impl->iface->destroy(handle);
}

int detour_impl_unload(detour_impl impl)
{
	if (impl != NULL)
	{
		impl->iface = NULL;

		if (impl->handle != NULL)
		{
			dynlink_unload(impl->handle);

			impl->handle = NULL;
		}
	}

	return 0;
}

int detour_impl_destroy(detour_impl impl)
{
	if (impl != NULL)
	{
		if (impl->handle != NULL)
		{
			dynlink_unload(impl->handle);
		}

		if (impl->host != NULL)
		{
			free(impl->host);
		}

		free(impl);
	}

	return 0;
}
