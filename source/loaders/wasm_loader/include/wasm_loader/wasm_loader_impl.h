/*
 *	Loader Library by Parra Studios
 *	A plugin for loading WebAssembly code at run-time into a process.
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

#ifndef WASM_LOADER_IMPL_H
#define WASM_LOADER_IMPL_H 1

#include <wasm_loader/wasm_loader_api.h>

#include <loader/loader_impl_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

WASM_LOADER_API loader_impl_data wasm_loader_impl_initialize(loader_impl impl, configuration config);
WASM_LOADER_API int wasm_loader_impl_execution_path(loader_impl impl, const loader_path path);
WASM_LOADER_API loader_handle wasm_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size);
WASM_LOADER_API loader_handle wasm_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size);
WASM_LOADER_API loader_handle wasm_loader_impl_load_from_package(loader_impl impl, const loader_path path);
WASM_LOADER_API int wasm_loader_impl_clear(loader_impl impl, loader_handle handle);
WASM_LOADER_API int wasm_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx);
WASM_LOADER_API int wasm_loader_impl_destroy(loader_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* WASM_LOADER_IMPL_H */
