/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading mock code at run-time into a process.
 *
 */

#ifndef MOCK_LOADER_DESCRIPTOR_H
#define MOCK_LOADER_DESCRIPTOR_H 1

#include <mock_loader/mock_loader_api.h>

#include <loader/loader_impl_descriptor.h>

#ifdef __cplusplus
extern "C" {
#endif

MOCK_LOADER_API loader_impl_descriptor mock_loader_impl_descriptor_singleton(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_LOADER_DESCRIPTOR_H */
