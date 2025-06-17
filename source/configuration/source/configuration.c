/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <configuration/configuration.h>
#include <configuration/configuration_impl.h>
#include <configuration/configuration_singleton.h>

#include <environment/environment_variable.h>
#include <environment/environment_variable_path.h>

#include <serial/serial.h>

#include <log/log.h>

/* -- Definitions -- */

#define CONFIGURATION_PATH		   "CONFIGURATION_PATH"
#define CONFIGURATION_DEFAULT_PATH "configurations" ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR CONFIGURATION_GLOBAL_SCOPE ".json"

/* -- Methods -- */

static int configuration_path_from_library_path(dynlink_path library_relative_path, const char relative_path[], size_t relative_path_size)
{
	static const char library_name[] = "metacall"
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
									   "d"
#endif
		;

	dynlink_path library_path, join_path;

	size_t size, length = 0;

	if (dynlink_library_path(library_name, library_path, &length) != 0)
	{
		return 1;
	}

	log_write("metacall", LOG_LEVEL_DEBUG, "Finding configuration relative path %s to %s", relative_path, library_path);

	/* Get the current folder without the library */
	size = portability_path_get_directory_inplace(library_path, length + 1);

	/* Append the relative path */
	size = portability_path_join(library_path, size, relative_path, relative_path_size, join_path, PORTABILITY_PATH_SIZE);

	/* Make it cannonical */
	size = portability_path_canonical(join_path, size, library_relative_path, PORTABILITY_PATH_SIZE);

	return size == 0;
}

int configuration_initialize(const char *reader, const char *path, void *allocator)
{
	configuration global = NULL;

	if (serial_initialize() != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration serial initialization");

		return 1;
	}

	/* The order of precedence is:
	* 1) Environment variable
	* 2) Default relative path to metacall library
	* 3) Locate it relative to metacall library install path
	* 4) Default installation path (if any)
	*/
	if (path == NULL)
	{
		static const char configuration_path[] = CONFIGURATION_PATH;

		const char *env_path = environment_variable_get(configuration_path, NULL);

		dynlink_path library_relative_path;

		if (env_path != NULL)
		{
			global = configuration_object_initialize(CONFIGURATION_GLOBAL_SCOPE, env_path, NULL);

			path = env_path;
		}

		if (global == NULL)
		{
			static const char configuration_default_path[] = CONFIGURATION_DEFAULT_PATH;

			if (configuration_path_from_library_path(library_relative_path, configuration_default_path, sizeof(configuration_default_path)) == 0)
			{
				global = configuration_object_initialize(CONFIGURATION_GLOBAL_SCOPE, library_relative_path, NULL);

				path = library_relative_path;
			}
		}

#if (defined(WIN32) || defined(_WIN32)) && defined(_MSC_VER)
		/* Windows MSVC when running the tests, it has the binaries in Debug / Release folders, so we must also check in the parent folder */
		if (global == NULL)
		{
			static const char configuration_default_path_win32[] = ".." ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR CONFIGURATION_DEFAULT_PATH;

			if (configuration_path_from_library_path(library_relative_path, configuration_default_path_win32, sizeof(configuration_default_path_win32)) == 0)
			{
				global = configuration_object_initialize(CONFIGURATION_GLOBAL_SCOPE, library_relative_path, NULL);

				path = library_relative_path;
			}
		}
#endif

		if (global == NULL)
		{
			static const char relative_path[] = ".." ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR "share" ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR "metacall" ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR CONFIGURATION_DEFAULT_PATH;

			if (configuration_path_from_library_path(library_relative_path, relative_path, sizeof(relative_path)) == 0)
			{
				global = configuration_object_initialize(CONFIGURATION_GLOBAL_SCOPE, library_relative_path, NULL);

				path = library_relative_path;
			}
		}

#if defined(CONFIGURATION_INSTALL_PATH)
		if (global == NULL)
		{
			static const char configuration_install_path[] = CONFIGURATION_INSTALL_PATH;

			global = configuration_object_initialize(CONFIGURATION_GLOBAL_SCOPE, configuration_install_path, NULL);

			path = configuration_install_path;
		}
#endif /* CONFIGURATION_INSTALL_PATH */

		if (global != NULL && path != NULL)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Global configuration loaded from %s", path);
		}
	}
	else
	{
		global = configuration_object_initialize(CONFIGURATION_GLOBAL_SCOPE, path, NULL);

		log_write("metacall", LOG_LEVEL_DEBUG, "Global configuration loaded from %s", path);
	}

	if (global == NULL)
	{
		log_write("metacall", LOG_LEVEL_WARNING, "MetaCall could not find the global configuration, "
												 "either use a relative path configuration '" CONFIGURATION_DEFAULT_PATH "' or define the environment "
												 "variable '" CONFIGURATION_PATH "', MetaCall will use a default configuration");

		global = configuration_object_initialize(CONFIGURATION_GLOBAL_SCOPE, NULL, NULL);

		if (global == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Default global configuration failed to initialize");

			goto configuration_error;
		}
	}

	if (configuration_singleton_initialize(global) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration singleton initialization");

		goto configuration_error;
	}

	if (configuration_impl_initialize(reader) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation initialization");

		goto configuration_error;
	}

	if (configuration_impl_load(global, allocator) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load <%p>", global);

		goto configuration_error;
	}

	return 0;

configuration_error:
	configuration_destroy();

	return 1;
}

configuration configuration_create(const char *scope, const char *path, const char *parent, void *allocator)
{
	configuration config = configuration_singleton_get(scope);

	if (config != NULL)
	{
		return config;
	}

	config = configuration_object_initialize(scope, path, configuration_scope(parent));

	if (config == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration (%s) scope map creation [%s]", scope, path);

		return NULL;
	}

	if (configuration_impl_load(config, allocator) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load <%p>", config);

		configuration_object_destroy(config);

		return NULL;
	}

	return config;
}

configuration configuration_scope(const char *name)
{
	if (name == NULL)
	{
		return NULL;
	}

	configuration config = configuration_singleton_get(name);

	if (config != NULL)
	{
		return config;
	}

	/* Insert a default configuration in this scope */
	config = configuration_object_initialize(name, NULL, configuration_singleton_get(CONFIGURATION_GLOBAL_SCOPE));

	if (config == NULL)
	{
		return NULL;
	}

	if (configuration_singleton_register(config) != 0)
	{
		configuration_object_destroy(config);
		return NULL;
	}

	return config;
}

value configuration_value(configuration config, const char *key)
{
	if (config == NULL)
	{
		return NULL;
	}

	return configuration_object_get(config, key);
}

value configuration_value_type(configuration config, const char *key, type_id id)
{
	if (config == NULL)
	{
		return NULL;
	}

	value v = configuration_object_get(config, key);

	if (v == NULL)
	{
		return NULL;
	}

	if (value_type_id(v) != id)
	{
		return NULL;
	}

	return v;
}

int configuration_define(configuration config, const char *key, value v)
{
	return configuration_object_set(config, key, v);
}

int configuration_undefine(configuration config, const char *key)
{
	return configuration_object_remove(config, key);
}

int configuration_clear(configuration config)
{
	int result = configuration_singleton_clear(config);

	configuration_object_destroy(config);

	return result;
}

void configuration_destroy(void)
{
	configuration_singleton_destroy();

	configuration_impl_destroy();

	serial_destroy();
}

const char *configuration_print_info(void)
{
	static const char configuration_info[] =
		"Configuration Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef CONFIGURATION_STATIC_DEFINE
		"Compiled as static library type"
#else
		"Compiled as shared library type"
#endif

		"\n";

	return configuration_info;
}
