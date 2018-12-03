/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading javascript code at run-time into a process.
 *
 */

#ifndef JSM_LOADER_H
#define JSM_LOADER_H 1

#include <jsm_loader/jsm_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

JSM_LOADER_API loader_impl_interface jsm_loader_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(jsm_loader_impl_interface_singleton);

JSM_LOADER_API const char * jsm_loader_print_info(void);

DYNLINK_SYMBOL_EXPORT(jsm_loader_print_info);

#ifdef __cplusplus
}
#endif

#endif /* JSM_LOADER_H */
