#ifndef LOADER_IMPL_H
#define LOADER_IMPL_H

#include <loader/loader_handle.h>

typedef int (*loader_impl_initialize)(void);

typedef int (*loader_impl_execution_path)(char *);

typedef loader_handle (*loader_impl_load)(char *);

typedef int (*loader_impl_clear)(loader_handle);

typedef int (*loader_impl_destroy)(void);

typedef struct loader_impl_type
{
	loader_impl_initialize initialize;
	loader_impl_execution_path execution_path;
	loader_impl_load load;
	loader_impl_clear clear;
	loader_impl_destroy destroy;

} * loader_impl;

typedef loader_impl (*loader_impl_singleton)(void);

#endif // LOADER_IMPL_H
