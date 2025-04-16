/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <detour/detour.h>
#include <detour/detour_interface.h>

#include <log/log.h>

/* -- Definitions -- */

#define DETOUR_MANAGER_NAME			"detour"
#define DETOUR_LIBRARY_PATH			"DETOUR_LIBRARY_PATH"
#define DETOUR_LIBRARY_DEFAULT_PATH "detours"

/* -- Macros -- */

#define detour_iface(d) \
	plugin_iface_type(d, detour_interface)

/* -- Member Data -- */

static plugin_manager_declare(detour_manager);

/* -- Member Data -- */

struct detour_handle_type
{
	/* TODO: Implement hash map for holding the symbol table? */
	/* TODO: Optimize the replace process by exposing the internal replace function
	* and store all the symbols in the hash table then iterate and replace at the
	* same time, so the functions are accessed in O(1) instead of O(n)
	*/
	set symbol_map;
	set replaced_symbols;
	detour_impl_handle impl;
};

/* -- Methods -- */

int detour_initialize(void)
{
	return plugin_manager_initialize(
		&detour_manager,
		DETOUR_MANAGER_NAME,
		DETOUR_LIBRARY_PATH,
#if defined(DETOUR_LIBRARY_INSTALL_PATH)
		DETOUR_LIBRARY_INSTALL_PATH,
#else
		DETOUR_LIBRARY_DEFAULT_PATH,
#endif /* DETOUR_LIBRARY_INSTALL_PATH */
		NULL,
		NULL);
}

detour detour_create(const char *name)
{
	return plugin_manager_create(&detour_manager, name, NULL, NULL);
}

const char *detour_name(detour d)
{
	return plugin_name(d);
}

static detour_handle detour_handle_allocate(void)
{
	detour_handle handle = malloc(sizeof(struct detour_handle_type));

	if (handle == NULL)
	{
		goto alloc_handle_error;
	}

	handle->symbol_map = set_create(&hash_callback_ptr, &comparable_callback_ptr);

	if (handle->symbol_map == NULL)
	{
		goto alloc_symbol_map_error;
	}

	handle->replaced_symbols = set_create(&hash_callback_ptr, &comparable_callback_ptr);

	if (handle->replaced_symbols == NULL)
	{
		goto alloc_replaced_symbols_error;
	}

	handle->impl = NULL;

	return handle;

alloc_replaced_symbols_error:
	set_destroy(handle->symbol_map);
alloc_symbol_map_error:
	free(handle);
alloc_handle_error:
	return NULL;
}

detour_handle detour_load_file(detour d, const char *path)
{
	detour_handle handle;

	if (d == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour load arguments");

		return NULL;
	}

	handle = detour_handle_allocate();

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour load handle allocation");

		return NULL;
	}

	if (detour_iface(d)->initialize_file(&handle->impl, path) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation handle initialization");

		free(handle);

		return NULL;
	}

	return handle;
}

detour_handle detour_load_handle(detour d, dynlink library)
{
	detour_handle handle;

	if (d == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour load arguments");

		return NULL;
	}

	handle = detour_handle_allocate();

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour load handle allocation");

		return NULL;
	}

	if (detour_iface(d)->initialize_handle(&handle->impl, library) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation handle initialization");

		free(handle);

		return NULL;
	}

	return handle;
}

detour_handle detour_load_address(detour d, void (*address)(void))
{
	detour_handle handle;

	if (d == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour load arguments");

		return NULL;
	}

	handle = detour_handle_allocate();

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour load handle allocation");

		return NULL;
	}

	if (detour_iface(d)->initialize_address(&handle->impl, address) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation handle initialization");

		free(handle);

		return NULL;
	}

	return handle;
}

int detour_enumerate(detour d, detour_handle handle, unsigned int *position, const char **name, void (***address)(void))
{
	if (d == NULL || handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour replace arguments");

		return 1;
	}

	return detour_iface(d)->enumerate(handle->impl, position, name, (void ***)address);
}

int detour_replace(detour d, detour_handle handle, const char *function_name, void (*function_addr)(void), void (**function_trampoline)(void))
{
	if (d == NULL || handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour replace arguments");

		return 1;
	}

	return detour_iface(d)->replace(handle->impl, function_name, function_addr, (void **)function_trampoline);
}

void detour_unload(detour d, detour_handle handle)
{
	if (d == NULL || handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour replace arguments");
		return;
	}

	/* TODO: Should we restore all the replaced symbols? */

	detour_iface(d)->destroy(handle->impl);

	set_destroy(handle->symbol_map);

	set_destroy(handle->replaced_symbols);
}

int detour_clear(detour d)
{
	return plugin_manager_clear(&detour_manager, d);
}

void detour_destroy(void)
{
	plugin_manager_destroy(&detour_manager);
}

const char *detour_print_info(void)
{
	static const char detour_info[] =
		"Detour Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef DETOUR_STATIC_DEFINE
		"Compiled as static library type"
#else
		"Compiled as shared library type"
#endif

		"\n";

	return detour_info;
}
