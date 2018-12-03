/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading javascript code at run-time into a process.
 *
 */

#ifndef JS_LOADER_H
#define JS_LOADER_H 1

#include <js_loader/js_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

JS_LOADER_API loader_impl_interface js_loader_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(js_loader_impl_interface_singleton);

JS_LOADER_API const char * js_loader_print_info(void);

DYNLINK_SYMBOL_EXPORT(js_loader_print_info);

#ifdef __cplusplus
}
#endif

#endif /* JS_LOADER_H */
