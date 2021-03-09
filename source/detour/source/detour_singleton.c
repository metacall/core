/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

/* -- Headers -- */

#include <detour/detour_singleton.h>

#include <environment/environment_variable_path.h>

#include <adt/adt_set.h>

#include <log/log.h>

#include <string.h>

/* -- Definitions -- */

#define DETOUR_LIBRARY_PATH			"DETOUR_LIBRARY_PATH"
#define DETOUR_LIBRARY_DEFAULT_PATH	"detours"

/* -- Member Data -- */

struct detour_singleton_type
{
	set detours;
	char * library_path;
};

/* -- Private Methods -- */

/**
*  @brief
*    Detour singleton destroy callback iterator
*
*  @param[in] s
*    Pointer to detours set
*
*  @param[in] key
*    Pointer to current detour key
*
*  @param[in] val
*    Pointer to current detour instance
*
*  @param[in] args
*    Pointer to user defined callback arguments
*
*  @return
*    Returns zero to continue iteration, distinct from zero otherwise
*
*/
static int detour_singleton_destroy_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

/* -- Member Data -- */

static struct detour_singleton_type detour_singleton_default =
{
	NULL,
	NULL
};

detour_singleton detour_singleton_ptr = &detour_singleton_default;

/* -- Methods -- */

detour_singleton detour_singleton_instance()
{
	return detour_singleton_ptr;
}

int detour_singleton_initialize()
{
	detour_singleton singleton = detour_singleton_instance();

	if (singleton->detours == NULL)
	{
		singleton->detours = set_create(&hash_callback_str, &comparable_callback_str);

		if (singleton->detours == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour singleton set initialization");

			detour_singleton_destroy();

			return 1;
		}
	}

	if (singleton->library_path == NULL)
	{
		static const char detour_library_path[] = DETOUR_LIBRARY_PATH;

		#if defined(DETOUR_LIBRARY_INSTALL_PATH)
			static const char detour_library_default_path[] = DETOUR_LIBRARY_INSTALL_PATH;
		#else
			static const char detour_library_default_path[] = DETOUR_LIBRARY_DEFAULT_PATH;
		#endif /* DETOUR_LIBRARY_INSTALL_PATH */

		singleton->library_path = environment_variable_path_create(detour_library_path, detour_library_default_path);

		if (singleton->library_path == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid detour singleton library path initialization");

			detour_singleton_destroy();

			return 1;
		}
	}

	return 0;
}

int detour_singleton_register(detour s)
{
	detour_singleton singleton = detour_singleton_instance();

	const char * name = detour_name(s);

	if (set_get(singleton->detours, (set_key)name) != NULL)
	{
		return 1;
	}

	return set_insert(singleton->detours, (set_key)name, s);
}

detour detour_singleton_get(const char * name)
{
	detour_singleton singleton = detour_singleton_instance();

	return set_get(singleton->detours, (set_key)name);
}

const char * detour_singleton_path()
{
	detour_singleton singleton = detour_singleton_instance();

	return singleton->library_path;
}

int detour_singleton_clear(detour s)
{
	detour_singleton singleton = detour_singleton_instance();

	const char * name = detour_name(s);

	if (set_get(singleton->detours, (set_key)name) == NULL)
	{
		return 0;
	}

	if (set_remove(singleton->detours, (const set_key)name) == NULL)
	{
		return 1;
	}

	return 0;
}

int detour_singleton_destroy_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
	(void)key;
	(void)args;

	if (val != NULL)
	{
		detour_clear((detour)val);
	}

	return 0;
}

void detour_singleton_destroy()
{
	detour_singleton singleton = detour_singleton_instance();

	if (singleton->detours != NULL)
	{
		set_iterate(singleton->detours, &detour_singleton_destroy_cb_iterate, NULL);

		set_destroy(singleton->detours);

		singleton->detours = NULL;
	}

	if (singleton->library_path != NULL)
	{
		environment_variable_path_destroy(singleton->library_path);

		singleton->library_path = NULL;
	}
}
