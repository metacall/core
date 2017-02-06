/*
 *	Environment Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/* -- Methods -- */

ENVIRONMENT_API char * environment_variable_path_create(const char * name, const char * default_path);

ENVIRONMENT_API void environment_variable_path_destroy(char * variable_path);

#ifdef __cplusplus
}
#endif

#endif /* ENVIRONMENT_VARIABLE_PATH_H */
