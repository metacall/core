/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef LOADER_IMPL_HANDLE_H
#define LOADER_IMPL_HANDLE_H 1

#include <loader/loader_handle.h>
#include <loader/loader_impl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef loader_handle (*loader_impl_handle_create)(loader_impl, const loader_naming_path, const loader_naming_name);

typedef int (*loader_impl_handle_discover)(loader_impl, loader_handle, context);

typedef int (*loader_impl_handle_destroy)(loader_impl, loader_handle);

typedef struct loader_impl_handle_type
{
	loader_impl_handle_create create;
	loader_impl_handle_discover discover;
	loader_impl_handle_destroy destroy;

} * loader_impl_handle;

typedef loader_impl_handle (*loader_impl_handle_singleton)(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_HANDLE_H */
