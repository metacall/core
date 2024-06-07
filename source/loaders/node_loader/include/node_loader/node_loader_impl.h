/*
 *	Loader Library by Parra Studios
 *	A plugin for loading nodejs code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef NODE_LOADER_IMPL_H
#define NODE_LOADER_IMPL_H 1

#include <node_loader/node_loader_api.h>

#include <configuration/configuration.h>
#include <loader/loader_impl_interface.h>

#include <node_api.h>

#ifdef __cplusplus
extern "C" {
#endif

struct loader_impl_node_type;
typedef struct loader_impl_node_type *loader_impl_node;

NODE_LOADER_API loader_impl_data node_loader_impl_initialize(loader_impl impl, configuration config);

NODE_LOADER_API int node_loader_impl_execution_path(loader_impl impl, const loader_path path);

NODE_LOADER_API loader_handle node_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size);

NODE_LOADER_API loader_handle node_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size);

NODE_LOADER_API loader_handle node_loader_impl_load_from_package(loader_impl impl, const loader_path path);

NODE_LOADER_API int node_loader_impl_clear(loader_impl impl, loader_handle handle);

NODE_LOADER_API int node_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx);

NODE_LOADER_API int node_loader_impl_destroy(loader_impl impl);

NODE_LOADER_NO_EXPORT void node_loader_impl_exception(napi_env env, napi_status status);

NODE_LOADER_NO_EXPORT void node_loader_impl_finalizer(napi_env env, napi_value v, void *data);

NODE_LOADER_NO_EXPORT napi_value node_loader_impl_promise_await(loader_impl_node node_impl, napi_env env, const char *name, value *args, size_t size);

NODE_LOADER_NO_EXPORT value node_loader_impl_napi_to_value(loader_impl_node node_impl, napi_env env, napi_value recv, napi_value v);

NODE_LOADER_NO_EXPORT napi_value node_loader_impl_value_to_napi(loader_impl_node node_impl, napi_env env, value arg);

NODE_LOADER_NO_EXPORT void node_loader_impl_env(loader_impl_node node_impl, napi_env env);

NODE_LOADER_NO_EXPORT void node_loader_impl_destroy_safe_impl(loader_impl_node node_impl, napi_env env);

NODE_LOADER_NO_EXPORT void node_loader_impl_print_handles(loader_impl_node node_impl);

NODE_LOADER_NO_EXPORT int64_t node_loader_impl_user_async_handles_count(loader_impl_node node_impl);

#ifdef __cplusplus
}
#endif

#endif /* NODE_LOADER_IMPL_H */
