/************************************************************************/
/*	Loader Library by Parra Studios										*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A library for loading executable code at run-time into a process.	*/
/*																		*/
/************************************************************************/

#ifndef LOADER_IMPL_H
#define LOADER_IMPL_H 1

#include <loader/loader_api.h>

#include <loader/loader_naming.h>
#include <loader/loader_impl_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

struct context_type;

typedef struct context_type * context;

typedef struct loader_impl_type * loader_impl;

LOADER_API loader_impl loader_impl_create(loader_naming_extension extension);

LOADER_API loader_impl_interface loader_impl_symbol(loader_impl impl);

LOADER_API loader_naming_extension * loader_impl_extension(loader_impl impl);

LOADER_API int loader_impl_load(loader_impl impl, loader_naming_name name);

LOADER_API void loader_impl_destroy(loader_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_H */
