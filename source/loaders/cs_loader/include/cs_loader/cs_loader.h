/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading net code at run-time into a process.
 *
 */

#ifndef CS_LOADER_H
#define CS_LOADER_H 1

#include <cs_loader/cs_loader_api.h>

#include <loader/loader_impl_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

CS_LOADER_API loader_impl_interface cs_loader_impl_interface_singleton(void);

CS_LOADER_API const char *cs_loader_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* CS_LOADER_H */
