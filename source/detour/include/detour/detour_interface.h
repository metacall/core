/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

#ifndef DETOUR_INTERFACE_H
#define DETOUR_INTERFACE_H 1

/* -- Headers -- */

#include <detour/detour_api.h>

#include <detour/detour_impl_handle.h>
#include <detour/detour_host.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct detour_interface_type;

/* -- Type Definitions -- */

typedef detour_impl_handle (*detour_interface_initialize)(detour_host);

typedef int (*detour_interface_install)(detour_impl_handle, void(**)(void), void(*)(void));

typedef int (*detour_interface_uninstall)(detour_impl_handle);

typedef int (*detour_interface_destroy)(detour_impl_handle);

typedef struct detour_interface_type * detour_interface;

typedef detour_interface (*detour_interface_singleton)(void);

/* -- Member Data -- */

struct detour_interface_type
{
	detour_interface_initialize initialize;
	detour_interface_install install;
	detour_interface_uninstall uninstall;
	detour_interface_destroy destroy;
};

#ifdef __cplusplus
}
#endif

#endif /* DETOUR_INTERFACE_H */
