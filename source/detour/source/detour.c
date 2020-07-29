/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <detour/detour.h>
#include <detour/detour_singleton.h>
#include <detour/detour_impl.h>

#include <log/log.h>

#include <memory/memory.h>

#include <string.h>

/* -- Member Data -- */

struct detour_type
{
	char * name;
	detour_impl impl;
};

struct detour_handle_type
{
	void(*target)(void);
	detour_impl_handle impl;
};

/* -- Methods -- */

int detour_initialize()
{
	if (detour_singleton_initialize() != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour singleton initialization");

		return 1;
	}

	return 0;
}

detour detour_create(const char * name)
{
	detour d;

	size_t name_length;

	if (name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour name");

		return NULL;
	}

	d = detour_singleton_get(name);

	if (d != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Detour <%p> already exists", (void *)d);

		return d;
	}

	name_length = strlen(name);

	if (name_length == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour name length");

		return NULL;
	}

	d = malloc(sizeof(struct detour_type));

	if (d == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour allocation");

		return NULL;
	}

	d->name = malloc(sizeof(char) * (name_length + 1));

	if (d->name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour name allocation");

		free(d);

		return NULL;
	}

	strncpy(d->name, name, name_length);

	d->name[name_length] = '\0';

	d->impl = detour_impl_create();

	if (d->impl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation creation");

		free(d->name);

		free(d);

		return NULL;
	}

	if (detour_impl_load(d->impl, detour_singleton_path(), d->name) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation loading");

		detour_impl_destroy(d->impl);

		free(d->name);

		free(d);

		return NULL;
	}

	if (detour_singleton_register(d) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour singleton insert");

		detour_impl_destroy(d->impl);

		free(d->name);

		free(d);

		return NULL;
	}

	return d;
}

const char * detour_name(detour d)
{
	return d->name;
}

void (*detour_trampoline(detour_handle handle))(void)
{
	return handle->target;
}

detour_handle detour_install(detour d, void(*target)(void), void(*hook)(void))
{
	detour_handle handle;

	void(**target_ptr)(void);

	if (d == NULL || target == NULL || hook == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid install arguments");

		return NULL;
	}

	handle = malloc(sizeof(struct detour_handle_type));

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid install handle allocation");

		return NULL;
	}

	target_ptr = &target;

	handle->impl = detour_impl_install(d->impl, target_ptr, hook);

	if (handle->impl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid install implementation");

		free(handle);

		return NULL;
	}

	handle->target = *target_ptr;

	return handle;
}

int detour_uninstall(detour d, detour_handle handle)
{
	if (d == NULL || handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid uninstall arguments");

		return 1;
	}

	if (detour_impl_uninstall(d->impl, handle->impl) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour uninstall");

		free(handle);

		return 1;
	}

	free(handle);

	return 0;
}

int detour_clear(detour d)
{
	if (d != NULL)
	{
		int result = 0;

		if (detour_singleton_clear(d) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour singleton clear");

			result = 1;
		}

		if (d->name != NULL)
		{
			free(d->name);
		}

		if (d->impl != NULL)
		{
			if (detour_impl_unload(d->impl) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation unloading");

				result = 1;
			}

			if (detour_impl_destroy(d->impl) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour implementation destruction");

				result = 1;
			}
		}

		free(d);

		return result;
	}

	return 0;
}

void detour_destroy()
{
	detour_singleton_destroy();
}

const char * detour_print_info()
{
	static const char detour_info[] =
		"Detour Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef DETOUR_STATIC_DEFINE
			"Compiled as static library type"
		#else
			"Compiled as shared library type"
		#endif

		"\n";

	return detour_info;
}
