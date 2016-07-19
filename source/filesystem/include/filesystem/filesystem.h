/*
 *	File System Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing file system, paths and files.
 *
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H 1

/* -- Headers -- */

#include <filesystem/filesystem_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Print over standard output the module information
*
*/
FILESYSTEM_API void filesystem_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* FILESYSTEM_H */
