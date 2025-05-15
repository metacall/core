/*
 *	Detour Library by Parra Studios
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#ifndef DETOUR_INTERFACE_H
#define DETOUR_INTERFACE_H 1

/* -- Headers -- */

#include <detour/detour_api.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct detour_interface_type;

/* -- Type Definitions -- */

typedef void *detour_impl_handle;
typedef struct detour_interface_type *detour_interface;

/* -- Member Data -- */

struct detour_interface_type
{
	int (*initialize_file)(detour_impl_handle *, const char *);
	int (*initialize_handle)(detour_impl_handle *, dynlink);
	int (*initialize_address)(detour_impl_handle *, void (*)(void));
	int (*enumerate)(detour_impl_handle, unsigned int *, const char **, void ***);
	int (*replace)(detour_impl_handle, const char *, void (*)(void), void **);
	const char *(*error)(detour_impl_handle);
	void (*destroy)(detour_impl_handle);
};

#ifdef __cplusplus
}
#endif

#endif /* DETOUR_INTERFACE_H */
