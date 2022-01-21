/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#include <loader/loader_env.h>

#include <environment/environment_variable_path.h>

#include <log/log.h>

/* -- Definitions -- */

#define LOADER_LIBRARY_PATH			"LOADER_LIBRARY_PATH"
#define LOADER_LIBRARY_DEFAULT_PATH "loaders"

#define LOADER_SCRIPT_PATH		   "LOADER_SCRIPT_PATH"
#define LOADER_SCRIPT_DEFAULT_PATH "."

/* -- Private Data -- */

static char *script_path = NULL;
static char *library_path = NULL;

/* -- Methods -- */

void loader_env_initialize(void)
{
	if (library_path == NULL)
	{
#if defined(LOADER_LIBRARY_INSTALL_PATH)
		static const char loader_library_default_path[] = LOADER_LIBRARY_INSTALL_PATH;
#else
		static const char loader_library_default_path[] = LOADER_LIBRARY_DEFAULT_PATH;
#endif /* LOADER_LIBRARY_INSTALL_PATH */

		library_path = environment_variable_path_create(LOADER_LIBRARY_PATH, loader_library_default_path);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader library path: %s", library_path);
	}

	if (script_path == NULL)
	{
		static const char loader_script_default_path[] = LOADER_SCRIPT_DEFAULT_PATH;

		script_path = environment_variable_path_create(LOADER_SCRIPT_PATH, loader_script_default_path);

		log_write("metacall", LOG_LEVEL_DEBUG, "Loader script path: %s", script_path);
	}
}

const char *loader_env_script_path(void)
{
	return script_path;
}

const char *loader_env_library_path(void)
{
	return library_path;
}

void loader_env_destroy(void)
{
	if (library_path != NULL)
	{
		environment_variable_path_destroy(library_path);

		library_path = NULL;
	}

	if (script_path != NULL)
	{
		environment_variable_path_destroy(script_path);

		script_path = NULL;
	}
}
