/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading python code at run-time into a process.
 *
 */

#ifndef PY_LOADER_H
#define PY_LOADER_H 1

#include <py_loader/py_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

PY_LOADER_API loader_impl_interface py_loader_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(py_loader_impl_interface_singleton);

PY_LOADER_API const char * py_loader_print_info(void);

DYNLINK_SYMBOL_EXPORT(py_loader_print_info);

#ifdef __cplusplus
}
#endif

#endif /* PY_LOADER_H */
