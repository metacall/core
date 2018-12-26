/*
 *	Environment Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for supporting platform specific environment features.
 *
 */

#ifndef ENVIRONMENT_VARIABLE_PATH_H
#define ENVIRONMENT_VARIABLE_PATH_H 1

/* -- Headers -- */

#include <environment/environment_api.h>

#ifdef __cplusplus
extern "C" {
#endif


/* -- Definitions -- */

#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)
#	define ENVIRONMENT_VARIABLE_PATH_SEPARATOR_C '/'
#	define ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR "/"
#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
#	define ENVIRONMENT_VARIABLE_PATH_SEPARATOR_C '/'
#	define ENVIRONMENT_VARIABLE_PATH_SEPARATOR_STR "/"
#else
#	error "Unknown environment variable path separator"
#endif

/* -- Methods -- */

ENVIRONMENT_API char * environment_variable_path_create(const char * name, const char * default_path);

ENVIRONMENT_API void environment_variable_path_destroy(char * variable_path);

#ifdef __cplusplus
}
#endif

#endif /* ENVIRONMENT_VARIABLE_PATH_H */
