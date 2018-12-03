/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_IMPL_PRINT_H
#define LOADER_IMPL_PRINT_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*loader_impl_print_info)(void);

typedef struct loader_impl_print_type
{
	loader_impl_print_info print_info;

} * loader_impl_print;

typedef loader_impl_print (*loader_impl_print_singleton)(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_PRINT_H */
