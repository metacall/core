/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <metacall/metacall-version.h>

#include <configuration/configuration.h>
#include <configuration/configuration_singleton.h>
#include <configuration/configuration_impl.h>

#include <environment/environment_variable.h>
#include <environment/environment_variable_path.h>

#include <log/log.h>

/* -- Definitions -- */

#define CONFIGURATION_PATH			"CONFIGURATION_PATH"
#define CONFIGURATION_DEFAULT_PATH	"configurations" ENVIRONMENT_VARIABLE_PATH_SEPARATOR "global.json"

/* -- Methods -- */

int configuration_initialize(const char * name, const char * path)
{
	configuration global = NULL;

	if (path == NULL)
	{
		static const char configuration_path[] = CONFIGURATION_PATH;

		const char * env_path = environment_variable_get(configuration_path, CONFIGURATION_DEFAULT_PATH);

		global = configuration_object_initialize("global", env_path, NULL);
	}
	else
	{
		global = configuration_object_initialize("global", path, NULL);
	}

	if (global == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration global scope map creation");

		configuration_destroy();

		return 1;
	}

	if (configuration_singleton_initialize(global) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration singleton initialization");

		configuration_destroy();

		return 1;
	}

	if (configuration_impl_initialize(name) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation initialization");

		configuration_destroy();

		return 1;
	}

	if (configuration_impl_load(global) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load <%p>", global);

		configuration_destroy();

		return 1;
	}

	return 0;
}

configuration configuration_scope(const char * name)
{
	return configuration_singleton_get(name);
}

value configuration_value(configuration config, const char * key)
{
	return configuration_object_get(config, key);
}

void configuration_destroy()
{
	configuration_singleton_destroy();

	configuration_impl_destroy();
}

const char * configuration_print_info()
{
	static const char configuration_info[] =
		"Configuration Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef CONFIGURATION_STATIC_DEFINE
			"Compiled as static library type"
		#else
			"Compiled as shared library type"
		#endif

		"\n";

	return configuration_info;
}
