/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading c code at run-time into a process.
 *
 */

#ifndef C_LOADER_H
#define C_LOADER_H 1

#include <c_loader/c_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

C_LOADER_API loader_impl_interface c_loader_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(c_loader_impl_interface_singleton);

C_LOADER_API void c_loader_print_info(void);

DYNLINK_SYMBOL_EXPORT(c_loader_print_info);

#ifdef __cplusplus
}
#endif

#endif /* C_LOADER_H */
