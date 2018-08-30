/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading file code at run-time into a process.
 *
 */

#ifndef FILE_LOADER_H
#define FILE_LOADER_H 1

#include <file_loader/file_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

FILE_LOADER_API loader_impl_interface file_loader_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(file_loader_impl_interface_singleton);

FILE_LOADER_API const char * file_loader_print_info(void);

DYNLINK_SYMBOL_EXPORT(file_loader_print_info);

#ifdef __cplusplus
}
#endif

#endif /* FILE_LOADER_H */
