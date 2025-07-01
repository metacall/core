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

#ifdef WASMTIME
	#if defined(_WIN32) && defined(_MSC_VER)
		#define WASM_API_EXTERN
	#endif
	#include <wasmtime.h>
#endif

#include <wasm_loader/wasm_loader_function.h>
#include <wasm_loader/wasm_loader_handle.h>
#include <wasm_loader/wasm_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <portability/portability_path.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <log/log.h>

#include <stdlib.h>

#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))

typedef struct loader_impl_wasm_type
{
	wasm_engine_t *engine;
	wasm_store_t *store;
	vector paths;
} * loader_impl_wasm;

static int initialize_types(loader_impl impl);

static FILE *open_file_absolute(const loader_path path, size_t *file_size);
static FILE *open_file_relative(loader_impl_wasm impl, const loader_path path, size_t *file_size);
static char *read_buffer_from_file(loader_impl impl, const loader_path path, size_t *file_size);

static int try_wat2wasm(const char *buffer, size_t size, wasm_byte_vec_t *binary);
static int load_module_from_package(loader_impl impl, loader_impl_wasm_handle handle, const loader_path path);
static int load_module_from_file(loader_impl impl, loader_impl_wasm_handle handle, const loader_path path);

loader_impl_data wasm_loader_impl_initialize(loader_impl impl, configuration config)
{
	(void)config;

	loader_impl_wasm wasm_impl = malloc(sizeof(struct loader_impl_wasm_type));

	if (wasm_impl == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to allocate memory for loader implementation");
		goto error_impl_alloc;
	}

	if (initialize_types(impl) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to initialize types");
		goto error_types_init;
	}

	// TODO: Use wasm_engine_new_with_config function for passing in config?
	wasm_impl->engine = wasm_engine_new();

	if (wasm_impl->engine == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to create engine");
		goto error_engine_creation;
	}

	wasm_impl->store = wasm_store_new(wasm_impl->engine);

	if (wasm_impl->store == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to create store");
		goto error_store_creation;
	}

	wasm_impl->paths = vector_create_type(loader_path);

	if (wasm_impl->paths == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to create paths vector");
		goto error_paths_creation;
	}

	loader_initialization_register(impl);
	log_write("metacall", LOG_LEVEL_DEBUG, "WebAssembly loader initialized correctly");

	return wasm_impl;

error_paths_creation:
	wasm_store_delete(wasm_impl->store);
error_store_creation:
	wasm_engine_delete(wasm_impl->engine);
error_engine_creation:
error_types_init:
	free(wasm_impl);
error_impl_alloc:
	return NULL;
}

int wasm_loader_impl_execution_path(loader_impl impl, const loader_path path)
{
	loader_impl_wasm wasm_impl = loader_impl_get(impl);
	vector_push_back_empty(wasm_impl->paths);
	loader_path *back_ptr = vector_back(wasm_impl->paths);
	strncpy(*back_ptr, path, sizeof(loader_path));

	return 0;
}

loader_handle wasm_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size)
{
	loader_impl_wasm_handle handle = wasm_loader_handle_create(size);

	if (handle == NULL)
	{
		goto error_alloc_handle;
	}

	for (size_t idx = 0; idx < size; idx++)
	{
		if (load_module_from_file(impl, handle, paths[idx]) != 0)
		{
			goto error_load_module;
		}
	}

	return handle;

error_load_module:
	wasm_loader_impl_clear(impl, handle);
error_alloc_handle:
	return NULL;
}

loader_handle wasm_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size)
{
	loader_impl_wasm wasm_impl = loader_impl_get(impl);
	loader_impl_wasm_handle handle = wasm_loader_handle_create(1);

	if (handle == NULL)
	{
		goto error_alloc_handle;
	}

	wasm_byte_vec_t binary;
	// There is sadly no way to check whether `wasm_byte_vec_new`
	// fails, so we just have to hope for the best here.
	wasm_byte_vec_new(&binary, size, buffer);
	if (!wasm_module_validate(wasm_impl->store, &binary))
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "WebAssembly loader: Buffer is not valid binary module, trying wat2wasm");

		wasm_byte_vec_delete(&binary);
		if (try_wat2wasm(buffer, size, &binary) != 0)
		{
			goto error_convert_buffer;
		}
	}

	if (wasm_loader_handle_add_module(handle, name, wasm_impl->store, &binary) != 0)
	{
		goto error_load_module;
	}

	wasm_byte_vec_delete(&binary);

	return handle;

error_load_module:
	wasm_byte_vec_delete(&binary);
error_convert_buffer:
	wasm_loader_impl_clear(impl, handle);
error_alloc_handle:
	return NULL;
}

loader_handle wasm_loader_impl_load_from_package(loader_impl impl, const loader_path path)
{
	loader_impl_wasm_handle handle = wasm_loader_handle_create(1);

	if (handle == NULL)
	{
		goto error_alloc_handle;
	}

	if (load_module_from_package(impl, handle, path) != 0)
	{
		goto error_load_module;
	}

	return handle;

error_load_module:
	wasm_loader_impl_clear(impl, handle);
error_alloc_handle:
	return NULL;
}

int wasm_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	(void)impl;

	wasm_loader_handle_destroy(handle);

	return 0;
}

int wasm_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	return wasm_loader_handle_discover(impl, handle, context_scope(ctx));
}

int wasm_loader_impl_destroy(loader_impl impl)
{
	loader_impl_wasm wasm_impl = loader_impl_get(impl);
	loader_unload_children(impl);
	vector_destroy(wasm_impl->paths);
	wasm_store_delete(wasm_impl->store);
	wasm_engine_delete(wasm_impl->engine);
	free(wasm_impl);

	log_write("metacall", LOG_LEVEL_DEBUG, "WebAssembly loader destroyed");

	return 0;
}

static int initialize_types(loader_impl impl)
{
	// TODO: Implement ANYREF and FUNCREF?
	static struct
	{
		type_id id;
		const char *name;
	} type_names[] = {
		{ TYPE_INT, "i32" },
		{ TYPE_LONG, "i64" },
		{ TYPE_FLOAT, "f32" },
		{ TYPE_DOUBLE, "f64" },
		{ TYPE_ARRAY, "array" },
	};

	const size_t size = COUNT_OF(type_names);

	for (size_t i = 0; i < size; i++)
	{
		type t = type_create(type_names[i].id, type_names[i].name, NULL, NULL);

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

static FILE *open_file_absolute(const loader_path path, size_t *file_size)
{
	FILE *file = fopen(path, "rb");
	if (file == NULL)
	{
		goto error_open_file;
	}

	if (fseek(file, 0, SEEK_END) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to seek to end of file");
		goto error_seek_file;
	}

	*file_size = ftell(file);

	if (fseek(file, 0, SEEK_SET) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to seek to start of file");
		goto error_seek_file;
	}

	return file;

error_seek_file:
	fclose(file);
error_open_file:
	return NULL;
}

static FILE *open_file_relative(loader_impl_wasm impl, const loader_path path, size_t *file_size)
{
	for (size_t i = 0; i < vector_size(impl->paths); i++)
	{
		loader_path *exec_path = vector_at(impl->paths, i);

		loader_path abs_path;
		(void)portability_path_join(*exec_path, strlen(*exec_path) + 1, path, strnlen(path, LOADER_PATH_SIZE) + 1, abs_path, LOADER_PATH_SIZE);

		FILE *file = open_file_absolute(abs_path, file_size);

		if (file == NULL)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "WebAssembly loader: Could not open file %s", abs_path);
			continue;
		}

		log_write("metacall", LOG_LEVEL_DEBUG, "WebAssembly loader: Opened file %s", abs_path);
		return file;
	}

	return NULL;
}

static char *read_buffer_from_file(loader_impl impl, const loader_path path, size_t *file_size)
{
	loader_impl_wasm wasm_impl = loader_impl_get(impl);

	FILE *file = open_file_relative(wasm_impl, path, file_size);

	if (file == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to open file %s", path);
		goto error_open_file;
	}

	char *buffer = malloc(*file_size);
	if (buffer == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to allocate file buffer");
		goto error_buffer_alloc;
	}

	size_t bytes_read = fread(buffer, 1, *file_size, file);
	if (bytes_read != *file_size)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Failed to read file (read %zu of %zu bytes)", bytes_read, *file_size);
		goto error_read_file;
	}

	fclose(file);
	return buffer;

error_read_file:
	free(buffer);
error_buffer_alloc:
	fclose(file);
error_open_file:
	return NULL;
}

#ifdef WASMTIME
static void wasm_loader_impl_handle_wasmtime_error(wasmtime_error_t *error)
{
	wasm_name_t message;
	wasmtime_error_message(error, &message);

	log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Encountered Wasmtime error\n%.*s", message.size, message.data);

	wasmtime_error_delete(error);
	wasm_byte_vec_delete(&message);
}
#endif

static int try_wat2wasm(const char *buffer, size_t size, wasm_byte_vec_t *binary)
{
#ifdef WASMTIME
	wasmtime_error_t *error = wasmtime_wat2wasm(buffer, size, binary);

	if (error != NULL)
	{
		wasm_loader_impl_handle_wasmtime_error(error);
		return 1;
	}

	return 0;
#else
	(void)buffer;
	(void)size;
	(void)binary;

	log_write("metacall", LOG_LEVEL_ERROR, "WebAssembly loader: Current WebAssembly runtime does not support conversion from text format to binary format");
	return 1;
#endif
}

static int load_module_from_package(loader_impl impl, loader_impl_wasm_handle handle, const loader_path path)
{
	int ret = 1;

	size_t size;
	char *buffer = read_buffer_from_file(impl, path, &size);

	if (buffer == NULL)
	{
		goto error_read_file;
	}

	wasm_byte_vec_t binary;
	wasm_byte_vec_new(&binary, size, buffer);

	loader_name module_name;
	portability_path_get_name(path, strnlen(path, LOADER_PATH_SIZE) + 1, module_name, LOADER_NAME_SIZE);

	loader_impl_wasm wasm_impl = loader_impl_get(impl);
	if (wasm_loader_handle_add_module(handle, module_name, wasm_impl->store, &binary) != 0)
	{
		goto error_add_module;
	}

	ret = 0;

error_add_module:
	wasm_byte_vec_delete(&binary);
error_read_file:
	free(buffer);
	return ret;
}

static int load_module_from_file(loader_impl impl, loader_impl_wasm_handle handle, const loader_path path)
{
	static const loader_tag TEXT_EXTENSION = "wat";

	int ret = 1;

	size_t size;
	char *buffer = read_buffer_from_file(impl, path, &size);

	if (buffer == NULL)
	{
		goto error_read_file;
	}

	wasm_byte_vec_t binary;
	if (try_wat2wasm(buffer, size, &binary) != 0)
	{
		goto error_convert_buffer;
	}

	loader_name module_name;
	(void)portability_path_get_module_name(path, strnlen(path, LOADER_PATH_SIZE) + 1, TEXT_EXTENSION, sizeof(TEXT_EXTENSION), module_name, LOADER_NAME_SIZE);

	loader_impl_wasm wasm_impl = loader_impl_get(impl);
	if (wasm_loader_handle_add_module(handle, module_name, wasm_impl->store, &binary) != 0)
	{
		goto error_add_module;
	}

	ret = 0;

error_add_module:
	wasm_byte_vec_delete(&binary);
error_convert_buffer:
error_read_file:
	free(buffer);
	return ret;
}
