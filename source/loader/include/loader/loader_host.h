/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef LOADER_HOST_H
#define LOADER_HOST_H 1

/* -- Headers -- */

#include <loader/loader_api.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_context.h>

#include <portability/portability_assert.h>

#include <plugin/plugin_impl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods  -- */

LOADER_API plugin loader_host_initialize(void);

LOADER_API int loader_host_register(loader_impl host, context ctx, const char *name, loader_register_invoke invoke, function *func, type_id return_type, size_t arg_size, type_id args_type_id[]);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_HOST_H */
