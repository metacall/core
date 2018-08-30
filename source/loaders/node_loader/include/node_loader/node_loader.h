/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading nodejs code at run-time into a process.
 *
 */

#ifndef NODE_LOADER_H
#define NODE_LOADER_H 1

#include <node_loader/node_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

NODE_LOADER_API loader_impl_interface node_loader_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(node_loader_impl_interface_singleton);

NODE_LOADER_API const char * node_loader_print_info(void);

DYNLINK_SYMBOL_EXPORT(node_loader_print_info);

#ifdef __cplusplus
}
#endif

#endif /* NODE_LOADER_H */
