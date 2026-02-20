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

#include <hs_loader/hs_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <portability/portability_path.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>
#include <reflect/reflect_value_type.h>

#include <adt/adt_vector.h>

#include <dynlink/dynlink.h>

#include <log/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct loader_impl_hs_bridge_type
{
	hs_loader_module_initialize_ptr initialize;
	hs_loader_module_destroy_ptr destroy;
	hs_loader_module_function_count_ptr function_count;
	hs_loader_module_function_name_ptr function_name;
	hs_loader_module_function_args_count_ptr function_args_count;
	hs_loader_module_function_arg_name_ptr function_arg_name;
	hs_loader_module_function_arg_type_ptr function_arg_type;
	hs_loader_module_function_return_type_ptr function_return_type;
	hs_loader_module_invoke_ptr invoke;
	hs_loader_module_value_destroy_ptr value_destroy;
	hs_loader_module_last_error_ptr last_error;

} loader_impl_hs_bridge;

typedef struct loader_impl_hs_module_type
{
	loader_path path;
	dynlink lib;
	loader_impl_hs_bridge bridge;
	hs_loader_runtime_initialize_ptr runtime_initialize;
	hs_loader_runtime_destroy_ptr runtime_destroy;
	int runtime_attached;

} loader_impl_hs_module;

typedef struct loader_impl_hs_type
{
	vector execution_paths;
	hs_loader_load_from_memory_bridge_ptr load_from_memory_bridge;

} * loader_impl_hs;

typedef struct loader_impl_hs_handle_type
{
	vector modules;

} * loader_impl_hs_handle;

typedef struct loader_impl_hs_function_type
{
	loader_impl_hs_module *module;
	size_t function_index;

} * loader_impl_hs_function;

typedef union loader_impl_hs_symbol_cast_type
{
	dynlink_symbol_addr addr;
	hs_loader_module_initialize_ptr initialize;
	hs_loader_module_destroy_ptr destroy;
	hs_loader_module_function_count_ptr function_count;
	hs_loader_module_function_name_ptr function_name;
	hs_loader_module_function_args_count_ptr function_args_count;
	hs_loader_module_function_arg_name_ptr function_arg_name;
	hs_loader_module_function_arg_type_ptr function_arg_type;
	hs_loader_module_function_return_type_ptr function_return_type;
	hs_loader_module_invoke_ptr invoke;
	hs_loader_module_value_destroy_ptr value_destroy;
	hs_loader_module_last_error_ptr last_error;
	hs_loader_load_from_memory_bridge_ptr load_from_memory_bridge;
	hs_loader_runtime_initialize_ptr runtime_initialize;
	hs_loader_runtime_destroy_ptr runtime_destroy;

} loader_impl_hs_symbol_cast;

static const char hs_loader_module_initialize_symbol[] = "metacall_hs_module_initialize";
static const char hs_loader_module_destroy_symbol[] = "metacall_hs_module_destroy";
static const char hs_loader_module_function_count_symbol[] = "metacall_hs_module_function_count";
static const char hs_loader_module_function_name_symbol[] = "metacall_hs_module_function_name";
static const char hs_loader_module_function_args_count_symbol[] = "metacall_hs_module_function_args_count";
static const char hs_loader_module_function_arg_name_symbol[] = "metacall_hs_module_function_arg_name";
static const char hs_loader_module_function_arg_type_symbol[] = "metacall_hs_module_function_arg_type";
static const char hs_loader_module_function_return_type_symbol[] = "metacall_hs_module_function_return_type";
static const char hs_loader_module_invoke_symbol[] = "metacall_hs_module_invoke";
static const char hs_loader_module_value_destroy_symbol[] = "metacall_hs_module_value_destroy";
static const char hs_loader_module_last_error_symbol[] = "metacall_hs_module_last_error";
static const char hs_loader_load_from_memory_symbol[] = "metacall_hs_loader_load_from_memory";
static const char hs_loader_runtime_initialize_symbol[] = "hs_init";
static const char hs_loader_runtime_destroy_symbol[] = "hs_exit";

typedef struct hs_loader_runtime_state_type
{
	size_t references;

} hs_loader_runtime_state;

static hs_loader_runtime_state hs_loader_runtime = { 0 };

static int hs_loader_impl_valid_type_id(int id);
static int hs_loader_impl_initialize_types(loader_impl impl);
static int hs_loader_impl_symbol_required(dynlink lib, const char *symbol_name, dynlink_symbol_addr *symbol_address);
static int hs_loader_impl_symbol_optional(dynlink lib, const char *symbol_name, dynlink_symbol_addr *symbol_address);
static int hs_loader_impl_runtime_attach(loader_impl_hs_module *module);
static void hs_loader_impl_runtime_detach(loader_impl_hs_module *module);
static const char *hs_loader_impl_module_last_error(loader_impl_hs_module *module);
static void hs_loader_impl_module_unload(loader_impl_hs_module *module);
static int hs_loader_impl_module_load(const loader_path path, loader_impl_hs_module *module);
static int hs_loader_impl_resolve_path(loader_impl_hs hs_impl, const loader_path path, loader_path absolute_path);
static void hs_loader_impl_initialize_load_from_memory_bridge(loader_impl_hs hs_impl);
static int hs_loader_impl_value_to_ffi(value v, type_id id, hs_loader_ffi_value *ffi_value);
static value hs_loader_impl_value_from_ffi(const hs_loader_ffi_value *ffi_value);
static int hs_loader_impl_discover_module_function(loader_impl impl, loader_impl_hs_module *module, size_t function_index, scope sp);

int function_hs_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

function_return function_hs_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	loader_impl_hs_function hs_function = (loader_impl_hs_function)impl;
	hs_loader_ffi_value hs_return_value;
	hs_loader_ffi_value *hs_args = NULL;
	int invoke_result;
	size_t iterator;
	value ret;

	if (hs_function == NULL || hs_function->module == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Haskell function invocation failed because of invalid function state");
		return NULL;
	}

	memset(&hs_return_value, 0, sizeof(hs_return_value));
	hs_return_value.type = TYPE_INVALID;

	if (size > 0)
	{
		signature s = function_signature(func);

		hs_args = (hs_loader_ffi_value *)malloc(sizeof(hs_loader_ffi_value) * size);

		if (hs_args == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Haskell function invocation failed allocating argument bridge");
			return NULL;
		}

		memset(hs_args, 0, sizeof(hs_loader_ffi_value) * size);

		for (iterator = 0; iterator < size; ++iterator)
		{
			type t = signature_get_type(s, iterator);
			type_id id = (t == NULL) ? value_type_id((value)args[iterator]) : type_index(t);

			if (hs_loader_impl_value_to_ffi((value)args[iterator], id, &hs_args[iterator]) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Haskell function invocation failed converting argument %zu", iterator);
				free(hs_args);
				return NULL;
			}
		}
	}

	invoke_result = hs_function->module->bridge.invoke(hs_function->function_index, hs_args, size, &hs_return_value);

	if (hs_args != NULL)
	{
		free(hs_args);
	}

	if (invoke_result != 0)
	{
		const char *error_message = hs_loader_impl_module_last_error(hs_function->module);

		log_write("metacall", LOG_LEVEL_ERROR, "Haskell function invocation failed: %s", error_message);

		if (hs_function->module->bridge.value_destroy != NULL && hs_return_value.type != TYPE_INVALID)
		{
			hs_function->module->bridge.value_destroy(&hs_return_value);
		}

		return NULL;
	}

	ret = hs_loader_impl_value_from_ffi(&hs_return_value);

	if (hs_function->module->bridge.value_destroy != NULL && hs_return_value.type != TYPE_INVALID)
	{
		hs_function->module->bridge.value_destroy(&hs_return_value);
	}

	return ret;
}

function_return function_hs_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	(void)func;
	(void)impl;
	(void)args;
	(void)size;
	(void)resolve_callback;
	(void)reject_callback;
	(void)context;

	return NULL;
}

void function_hs_interface_destroy(function func, function_impl impl)
{
	loader_impl_hs_function hs_function = (loader_impl_hs_function)impl;

	(void)func;

	if (hs_function != NULL)
	{
		free(hs_function);
	}
}

function_interface function_hs_singleton(void)
{
	static struct function_interface_type hs_interface = {
		&function_hs_interface_create,
		&function_hs_interface_invoke,
		&function_hs_interface_await,
		&function_hs_interface_destroy
	};

	return &hs_interface;
}

static int hs_loader_impl_valid_type_id(int id)
{
	return id >= 0 && id < TYPE_SIZE;
}

static int hs_loader_impl_initialize_types(loader_impl impl)
{
	type_id index;

	for (index = 0; index < TYPE_SIZE; ++index)
	{
		type t = type_create(index, type_id_name(index), NULL, NULL);

		if (t == NULL)
		{
			return 1;
		}

		if (loader_impl_type_define(impl, type_name(t), t) != 0)
		{
			type_destroy(t);
			return 1;
		}
	}

	return 0;
}

static int hs_loader_impl_symbol_required(dynlink lib, const char *symbol_name, dynlink_symbol_addr *symbol_address)
{
	if (dynlink_symbol(lib, symbol_name, symbol_address) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Haskell module is missing required symbol: %s", symbol_name);
		return 1;
	}

	return 0;
}

static int hs_loader_impl_symbol_optional(dynlink lib, const char *symbol_name, dynlink_symbol_addr *symbol_address)
{
	if (dynlink_symbol(lib, symbol_name, symbol_address) != 0)
	{
		*symbol_address = NULL;
	}

	return 0;
}

static int hs_loader_impl_runtime_attach(loader_impl_hs_module *module)
{
	loader_impl_hs_symbol_cast cast;

	if (module == NULL || module->lib == NULL)
	{
		return 1;
	}

	if (hs_loader_impl_symbol_required(module->lib, hs_loader_runtime_initialize_symbol, &cast.addr) != 0)
	{
		return 1;
	}
	module->runtime_initialize = cast.runtime_initialize;

	if (hs_loader_impl_symbol_required(module->lib, hs_loader_runtime_destroy_symbol, &cast.addr) != 0)
	{
		module->runtime_initialize = NULL;
		return 1;
	}
	module->runtime_destroy = cast.runtime_destroy;

	if (hs_loader_runtime.references == 0)
	{
		int argc = 1;
		char *argv_values[2] = { "metacall-hs", NULL };
		char **argv = argv_values;

		module->runtime_initialize(&argc, &argv);
	}

	++hs_loader_runtime.references;
	module->runtime_attached = 1;

	return 0;
}

static void hs_loader_impl_runtime_detach(loader_impl_hs_module *module)
{
	if (module == NULL || module->runtime_attached == 0)
	{
		return;
	}

	if (hs_loader_runtime.references > 0)
	{
		--hs_loader_runtime.references;
	}

	if (hs_loader_runtime.references == 0 && module->runtime_destroy != NULL)
	{
		module->runtime_destroy();
	}

	module->runtime_attached = 0;
	module->runtime_initialize = NULL;
	module->runtime_destroy = NULL;
}

static const char *hs_loader_impl_module_last_error(loader_impl_hs_module *module)
{
	const char *error_message = NULL;

	if (module != NULL && module->bridge.last_error != NULL)
	{
		error_message = module->bridge.last_error();
	}

	return (error_message != NULL && error_message[0] != '\0') ? error_message : "unknown error";
}

static void hs_loader_impl_module_unload(loader_impl_hs_module *module)
{
	if (module == NULL)
	{
		return;
	}

	if (module->bridge.destroy != NULL)
	{
		module->bridge.destroy();
	}

	hs_loader_impl_runtime_detach(module);

	if (module->lib != NULL)
	{
		dynlink_unload(module->lib);
	}

	memset(module, 0, sizeof(loader_impl_hs_module));
}

static int hs_loader_impl_module_load(const loader_path path, loader_impl_hs_module *module)
{
	loader_impl_hs_symbol_cast cast;
	dynlink lib = dynlink_load_absolute(path, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
	size_t path_length;

	if (lib == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to load Haskell module from: %s", path);
		return 1;
	}

	memset(module, 0, sizeof(loader_impl_hs_module));

	module->lib = lib;

	path_length = strnlen(path, LOADER_PATH_SIZE - 1);
	memcpy(module->path, path, path_length);
	module->path[path_length] = '\0';

	if (hs_loader_impl_runtime_attach(module) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to initialize Haskell runtime for module: %s", module->path);
		goto error;
	}

	if (hs_loader_impl_symbol_required(lib, hs_loader_module_function_count_symbol, &cast.addr) != 0)
	{
		goto error;
	}
	module->bridge.function_count = cast.function_count;

	if (hs_loader_impl_symbol_required(lib, hs_loader_module_function_name_symbol, &cast.addr) != 0)
	{
		goto error;
	}
	module->bridge.function_name = cast.function_name;

	if (hs_loader_impl_symbol_required(lib, hs_loader_module_function_args_count_symbol, &cast.addr) != 0)
	{
		goto error;
	}
	module->bridge.function_args_count = cast.function_args_count;

	if (hs_loader_impl_symbol_required(lib, hs_loader_module_function_arg_name_symbol, &cast.addr) != 0)
	{
		goto error;
	}
	module->bridge.function_arg_name = cast.function_arg_name;

	if (hs_loader_impl_symbol_required(lib, hs_loader_module_function_arg_type_symbol, &cast.addr) != 0)
	{
		goto error;
	}
	module->bridge.function_arg_type = cast.function_arg_type;

	if (hs_loader_impl_symbol_required(lib, hs_loader_module_function_return_type_symbol, &cast.addr) != 0)
	{
		goto error;
	}
	module->bridge.function_return_type = cast.function_return_type;

	if (hs_loader_impl_symbol_required(lib, hs_loader_module_invoke_symbol, &cast.addr) != 0)
	{
		goto error;
	}
	module->bridge.invoke = cast.invoke;

	if (hs_loader_impl_symbol_required(lib, hs_loader_module_value_destroy_symbol, &cast.addr) != 0)
	{
		goto error;
	}
	module->bridge.value_destroy = cast.value_destroy;

	(void)hs_loader_impl_symbol_optional(lib, hs_loader_module_initialize_symbol, &cast.addr);
	module->bridge.initialize = cast.initialize;

	(void)hs_loader_impl_symbol_optional(lib, hs_loader_module_destroy_symbol, &cast.addr);
	module->bridge.destroy = cast.destroy;

	(void)hs_loader_impl_symbol_optional(lib, hs_loader_module_last_error_symbol, &cast.addr);
	module->bridge.last_error = cast.last_error;

	if (module->bridge.initialize != NULL && module->bridge.initialize() != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to initialize Haskell module: %s", module->path);
		goto error;
	}

	return 0;

error:
	hs_loader_impl_module_unload(module);
	return 1;
}

static int hs_loader_impl_resolve_path(loader_impl_hs hs_impl, const loader_path path, loader_path absolute_path)
{
	size_t path_size = strnlen(path, LOADER_PATH_SIZE) + 1;

	if (portability_path_is_absolute(path, path_size) == 0)
	{
		if (portability_path_file_exists(path) != 0)
		{
			return 1;
		}

		memcpy(absolute_path, path, path_size);

		return 0;
	}

	if (hs_impl != NULL && hs_impl->execution_paths != NULL)
	{
		size_t iterator, size = vector_size(hs_impl->execution_paths);

		for (iterator = 0; iterator < size; ++iterator)
		{
			loader_path *execution_path = vector_at(hs_impl->execution_paths, iterator);
			loader_path candidate_path;
			size_t execution_path_size = strnlen(*execution_path, LOADER_PATH_SIZE) + 1;
			size_t candidate_path_size = portability_path_join(*execution_path, execution_path_size, path, path_size, candidate_path, LOADER_PATH_SIZE);

			if (candidate_path_size == 0)
			{
				continue;
			}

			if (portability_path_file_exists(candidate_path) == 0)
			{
				memcpy(absolute_path, candidate_path, candidate_path_size);
				return 0;
			}
		}
	}

	return 1;
}

static void hs_loader_impl_initialize_load_from_memory_bridge(loader_impl_hs hs_impl)
{
	loader_impl_hs_symbol_cast cast;
	dynlink self = dynlink_load_self(DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_LOCAL);

	hs_impl->load_from_memory_bridge = NULL;

	if (self == NULL)
	{
		return;
	}

	if (dynlink_symbol(self, hs_loader_load_from_memory_symbol, &cast.addr) == 0)
	{
		hs_impl->load_from_memory_bridge = cast.load_from_memory_bridge;
	}

	dynlink_unload(self);
}

static int hs_loader_impl_value_to_ffi(value v, type_id id, hs_loader_ffi_value *ffi_value)
{
	if (ffi_value == NULL)
	{
		return 1;
	}

	memset(ffi_value, 0, sizeof(hs_loader_ffi_value));

	switch (id)
	{
		case TYPE_BOOL:
			ffi_value->type = TYPE_BOOL;
			ffi_value->data.v_bool = (uint8_t)(value_to_bool(v) != 0);
			return 0;

		case TYPE_CHAR:
			ffi_value->type = TYPE_CHAR;
			ffi_value->data.v_char = value_to_char(v);
			return 0;

		case TYPE_SHORT:
			ffi_value->type = TYPE_SHORT;
			ffi_value->data.v_short = value_to_short(v);
			return 0;

		case TYPE_INT:
			ffi_value->type = TYPE_INT;
			ffi_value->data.v_int = value_to_int(v);
			return 0;

		case TYPE_LONG:
			ffi_value->type = TYPE_LONG;
			ffi_value->data.v_long = value_to_long(v);
			return 0;

		case TYPE_FLOAT:
			ffi_value->type = TYPE_FLOAT;
			ffi_value->data.v_float = value_to_float(v);
			return 0;

		case TYPE_DOUBLE:
			ffi_value->type = TYPE_DOUBLE;
			ffi_value->data.v_double = value_to_double(v);
			return 0;

		case TYPE_STRING:
		{
			char *str = value_to_string(v);
			size_t string_size = value_type_size(v);

			ffi_value->type = TYPE_STRING;
			ffi_value->data.v_string.data = str;
			ffi_value->data.v_string.size = (str == NULL) ? 0 : strnlen(str, string_size);
			return 0;
		}

		case TYPE_BUFFER:
			ffi_value->type = TYPE_BUFFER;
			ffi_value->data.v_buffer.data = value_to_buffer(v);
			ffi_value->data.v_buffer.size = value_type_size(v);
			return 0;

		case TYPE_PTR:
			ffi_value->type = TYPE_PTR;
			ffi_value->data.v_ptr = value_to_ptr(v);
			return 0;

		case TYPE_NULL:
			ffi_value->type = TYPE_NULL;
			ffi_value->data.v_ptr = NULL;
			return 0;

		default:
			break;
	}

	return 1;
}

static value hs_loader_impl_value_from_ffi(const hs_loader_ffi_value *ffi_value)
{
	if (ffi_value == NULL)
	{
		return NULL;
	}

	switch (ffi_value->type)
	{
		case TYPE_BOOL:
			return value_create_bool((boolean)ffi_value->data.v_bool);

		case TYPE_CHAR:
			return value_create_char(ffi_value->data.v_char);

		case TYPE_SHORT:
			return value_create_short(ffi_value->data.v_short);

		case TYPE_INT:
			return value_create_int(ffi_value->data.v_int);

		case TYPE_LONG:
			return value_create_long(ffi_value->data.v_long);

		case TYPE_FLOAT:
			return value_create_float(ffi_value->data.v_float);

		case TYPE_DOUBLE:
			return value_create_double(ffi_value->data.v_double);

		case TYPE_STRING:
		{
			const char *str = (const char *)ffi_value->data.v_string.data;
			size_t str_size = ffi_value->data.v_string.size;

			if (str == NULL)
			{
				return value_create_string("", 0);
			}

			return value_create_string(str, str_size);
		}

		case TYPE_BUFFER:
			return value_create_buffer(ffi_value->data.v_buffer.data, ffi_value->data.v_buffer.size);

		case TYPE_PTR:
			return value_create_ptr(ffi_value->data.v_ptr);

		case TYPE_NULL:
			return value_create_null();

		default:
			break;
	}

	return NULL;
}

static int hs_loader_impl_discover_module_function(loader_impl impl, loader_impl_hs_module *module, size_t function_index, scope sp)
{
	const char *function_name_str = module->bridge.function_name(function_index);
	size_t args_count = module->bridge.function_args_count(function_index);
	loader_impl_hs_function hs_function = NULL;
	function f = NULL;
	value v = NULL;
	signature s;
	size_t arg_index;
	int ret_type_id;

	if (function_name_str == NULL || function_name_str[0] == '\0')
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Haskell discovery failed due to unnamed function at index %zu", function_index);
		return 1;
	}

	hs_function = (loader_impl_hs_function)malloc(sizeof(struct loader_impl_hs_function_type));

	if (hs_function == NULL)
	{
		return 1;
	}

	hs_function->module = module;
	hs_function->function_index = function_index;

	f = function_create(function_name_str, args_count, hs_function, &function_hs_singleton);

	if (f == NULL)
	{
		free(hs_function);
		return 1;
	}

	s = function_signature(f);

	ret_type_id = module->bridge.function_return_type(function_index);

	if (hs_loader_impl_valid_type_id(ret_type_id))
	{
		signature_set_return(s, loader_impl_type(impl, type_id_name(ret_type_id)));
	}

	for (arg_index = 0; arg_index < args_count; ++arg_index)
	{
		const char *arg_name = module->bridge.function_arg_name(function_index, arg_index);
		char generated_arg_name[0x20];
		int arg_type_id = module->bridge.function_arg_type(function_index, arg_index);
		type arg_type = hs_loader_impl_valid_type_id(arg_type_id) ? loader_impl_type(impl, type_id_name(arg_type_id)) : NULL;

		if (arg_name == NULL || arg_name[0] == '\0')
		{
			size_t arg_name_size = snprintf(generated_arg_name, sizeof(generated_arg_name), "arg_%zu", arg_index);

			if (arg_name_size == 0 || arg_name_size >= sizeof(generated_arg_name))
			{
				strcpy(generated_arg_name, "arg");
			}

			arg_name = generated_arg_name;
		}

		signature_set(s, arg_index, arg_name, arg_type);
	}

	v = value_create_function(f);

	if (v == NULL)
	{
		function_destroy(f);
		return 1;
	}

	if (scope_define(sp, function_name(f), v) != 0)
	{
		value_type_destroy(v);
		return 1;
	}

	return 0;
}

loader_impl_data hs_loader_impl_initialize(loader_impl impl, configuration config)
{
	loader_impl_hs hs_impl = (loader_impl_hs)malloc(sizeof(struct loader_impl_hs_type));

	(void)config;

	if (hs_impl == NULL)
	{
		return NULL;
	}

	if (hs_loader_impl_initialize_types(impl) != 0)
	{
		free(hs_impl);
		return NULL;
	}

	hs_impl->execution_paths = vector_create(sizeof(loader_path));

	if (hs_impl->execution_paths == NULL)
	{
		free(hs_impl);
		return NULL;
	}

	hs_loader_impl_initialize_load_from_memory_bridge(hs_impl);

	/* Register initialization */
	loader_initialization_register(impl);

	return (loader_impl_data)hs_impl;
}

int hs_loader_impl_execution_path(loader_impl impl, const loader_path path)
{
	loader_impl_hs hs_impl = (loader_impl_hs)loader_impl_get(impl);
	loader_path *execution_path;
	size_t path_size;

	if (hs_impl == NULL || hs_impl->execution_paths == NULL)
	{
		return 1;
	}

	vector_push_back_empty(hs_impl->execution_paths);
	execution_path = vector_back(hs_impl->execution_paths);

	path_size = strnlen(path, LOADER_PATH_SIZE - 1) + 1;
	memcpy(*execution_path, path, path_size);

	return 0;
}

loader_handle hs_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size)
{
	loader_impl_hs hs_impl = (loader_impl_hs)loader_impl_get(impl);
	loader_impl_hs_handle handle;
	size_t iterator;
	size_t loaded_modules;

	if (hs_impl == NULL)
	{
		return NULL;
	}

	handle = (loader_impl_hs_handle)malloc(sizeof(struct loader_impl_hs_handle_type));

	if (handle == NULL)
	{
		return NULL;
	}

	handle->modules = vector_create(sizeof(loader_impl_hs_module));

	if (handle->modules == NULL)
	{
		free(handle);
		return NULL;
	}

	loaded_modules = 0;

	for (iterator = 0; iterator < size; ++iterator)
	{
		loader_path absolute_path;
		loader_impl_hs_module *module;

		if (hs_loader_impl_resolve_path(hs_impl, paths[iterator], absolute_path) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Haskell module could not be resolved: %s", paths[iterator]);
			continue;
		}

		vector_push_back_empty(handle->modules);
		module = vector_back(handle->modules);

		if (hs_loader_impl_module_load(absolute_path, module) != 0)
		{
			vector_pop_back(handle->modules);
			continue;
		}

		++loaded_modules;

		log_write("metacall", LOG_LEVEL_DEBUG, "Haskell module %s loaded from file", absolute_path);
	}

	if (loaded_modules == 0)
	{
		vector_destroy(handle->modules);
		free(handle);
		return NULL;
	}

	return (loader_handle)handle;
}

loader_handle hs_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size)
{
	loader_impl_hs hs_impl = (loader_impl_hs)loader_impl_get(impl);
	loader_path output_path;

	if (hs_impl == NULL)
	{
		return NULL;
	}

	if (hs_impl->load_from_memory_bridge == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Haskell load from memory requires host bridge symbol: %s", hs_loader_load_from_memory_symbol);
		return NULL;
	}

	if (hs_impl->load_from_memory_bridge(name, buffer, size, output_path, LOADER_PATH_SIZE) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Haskell load from memory bridge failed for module: %s", name);
		return NULL;
	}

	return hs_loader_impl_load_from_file(impl, (const loader_path *)&output_path, 1);
}

loader_handle hs_loader_impl_load_from_package(loader_impl impl, const loader_path path)
{
	loader_path package_path;
	size_t path_size = strnlen(path, LOADER_PATH_SIZE - 1) + 1;

	memcpy(package_path, path, path_size);

	return hs_loader_impl_load_from_file(impl, (const loader_path *)&package_path, 1);
}

int hs_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_hs_handle hs_handle = (loader_impl_hs_handle)handle;

	(void)impl;

	if (hs_handle != NULL)
	{
		if (hs_handle->modules != NULL)
		{
			size_t iterator, size = vector_size(hs_handle->modules);

			for (iterator = 0; iterator < size; ++iterator)
			{
				loader_impl_hs_module *module = vector_at(hs_handle->modules, iterator);

				hs_loader_impl_module_unload(module);
			}

			vector_destroy(hs_handle->modules);
		}

		free(hs_handle);

		return 0;
	}

	return 1;
}

int hs_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_hs_handle hs_handle = (loader_impl_hs_handle)handle;
	scope sp = context_scope(ctx);
	size_t module_index;
	size_t module_count;

	if (hs_handle == NULL || hs_handle->modules == NULL)
	{
		return 1;
	}

	module_count = vector_size(hs_handle->modules);

	for (module_index = 0; module_index < module_count; ++module_index)
	{
		loader_impl_hs_module *module = vector_at(hs_handle->modules, module_index);
		size_t function_index;
		size_t function_count = module->bridge.function_count();

		for (function_index = 0; function_index < function_count; ++function_index)
		{
			if (hs_loader_impl_discover_module_function(impl, module, function_index, sp) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Haskell discovery failed for module %s at function index %zu", module->path, function_index);
				return 1;
			}
		}
	}

	return 0;
}

int hs_loader_impl_destroy(loader_impl impl)
{
	loader_impl_hs hs_impl = (loader_impl_hs)loader_impl_get(impl);

	if (hs_impl != NULL)
	{
		/* Destroy children loaders */
		loader_unload_children(impl);

		if (hs_impl->execution_paths != NULL)
		{
			vector_destroy(hs_impl->execution_paths);
		}

		free(hs_impl);

		return 0;
	}

	return 1;
}
