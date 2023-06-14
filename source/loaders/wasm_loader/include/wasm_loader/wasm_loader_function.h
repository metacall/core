/*
 *	Loader Library by Parra Studios
 *	A plugin for loading WebAssembly code at run-time into a process.
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

#ifndef WASM_LOADER_FUNCTION_H
#define WASM_LOADER_FUNCTION_H 1

#include <wasm_loader/wasm_loader_api.h>

#include <reflect/reflect_function.h>

#if defined(WASMTIME) && defined(_WIN32) && defined(_MSC_VER)
	#define WASM_API_EXTERN
#endif
#include <wasm.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct loader_impl_wasm_function_type *loader_impl_wasm_function;

WASM_LOADER_API function_interface function_wasm_singleton(void);

WASM_LOADER_API loader_impl_wasm_function loader_impl_wasm_function_create(const wasm_func_t *func, size_t args_size);

#ifdef __cplusplus
}
#endif

#endif /* WASM_LOADER_FUNCTION_H */
