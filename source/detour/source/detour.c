/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
	void (*target)(void);
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

void (*detour_trampoline(detour_handle handle))(void)
{
	if (handle == NULL)
	{
		return NULL;
	}

	return handle->target;
}

detour_handle detour_install(detour d, void (*target)(void), void (*hook)(void))
{
	if (d == NULL || target == NULL || hook == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour install arguments");

		return NULL;
	}

	detour_handle handle = malloc(sizeof(struct detour_handle_type));

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour install handle allocation");

		return NULL;
	}

	handle->impl = detour_iface(d)->initialize();

	if (handle->impl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation handle initialization");

		free(handle);

		return NULL;
	}

	void (**target_ptr)(void) = &target;

	if (detour_iface(d)->install(handle->impl, target_ptr, hook) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation handle installation");

		if (detour_iface(d)->destroy(handle->impl) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation handle destruction");
		}

		free(handle);

		return NULL;
	}

	handle->target = *target_ptr;

	return handle;
}

int detour_uninstall(detour d, detour_handle handle)
{
	int result = 0;

	if (d == NULL || handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid uninstall arguments");

		return 1;
	}

	result |= detour_iface(d)->uninstall(handle->impl);

	if (result != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation handle uninstallation");
	}

	result |= detour_iface(d)->destroy(handle->impl);

	if (result != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation handle destruction");
	}

	free(handle);

	return result;
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
		"Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef DETOUR_STATIC_DEFINE
		"Compiled as static library type"
#else
		"Compiled as shared library type"
#endif

		"\n";

	return detour_info;
}
