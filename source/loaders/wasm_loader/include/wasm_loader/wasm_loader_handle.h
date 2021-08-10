/*
 *	Loader Library by Parra Studios
 *	A plugin for loading WebAssembly code at run-time into a process.
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

#ifndef WASM_LOADER_HANDLE_H
#define WASM_LOADER_HANDLE_H 1

#include <loader/loader.h>

#include <wasm_loader/wasm_loader_api.h>

#include <adt/adt_vector.h>

#include <wasm.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct loader_impl_wasm_handle_type *loader_impl_wasm_handle;
typedef struct loader_impl_wasm_module_type loader_impl_wasm_module;

WASM_LOADER_API loader_impl_wasm_handle wasm_loader_handle_create(size_t num_modules);
WASM_LOADER_API void wasm_loader_handle_destroy(loader_impl_wasm_handle handle);
WASM_LOADER_API int wasm_loader_handle_add_module(loader_impl_wasm_handle handle, const loader_naming_name name, wasm_store_t *store, const wasm_byte_vec_t *binary);
WASM_LOADER_API int wasm_loader_handle_discover(loader_impl impl, loader_impl_wasm_handle handle, scope scp);

#ifdef __cplusplus
}
#endif

#endif /* WASM_LOADER_HANDLE_H */
