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

#ifndef LOADER_IMPL_FUNCTION_H
#define LOADER_IMPL_FUNCTION_H 1

#include <loader/loader_function.h>
#include <loader/loader_handle.h>
#include <loader/loader_impl.h>

#include <reflect/function.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef loader_function (*loader_impl_function_create)(loader_impl, loader_handle);

typedef function_impl_interface_singleton (*loader_impl_function_interface)(void);

typedef int (*loader_impl_function_destroy)(loader_impl, loader_handle, loader_function);

typedef struct loader_impl_function_type
{
	loader_impl_function_create create;
	loader_impl_function_interface interface;
	loader_impl_function_destroy destroy;

} * loader_impl_function;

typedef loader_impl_function (*loader_impl_function_singleton)(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_FUNCTION_H */
