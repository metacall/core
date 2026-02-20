/*
 *	Loader Library by Parra Studios
 *	A plugin for loading Haskell code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef HS_LOADER_IMPL_H
#define HS_LOADER_IMPL_H 1

#include <hs_loader/hs_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <configuration/configuration.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Haskell bridge ABI expected from each loaded module:
 *  - metacall_hs_module_function_count
 *  - metacall_hs_module_function_name
 *  - metacall_hs_module_function_args_count
 *  - metacall_hs_module_function_arg_name
 *  - metacall_hs_module_function_arg_type
 *  - metacall_hs_module_function_return_type
 *  - metacall_hs_module_invoke
 *  - metacall_hs_module_value_destroy
 *
 * Optional:
 *  - metacall_hs_module_initialize
 *  - metacall_hs_module_destroy
 *  - metacall_hs_module_last_error
 */
typedef struct hs_loader_ffi_bytes_type
{
	const void *data;
	size_t size;

} hs_loader_ffi_bytes;

typedef struct hs_loader_ffi_value_type
{
	int type;

	union
	{
		uint8_t v_bool;
		char v_char;
		short v_short;
		int v_int;
		long v_long;
		float v_float;
		double v_double;
		void *v_ptr;
		hs_loader_ffi_bytes v_string;
		hs_loader_ffi_bytes v_buffer;

	} data;

} hs_loader_ffi_value;

typedef int (*hs_loader_module_initialize_ptr)(void);
typedef void (*hs_loader_module_destroy_ptr)(void);
typedef size_t (*hs_loader_module_function_count_ptr)(void);
typedef const char *(*hs_loader_module_function_name_ptr)(size_t);
typedef size_t (*hs_loader_module_function_args_count_ptr)(size_t);
typedef const char *(*hs_loader_module_function_arg_name_ptr)(size_t, size_t);
typedef int (*hs_loader_module_function_arg_type_ptr)(size_t, size_t);
typedef int (*hs_loader_module_function_return_type_ptr)(size_t);
typedef int (*hs_loader_module_invoke_ptr)(size_t, const hs_loader_ffi_value *, size_t, hs_loader_ffi_value *);
typedef void (*hs_loader_module_value_destroy_ptr)(hs_loader_ffi_value *);
typedef const char *(*hs_loader_module_last_error_ptr)(void);
typedef void (*hs_loader_runtime_initialize_ptr)(int *, char ***);
typedef void (*hs_loader_runtime_destroy_ptr)(void);

/*
 * Optional host-side callback for load-from-memory.
 * It must compile/emit a loadable Haskell shared object and return the path.
 */
typedef int (*hs_loader_load_from_memory_bridge_ptr)(const loader_name, const char *, size_t, loader_path, size_t);

HS_LOADER_API loader_impl_data hs_loader_impl_initialize(loader_impl impl, configuration config);

HS_LOADER_API int hs_loader_impl_execution_path(loader_impl impl, const loader_path path);

HS_LOADER_API loader_handle hs_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size);

HS_LOADER_API loader_handle hs_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size);

HS_LOADER_API loader_handle hs_loader_impl_load_from_package(loader_impl impl, const loader_path path);

HS_LOADER_API int hs_loader_impl_clear(loader_impl impl, loader_handle handle);

HS_LOADER_API int hs_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx);

HS_LOADER_API int hs_loader_impl_destroy(loader_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* HS_LOADER_IMPL_H */
