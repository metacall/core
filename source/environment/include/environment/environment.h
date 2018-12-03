/*
 *	Environment Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for supporting platform specific environment features.
 *
 */

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H 1

/* -- Headers -- */

#include <environment/environment_api.h>

#include <environment/environment_variable.h>
#include <environment/environment_variable_path.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

ENVIRONMENT_API const char * environment_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* ENVIRONMENT_H */
