/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading ruby code at run-time into a process.
 *
 */

#ifndef RB_LOADER_H
#define RB_LOADER_H 1

#include <rb_loader/rb_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

RB_LOADER_API loader_impl_interface rb_loader_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(rb_loader_impl_interface_singleton);

RB_LOADER_API const char * rb_loader_print_info(void);

DYNLINK_SYMBOL_EXPORT(rb_loader_print_info);

#ifdef __cplusplus
}
#endif

#endif /* RB_LOADER_H */
