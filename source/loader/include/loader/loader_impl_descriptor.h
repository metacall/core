/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_IMPL_DESCRIPTOR_H
#define LOADER_IMPL_DESCRIPTOR_H 1

#include <loader/loader_descriptor.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef loader_descriptor (*loader_impl_descriptor_instance)(void);

typedef struct loader_impl_descriptor_type
{
	loader_impl_descriptor_instance descriptor;

} * loader_impl_descriptor;

typedef loader_impl_descriptor (*loader_impl_descriptor_singleton)(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_DESCRIPTOR_H */
