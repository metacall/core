/*
 *	Loader Library by Parra Studios
 *	A plugin for loading file code at run-time into a process.
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

#include <file_loader/file_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <portability/portability_path.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <adt/adt_vector.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32)
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif

	#include <windows.h>
	#include <sys/stat.h> /* Apparently it is supported on newer windows standards */

typedef struct _stat file_stat_type;

	#define file_stat _stat

#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__) || \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

	#include <sys/stat.h>

typedef struct stat file_stat_type;

	#define file_stat stat

#endif

/* Support for glob, only in POSIX for now (TODO: Implement Windows support) */
#if !defined(_WIN32) && \
	(defined(unix) || defined(__unix__) || defined(__unix) || \
		defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
		defined(__CYGWIN__) || defined(__CYGWIN32__) || \
		(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__))

	#include <unistd.h>

	#if defined(_POSIX_VERSION)
		#define FILE_LOADER_GLOB_SUPPORT 1
		#include <glob.h>
	#endif
#endif

typedef struct loader_impl_file_descriptor_type
{
	loader_path path;
	size_t length;

} * loader_impl_file_descriptor;

typedef struct loader_impl_file_type
{
	vector execution_paths;

} * loader_impl_file;

typedef struct loader_impl_file_handle_type
{
	vector paths;

} * loader_impl_file_handle;

typedef struct loader_impl_file_function_type
{
	loader_impl_file_descriptor descriptor;

} * loader_impl_file_function;

static int file_loader_impl_load_path(loader_impl_file_handle handle, const loader_path path, size_t path_size);
#if defined(FILE_LOADER_GLOB_SUPPORT)
static int file_loader_impl_load_glob(loader_impl_file_handle handle, const loader_path path, size_t path_size);
#endif
static int file_loader_impl_load_execution_path(loader_impl_file file_impl, loader_impl_file_handle handle, const loader_path path);

int function_file_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

function_return function_file_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	loader_impl_file_function file_function = (loader_impl_file_function)impl;

	(void)func;
	(void)args;
	(void)size;

	return value_create_string(file_function->descriptor->path, file_function->descriptor->length);
}

function_return function_file_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;
	(void)resolve_callback;
	(void)reject_callback;
	(void)context;

	return NULL;
}

void function_file_interface_destroy(function func, function_impl impl)
{
	loader_impl_file_function file_function = (loader_impl_file_function)impl;

	(void)func;

	if (file_function != NULL)
	{
		free(file_function);
	}
}

function_interface function_file_singleton(void)
{
	static struct function_interface_type file_interface = {
		&function_file_interface_create,
		&function_file_interface_invoke,
		&function_file_interface_await,
		&function_file_interface_destroy
	};

	return &file_interface;
}

int file_loader_impl_initialize_types(loader_impl impl)
{
	static struct
	{
		type_id id;
		const char *name;
	} type_id_name_pair[] = {
		{ TYPE_STRING, "File" },
	};

	size_t index, size = sizeof(type_id_name_pair) / sizeof(type_id_name_pair[0]);

	for (index = 0; index < size; ++index)
	{
		type t = type_create(type_id_name_pair[index].id, type_id_name_pair[index].name, NULL, NULL);

		if (t != NULL)
		{
			if (loader_impl_type_define(impl, type_name(t), t) != 0)
			{
				type_destroy(t);
				return 1;
			}
		}
	}

	return 0;
}

loader_impl_data file_loader_impl_initialize(loader_impl impl, configuration config)
{
	loader_impl_file file_impl;

	(void)impl;
	(void)config;

	file_impl = malloc(sizeof(struct loader_impl_file_type));

	if (file_impl == NULL)
	{
		return NULL;
	}

	if (file_loader_impl_initialize_types(impl) != 0)
	{
		free(file_impl);

		return NULL;
	}

	file_impl->execution_paths = vector_create(sizeof(loader_path));

	if (file_impl->execution_paths == NULL)
	{
		free(file_impl);

		return NULL;
	}

	/* Register initialization */
	loader_initialization_register(impl);

	return (loader_impl_data)file_impl;
}

int file_loader_impl_execution_path(loader_impl impl, const loader_path path)
{
	loader_impl_file file_impl = loader_impl_get(impl);
	loader_path *execution_path;

	vector_push_back_empty(file_impl->execution_paths);

	execution_path = vector_back(file_impl->execution_paths);

	memcpy(*execution_path, path, strnlen(path, LOADER_PATH_SIZE) + 1);

	return 0;
}

int file_loader_impl_load_path(loader_impl_file_handle handle, const loader_path path, size_t path_size)
{
	file_stat_type fs;

	if (file_stat(path, &fs) == 0)
	{
		loader_impl_file_descriptor descriptor = NULL;

		vector_insert_empty(handle->paths, vector_size(handle->paths));

		descriptor = vector_back(handle->paths);

		descriptor->length = path_size - 1;

		memcpy(descriptor->path, path, path_size);

		return 0;
	}

	return 1;
}

#if defined(FILE_LOADER_GLOB_SUPPORT)
int file_loader_impl_load_glob(loader_impl_file_handle handle, const loader_path path, size_t path_size)
{
	glob_t glob_result;

	(void)path_size;

	memset(&glob_result, 0, sizeof(glob_result));

	if (glob(path, GLOB_TILDE, NULL, &glob_result) != 0)
	{
		globfree(&glob_result);
		return 1;
	}

	size_t i, loaded_files = 0;

	for (i = 0; i < glob_result.gl_pathc; ++i)
	{
		loader_path glob_path;
		size_t length = strnlen(glob_result.gl_pathv[i], LOADER_PATH_SIZE);

		strncpy(glob_path, glob_result.gl_pathv[i], length);

		glob_path[length] = '\0';

		if (file_loader_impl_load_path(handle, glob_path, length + 1) == 0)
		{
			++loaded_files;
		}
	}

	globfree(&glob_result);

	return (loaded_files == 0);
}
#endif

int file_loader_impl_load_execution_path(loader_impl_file file_impl, loader_impl_file_handle handle, const loader_path path)
{
	size_t path_size = strnlen(path, LOADER_PATH_SIZE) + 1;
	int (*impl_load_path)(loader_impl_file_handle, const loader_path, size_t);

#if defined(FILE_LOADER_GLOB_SUPPORT)
	if (portability_path_is_pattern(path, path_size) == 0)
	{
		impl_load_path = &file_loader_impl_load_glob;
	}
	else
#endif
	{
		impl_load_path = &file_loader_impl_load_path;
	}

	if (portability_path_is_absolute(path, path_size) == 0)
	{
		return impl_load_path(handle, path, path_size);
	}
	else
	{
		size_t iterator, size = vector_size(file_impl->execution_paths);

		for (iterator = 0; iterator < size; ++iterator)
		{
			loader_path *execution_path = vector_at(file_impl->execution_paths, iterator);
			loader_path absolute_path;
			size_t absolute_path_size = portability_path_join(*execution_path, strnlen(*execution_path, LOADER_PATH_SIZE) + 1, path, path_size, absolute_path, LOADER_PATH_SIZE);

			if (impl_load_path(handle, absolute_path, absolute_path_size) == 0)
			{
				return 0;
			}
		}
	}

	return 1;
}

loader_handle file_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size)
{
	loader_impl_file file_impl = loader_impl_get(impl);

	if (file_impl == NULL)
	{
		return NULL;
	}

	loader_impl_file_handle handle = malloc(sizeof(struct loader_impl_file_handle_type));

	if (handle != NULL)
	{
		size_t iterator;

		handle->paths = vector_create(sizeof(struct loader_impl_file_descriptor_type));

		if (handle->paths == NULL)
		{
			free(handle);

			return NULL;
		}

		for (iterator = 0; iterator < size; ++iterator)
		{
			if (file_loader_impl_load_execution_path(file_impl, handle, paths[iterator]) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "File %s not found", paths[iterator]);
			}
		}

		if (vector_size(handle->paths) == 0)
		{
			vector_destroy(handle->paths);

			free(handle);

			return NULL;
		}

		return (loader_handle)handle;
	}

	return NULL;
}

loader_handle file_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size)
{
	(void)impl;
	(void)name;
	(void)buffer;
	(void)size;

	// TODO: In theory this should load the buffer, not the name... no?
	// Although this loader does not take care of the file itself, only the paths;
	// so probably this make or neither makes sense to be used or it should handle more than the paths

	/*
	loader_impl_file_handle handle = malloc(sizeof(struct loader_impl_file_handle_type));

	if (handle != NULL)
	{
		loader_impl_file_descriptor descriptor = NULL;

		handle->paths = vector_create(sizeof(struct loader_impl_file_descriptor_type));

		if (handle->paths == NULL)
		{
			free(handle);

			return NULL;
		}

		vector_insert_empty(handle->paths, 0);

		descriptor = vector_back(handle->paths);

		descriptor->length = strnlen(name, LOADER_NAME_SIZE);

		memcpy(descriptor->path, name, descriptor->length + 1);

		log_write("metacall", LOG_LEVEL_DEBUG, "File module %s loaded from memory", name);

		return (loader_handle)handle;
	}
	*/

	return NULL;
}

loader_handle file_loader_impl_load_from_package(loader_impl impl, const loader_path path)
{
	loader_impl_file file_impl = loader_impl_get(impl);

	if (file_impl == NULL)
	{
		return NULL;
	}

	loader_impl_file_handle handle = malloc(sizeof(struct loader_impl_file_handle_type));

	if (handle != NULL)
	{
		handle->paths = vector_create(sizeof(struct loader_impl_file_descriptor_type));

		if (handle->paths == NULL)
		{
			free(handle);

			return NULL;
		}

		if (file_loader_impl_load_execution_path(file_impl, handle, path) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "File %s not found", path);
		}

		if (vector_size(handle->paths) == 0)
		{
			vector_destroy(handle->paths);

			free(handle);

			return NULL;
		}

		return (loader_handle)handle;
	}

	return NULL;
}

int file_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_file_handle file_handle = (loader_impl_file_handle)handle;

	(void)impl;

	if (file_handle != NULL)
	{
		vector_destroy(file_handle->paths);

		free(file_handle);

		return 0;
	}

	return 1;
}

int file_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_file file_impl = loader_impl_get(impl);

	loader_impl_file_handle file_handle = (loader_impl_file_handle)handle;

	scope sp = context_scope(ctx);

	size_t iterator, size = vector_size(file_handle->paths);

	(void)file_impl;

	log_write("metacall", LOG_LEVEL_DEBUG, "File module %p discovering", handle);

	for (iterator = 0; iterator < size; ++iterator)
	{
		loader_impl_file_descriptor descriptor = vector_at(file_handle->paths, iterator);

		loader_impl_file_function file_function = malloc(sizeof(struct loader_impl_file_function_type));

		if (file_function != NULL)
		{
			/* TODO: Refactor script path, it supports multiple paths right now, it should not be accessed like that */
			const char *script_path = getenv("LOADER_SCRIPT_PATH");
			function f;
			signature s;
			value v;

			file_function->descriptor = descriptor;

			if (script_path != NULL)
			{
				loader_path path;

				(void)portability_path_get_relative(script_path, strlen(script_path) + 1, descriptor->path, descriptor->length + 1, path, LOADER_PATH_SIZE);

				log_write("metacall", LOG_LEVEL_DEBUG, "File registering function: %s", path);

				f = function_create(path, 0, file_function, &function_file_singleton);
			}
			else
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "File registering function: %s", descriptor->path);

				f = function_create(descriptor->path, 0, file_function, &function_file_singleton);
			}

			s = function_signature(f);

			signature_set_return(s, loader_impl_type(impl, "Path"));

			v = value_create_function(f);

			if (scope_define(sp, function_name(f), v) != 0)
			{
				value_type_destroy(v);
				return 1;
			}
		}
	}

	return 0;
}

int file_loader_impl_destroy(loader_impl impl)
{
	loader_impl_file file_impl = loader_impl_get(impl);

	if (file_impl != NULL)
	{
		/* Destroy children loaders */
		loader_unload_children(impl);

		if (file_impl->execution_paths != NULL)
		{
			vector_destroy(file_impl->execution_paths);
		}

		free(file_impl);

		return 0;
	}

	return 1;
}
