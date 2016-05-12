/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_NAMING_TYPE_H
#define LOADER_NAMING_TYPE_H 1

#include <loader/loader_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOADER_NAMING_PATH_SIZE			0x01FF
#define LOADER_NAMING_NAME_SIZE			0xFF
#define LOADER_NAMING_EXTENSION_SIZE	0x10

typedef char loader_naming_path[LOADER_NAMING_PATH_SIZE];
typedef char loader_naming_name[LOADER_NAMING_NAME_SIZE];
typedef char loader_naming_extension[LOADER_NAMING_EXTENSION_SIZE];

#ifdef __cplusplus
}
#endif

#endif /* LOADER_NAMING_TYPE_H */
