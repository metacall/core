/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

/* -- Headers -- */

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

#include <loader/loader.h>

#include <reflect/reflect.h>

#include <configuration/configuration.h>

#include <log/log.h>

#include <serial/serial.h>

#include <backtrace/backtrace.h>

#include <environment/environment_variable.h>

#include <stdio.h>
#include <string.h>

/* -- Definitions -- */

#define METACALL_ARGS_SIZE 0x10
#define METACALL_SERIAL	   "rapid_json"

/* -- Type Definitions -- */

typedef value (*method_invoke_ptr)(void *, method, void *[], size_t);

/* -- Global Variables -- */

void *metacall_null_args[1] = { NULL };

/* -- Private Variables -- */

static int metacall_initialize_flag = 1;
static int metacall_log_null_flag = 1;
static int metacall_config_flags = 0;
static int metacall_initialize_argc = 0;
static char **metacall_initialize_argv = NULL;

/* -- Private Methods -- */

static void *metacallv_method(void *target, const char *name, method_invoke_ptr call, vector v, void *args[], size_t size);
static type_id *metacall_type_ids(void *args[], size_t size);

/* -- Methods -- */

const char *metacall_serial(void)
{
	static const char metacall_serial_str[] = METACALL_SERIAL;

	return metacall_serial_str;
}

void metacall_log_null(void)
{
	metacall_log_null_flag = 0;
}

void metacall_flags(int flags)
{
	metacall_config_flags = flags;
}

int metacall_initialize(void)
{
	memory_allocator allocator;

	/* Initialize logs by default to stdout if none has been defined */
	if (metacall_log_null_flag != 0 && log_size() == 0)
	{
		struct metacall_log_stdio_type log_stdio;

		log_stdio.stream = stdout;

		if (metacall_log(METACALL_LOG_STDIO, (void *)&log_stdio) != 0)
		{
			return 1;
		}

		log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall default logger to stdout initialized");
	}

	if (metacall_initialize_flag == 0)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall already initialized");

		return 0;
	}

	log_write("metacall", LOG_LEVEL_DEBUG, "Initializing MetaCall");

	/* Initialize backtrace for catching segmentation faults */
	if (backtrace_initialize() != 0)
	{
		log_write("metacall", LOG_LEVEL_WARNING, "MetaCall backtrace could not be initialized");
	}

	/* Initialize MetaCall version environment variable */
	if (environment_variable_set_expand(METACALL_VERSION) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "MetaCall environment variables could not be initialized");
		return 1;
	}

#ifdef METACALL_FORK_SAFE
	if (metacall_config_flags & METACALL_FLAGS_FORK_SAFE)
	{
		if (metacall_fork_initialize() != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid MetaCall fork initialization");
		}

		log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall fork initialized");
	}
#endif /* METACALL_FORK_SAFE */

	allocator = memory_allocator_std(&malloc, &realloc, &free);

	if (configuration_initialize(metacall_serial(), NULL, allocator) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid MetaCall configuration initialization");

		memory_allocator_destroy(allocator);

		return 1;
	}

	memory_allocator_destroy(allocator);

	/* TODO: Improve log initialization and configuration */
	{
		configuration config = configuration_scope(CONFIGURATION_GLOBAL_SCOPE);

		if (config != NULL)
		{
			value *level = configuration_value(config, "log_level");

			if (level != NULL)
			{
				const char *level_str = (const char *)value_to_string(level);

				if (log_level("metacall", level_str, value_type_size(level) - 1) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Invalid MetaCall configuration log_level, %s is not valid", level_str);
				}
				else
				{
					log_write("metacall", LOG_LEVEL_INFO, "Set MetaCall log level to %s", level_str);
				}
			}
		}
	}

	if (loader_initialize() != 0)
	{
		configuration_destroy();

		/* Unregister backtrace */
		if (backtrace_destroy() != 0)
		{
			log_write("metacall", LOG_LEVEL_WARNING, "MetaCall backtrace could not be destroyed");
		}

		return 1;
	}

	metacall_initialize_flag = 0;

	return 0;
}

int metacall_initialize_ex(struct metacall_initialize_configuration_type initialize_config[])
{
	size_t index = 0;

	if (metacall_initialize() == 1)
	{
		return 1;
	}

	while (!(initialize_config[index].tag == NULL && initialize_config[index].options == NULL))
	{
		loader_impl impl = loader_get_impl(initialize_config[index].tag);

		if (impl == NULL)
		{
			return 1;
		}

		loader_set_options(initialize_config[index].tag, initialize_config[index].options);

		++index;
	}

	return 0;
}

void metacall_initialize_args(int argc, char *argv[])
{
	metacall_initialize_argc = argc;
	metacall_initialize_argv = argv;
}

char **metacall_argv(void)
{
	return metacall_initialize_argv;
}

int metacall_argc(void)
{
	return metacall_initialize_argc;
}

int metacall_is_initialized(const char *tag)
{
	return loader_is_initialized(tag);
}

size_t metacall_args_size(void)
{
	const size_t args_size = METACALL_ARGS_SIZE;

	return args_size;
}

int metacall_execution_path(const char *tag, const char *path)
{
	loader_path path_impl;

	if (tag == NULL || path == NULL)
	{
		return 1;
	}

	strncpy(path_impl, path, LOADER_PATH_SIZE - 1);

	return loader_execution_path(tag, path_impl);
}

int metacall_execution_path_s(const char *tag, size_t tag_length, const char *path, size_t path_length)
{
	loader_path path_impl;
	loader_tag tag_impl;

	if (tag == NULL || tag_length == 0 || tag_length >= LOADER_TAG_SIZE || path == NULL || path_length == 0 || path_length >= LOADER_PATH_SIZE)
	{
		return 1;
	}

	strncpy(path_impl, path, path_length);
	strncpy(tag_impl, tag, tag_length);

	path_impl[path_length] = '\0';
	tag_impl[tag_length] = '\0';

	return loader_execution_path(tag_impl, path_impl);
}

int metacall_load_from_file(const char *tag, const char *paths[], size_t size, void **handle)
{
	loader_path *path_impl;
	size_t iterator;

	if (size == 0)
	{
		return 1;
	}

	path_impl = (loader_path *)malloc(sizeof(loader_path) * size);

	if (path_impl == NULL)
	{
		return 1;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		strncpy(path_impl[iterator], paths[iterator], LOADER_PATH_SIZE);
	}

	int result = loader_load_from_file(tag, (const loader_path *)path_impl, size, handle);

	free(path_impl);

	return result;
}

int metacall_load_from_memory(const char *tag, const char *buffer, size_t size, void **handle)
{
	return loader_load_from_memory(tag, buffer, size, handle);
}

int metacall_load_from_package(const char *tag, const char *path, void **handle)
{
	return loader_load_from_package(tag, path, handle);
}

int metacall_load_from_configuration(const char *path, void **handle, void *allocator)
{
	return loader_load_from_configuration(path, handle, allocator);
}

void *metacallv(const char *name, void *args[])
{
	value f_val = loader_get(name);
	function f = NULL;

	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	return metacallfv(f, args);
}

void *metacallv_s(const char *name, void *args[], size_t size)
{
	value f_val = loader_get(name);
	function f = NULL;

	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	return metacallfv_s(f, args, size);
}

void *metacallhv(void *handle, const char *name, void *args[])
{
	if (loader_impl_handle_validate(handle) != 0)
	{
		// TODO: Implement type error return a value
		log_write("metacall", LOG_LEVEL_ERROR, "Handle %p passed to metacallhv is not valid", handle);
		return NULL;
	}

	value f_val = loader_handle_get(handle, name);
	function f = NULL;

	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	return metacallfv(f, args);
}

void *metacallhv_s(void *handle, const char *name, void *args[], size_t size)
{
	if (loader_impl_handle_validate(handle) != 0)
	{
		// TODO: Implement type error return a value
		log_write("metacall", LOG_LEVEL_ERROR, "Handle %p passed to metacallhv_s is not valid", handle);
		return NULL;
	}

	value f_val = loader_handle_get(handle, name);
	function f = NULL;

	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	return metacallfv_s(f, args, size);
}

void *metacall(const char *name, ...)
{
	value f_val = loader_get(name);
	function f = NULL;

	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	if (f != NULL)
	{
		void *args[METACALL_ARGS_SIZE];

		value ret = NULL;

		signature s = function_signature(f);

		size_t iterator, args_count = signature_count(s);

		va_list va;

		va_start(va, name);

		for (iterator = 0; iterator < args_count; ++iterator)
		{
			type t = signature_get_type(s, iterator);

			type_id id = type_index(t);

			if (id == TYPE_BOOL)
			{
				args[iterator] = value_create_bool((boolean)va_arg(va, unsigned int));
			}
			else if (id == TYPE_CHAR)
			{
				args[iterator] = value_create_char((char)va_arg(va, int));
			}
			else if (id == TYPE_SHORT)
			{
				args[iterator] = value_create_short((short)va_arg(va, int));
			}
			else if (id == TYPE_INT)
			{
				args[iterator] = value_create_int(va_arg(va, int));
			}
			else if (id == TYPE_LONG)
			{
				args[iterator] = value_create_long(va_arg(va, long));
			}
			else if (id == TYPE_FLOAT)
			{
				args[iterator] = value_create_float((float)va_arg(va, double));
			}
			else if (id == TYPE_DOUBLE)
			{
				args[iterator] = value_create_double(va_arg(va, double));
			}
			else if (id == TYPE_STRING)
			{
				const char *str = va_arg(va, const char *);

				args[iterator] = value_create_string(str, strlen(str));
			}
			else if (id == TYPE_PTR)
			{
				args[iterator] = value_create_ptr(va_arg(va, const void *));
			}
			else
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Calling metacall with unsupported type '%s', using null type instead", type_id_name(id));
				args[iterator] = metacall_value_create_null();
			}
		}

		va_end(va);

		ret = function_call(f, args, args_count);

		for (iterator = 0; iterator < args_count; ++iterator)
		{
			value_type_destroy(args[iterator]);
		}

		if (ret != NULL)
		{
			type t = signature_get_return(s);

			if (t != NULL)
			{
				type_id id = type_index(t);

				if (id != value_type_id(ret))
				{
					value cast_ret = value_type_cast(ret, id);

					return (cast_ret == NULL) ? ret : cast_ret;
				}
			}
		}

		return ret;
	}

	return NULL;
}

void *metacallt(const char *name, const enum metacall_value_id ids[], ...)
{
	value f_val = loader_get(name);
	function f = NULL;

	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	if (f != NULL)
	{
		void *args[METACALL_ARGS_SIZE];

		value ret = NULL;

		signature s = function_signature(f);

		size_t iterator, args_count = signature_count(s);

		va_list va;

		va_start(va, ids);

		for (iterator = 0; iterator < args_count; ++iterator)
		{
			type t = signature_get_type(s, iterator);

			type_id id = type_index(t);

			if (t != NULL)
			{
				id = type_index(t);
			}
			else
			{
				id = ids[iterator];
			}

			if (id == TYPE_BOOL)
			{
				args[iterator] = value_create_bool((boolean)va_arg(va, unsigned int));
			}
			if (id == TYPE_CHAR)
			{
				args[iterator] = value_create_char((char)va_arg(va, int));
			}
			else if (id == TYPE_SHORT)
			{
				args[iterator] = value_create_short((short)va_arg(va, int));
			}
			else if (id == TYPE_INT)
			{
				args[iterator] = value_create_int(va_arg(va, int));
			}
			else if (id == TYPE_LONG)
			{
				args[iterator] = value_create_long(va_arg(va, long));
			}
			else if (id == TYPE_FLOAT)
			{
				args[iterator] = value_create_float((float)va_arg(va, double));
			}
			else if (id == TYPE_DOUBLE)
			{
				args[iterator] = value_create_double(va_arg(va, double));
			}
			else if (id == TYPE_STRING)
			{
				const char *str = va_arg(va, const char *);

				args[iterator] = value_create_string(str, strlen(str));
			}
			else if (id == TYPE_PTR)
			{
				args[iterator] = value_create_ptr(va_arg(va, const void *));
			}
			else
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Calling metacallt with unsupported type '%s', using null type instead", type_id_name(id));
				args[iterator] = metacall_value_create_null();
			}
		}

		va_end(va);

		ret = function_call(f, args, args_count);

		for (iterator = 0; iterator < args_count; ++iterator)
		{
			value_type_destroy(args[iterator]);
		}

		return ret;
	}

	return NULL;
}

void *metacallt_s(const char *name, const enum metacall_value_id ids[], size_t size, ...)
{
	value f_val = loader_get(name);
	function f = NULL;

	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	if (f != NULL)
	{
		void *args[METACALL_ARGS_SIZE];

		value ret = NULL;

		signature s = function_signature(f);

		size_t iterator;

		va_list va;

		va_start(va, size);

		for (iterator = 0; iterator < size; ++iterator)
		{
			type t = signature_get_type(s, iterator);

			type_id id = type_index(t);

			if (t != NULL)
			{
				id = type_index(t);
			}
			else
			{
				id = ids[iterator];
			}

			if (id == TYPE_BOOL)
			{
				args[iterator] = value_create_bool((boolean)va_arg(va, unsigned int));
			}
			if (id == TYPE_CHAR)
			{
				args[iterator] = value_create_char((char)va_arg(va, int));
			}
			else if (id == TYPE_SHORT)
			{
				args[iterator] = value_create_short((short)va_arg(va, int));
			}
			else if (id == TYPE_INT)
			{
				args[iterator] = value_create_int(va_arg(va, int));
			}
			else if (id == TYPE_LONG)
			{
				args[iterator] = value_create_long(va_arg(va, long));
			}
			else if (id == TYPE_FLOAT)
			{
				args[iterator] = value_create_float((float)va_arg(va, double));
			}
			else if (id == TYPE_DOUBLE)
			{
				args[iterator] = value_create_double(va_arg(va, double));
			}
			else if (id == TYPE_STRING)
			{
				const char *str = va_arg(va, const char *);

				args[iterator] = value_create_string(str, strlen(str));
			}
			else if (id == TYPE_PTR)
			{
				args[iterator] = value_create_ptr(va_arg(va, const void *));
			}
			else
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Calling metacallt_s with unsupported type '%s', using null type instead", type_id_name(id));
				args[iterator] = metacall_value_create_null();
			}
		}

		va_end(va);

		ret = function_call(f, args, size);

		for (iterator = 0; iterator < size; ++iterator)
		{
			value_type_destroy(args[iterator]);
		}

		return ret;
	}

	return NULL;
}

void *metacallht_s(void *handle, const char *name, const enum metacall_value_id ids[], size_t size, ...)
{
	if (loader_impl_handle_validate(handle) != 0)
	{
		// TODO: Implement type error return a value
		log_write("metacall", LOG_LEVEL_ERROR, "Handle %p passed to metacallht_s is not valid", handle);
		return NULL;
	}

	value f_val = loader_handle_get(handle, name);
	function f = NULL;

	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	if (f != NULL)
	{
		void *args[METACALL_ARGS_SIZE];

		value ret = NULL;

		signature s = function_signature(f);

		size_t iterator;

		va_list va;

		va_start(va, size);

		for (iterator = 0; iterator < size; ++iterator)
		{
			type t = signature_get_type(s, iterator);

			type_id id = type_index(t);

			if (t != NULL)
			{
				id = type_index(t);
			}
			else
			{
				id = ids[iterator];
			}

			if (id == TYPE_BOOL)
			{
				args[iterator] = value_create_bool((boolean)va_arg(va, unsigned int));
			}
			if (id == TYPE_CHAR)
			{
				args[iterator] = value_create_char((char)va_arg(va, int));
			}
			else if (id == TYPE_SHORT)
			{
				args[iterator] = value_create_short((short)va_arg(va, int));
			}
			else if (id == TYPE_INT)
			{
				args[iterator] = value_create_int(va_arg(va, int));
			}
			else if (id == TYPE_LONG)
			{
				args[iterator] = value_create_long(va_arg(va, long));
			}
			else if (id == TYPE_FLOAT)
			{
				args[iterator] = value_create_float((float)va_arg(va, double));
			}
			else if (id == TYPE_DOUBLE)
			{
				args[iterator] = value_create_double(va_arg(va, double));
			}
			else if (id == TYPE_STRING)
			{
				const char *str = va_arg(va, const char *);

				args[iterator] = value_create_string(str, strlen(str));
			}
			else if (id == TYPE_PTR)
			{
				args[iterator] = value_create_ptr(va_arg(va, const void *));
			}
			else
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Calling metacallht_s with unsupported type '%s', using null type instead", type_id_name(id));
				args[iterator] = metacall_value_create_null();
			}
		}

		va_end(va);

		ret = function_call(f, args, size);

		for (iterator = 0; iterator < size; ++iterator)
		{
			value_type_destroy(args[iterator]);
		}

		return ret;
	}

	return NULL;
}

void *metacall_function(const char *name)
{
	value f_val = loader_get(name);
	function f = NULL;

	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	return f;
}

void *metacall_handle_function(void *handle, const char *name)
{
	if (loader_impl_handle_validate(handle) != 0)
	{
		// TODO: Implement type error return a value
		log_write("metacall", LOG_LEVEL_ERROR, "Handle %p passed to metacall_handle_function is not valid", handle);
		return NULL;
	}

	value f_val = loader_handle_get(handle, name);
	function f = NULL;

	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	return f;
}

int metacall_function_parameter_type(void *func, size_t parameter, enum metacall_value_id *id)
{
	if (func != NULL)
	{
		function f = (function)func;
		signature s = function_signature(f);

		if (parameter < signature_count(s))
		{
			*id = type_index(signature_get_type(s, parameter));

			return 0;
		}
	}

	*id = METACALL_INVALID;

	return 1;
}

int metacall_function_return_type(void *func, enum metacall_value_id *id)
{
	if (func != NULL)
	{
		function f = (function)func;
		signature s = function_signature(f);

		*id = type_index(signature_get_return(s));

		return 0;
	}

	*id = METACALL_INVALID;

	return 1;
}

size_t metacall_function_size(void *func)
{
	function f = (function)func;

	if (f != NULL)
	{
		signature s = function_signature(f);

		return signature_count(s);
	}

	return 0;
}

int metacall_function_async(void *func)
{
	function f = (function)func;

	if (f != NULL)
	{
		return function_async_id(f);
	}

	return -1;
}

void *metacall_handle(const char *tag, const char *name)
{
	return (void *)loader_get_handle(tag, name);
}

const char *metacall_handle_id(void *handle)
{
	if (loader_impl_handle_validate(handle) != 0)
	{
		// TODO: Implement error handling
		log_write("metacall", LOG_LEVEL_ERROR, "Handle %p passed to metacall_handle_id is not valid", handle);
		return NULL;
	}

	return loader_handle_id(handle);
}

void *metacall_handle_export(void *handle)
{
	if (loader_impl_handle_validate(handle) != 0)
	{
		// TODO: Implement type error return a value
		log_write("metacall", LOG_LEVEL_ERROR, "Handle %p passed to metacall_handle_export is not valid", handle);
		return NULL;
	}

	return loader_handle_export(handle);
}

void *metacallfv(void *func, void *args[])
{
	function f = (function)func;

	if (f != NULL)
	{
		signature s = function_signature(f);

		return metacallfv_s(func, args, signature_count(s));
	}

	return NULL;
}

void *metacallfv_s(void *func, void *args[], size_t size)
{
	function f = (function)func;

	if (f != NULL)
	{
		signature s = function_signature(f);

		size_t iterator;

		value ret;

		for (iterator = 0; iterator < size; ++iterator)
		{
			if (value_validate(args[iterator]) != 0)
			{
				// TODO: Implement type error return a value
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid argument at position %" PRIuS " when calling to metacallfv_s", iterator);
				return NULL;
			}

			type t = signature_get_type(s, iterator);

			if (t != NULL)
			{
				type_id id = type_index(t);

				if (id != value_type_id((value)args[iterator]))
				{
					value cast_arg = value_type_cast((value)args[iterator], id);

					if (cast_arg != NULL)
					{
						args[iterator] = cast_arg;
					}
				}
			}
		}

		ret = function_call(f, args, size);

		if (ret != NULL)
		{
			type t = signature_get_return(s);

			if (t != NULL)
			{
				type_id id = type_index(t);

				if (id != value_type_id(ret))
				{
					value cast_ret = value_type_cast(ret, id);

					return (cast_ret == NULL) ? ret : cast_ret;
				}
			}
		}

		return ret;
	}

	return NULL;
}

void *metacallf(void *func, ...)
{
	function f = (function)func;

	if (f != NULL)
	{
		void *args[METACALL_ARGS_SIZE];

		value ret = NULL;

		signature s = function_signature(f);

		size_t iterator, args_count = signature_count(s);

		va_list va;

		va_start(va, func);

		for (iterator = 0; iterator < args_count; ++iterator)
		{
			type t = signature_get_type(s, iterator);

			type_id id = type_index(t);

			if (id == TYPE_BOOL)
			{
				args[iterator] = value_create_bool((boolean)va_arg(va, unsigned int));
			}
			if (id == TYPE_CHAR)
			{
				args[iterator] = value_create_char((char)va_arg(va, int));
			}
			else if (id == TYPE_SHORT)
			{
				args[iterator] = value_create_short((short)va_arg(va, int));
			}
			else if (id == TYPE_INT)
			{
				args[iterator] = value_create_int(va_arg(va, int));
			}
			else if (id == TYPE_LONG)
			{
				args[iterator] = value_create_long(va_arg(va, long));
			}
			else if (id == TYPE_FLOAT)
			{
				args[iterator] = value_create_float((float)va_arg(va, double));
			}
			else if (id == TYPE_DOUBLE)
			{
				args[iterator] = value_create_double(va_arg(va, double));
			}
			else if (id == TYPE_STRING)
			{
				const char *str = va_arg(va, const char *);

				args[iterator] = value_create_string(str, strlen(str));
			}
			else if (id == TYPE_PTR)
			{
				args[iterator] = value_create_ptr(va_arg(va, const void *));
			}
			else
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Calling metacallf with unsupported type '%s', using null type instead", type_id_name(id));
				args[iterator] = metacall_value_create_null();
			}
		}

		va_end(va);

		ret = function_call(f, args, args_count);

		for (iterator = 0; iterator < args_count; ++iterator)
		{
			value_type_destroy(args[iterator]);
		}

		return ret;
	}

	return NULL;
}

void *metacallfs(void *func, const char *buffer, size_t size, void *allocator)
{
	function f = (function)func;

	if (f != NULL)
	{
		signature s = function_signature(f);

		if (buffer == NULL || size == 0)
		{
			if (signature_count(s) == 0)
			{
				value ret = function_call(f, metacall_null_args, 0);

				if (ret != NULL)
				{
					type t = signature_get_return(s);

					if (t != NULL)
					{
						type_id id = type_index(t);

						if (id != value_type_id(ret))
						{
							value cast_ret = value_type_cast(ret, id);

							return (cast_ret == NULL) ? ret : cast_ret;
						}
					}
				}

				return ret;
			}

			return NULL;
		}
		else
		{
			void *args[METACALL_ARGS_SIZE];

			value *v_array, ret, v = (value)metacall_deserialize(metacall_serial(), buffer, size, allocator);

			size_t iterator, args_count;

			if (v == NULL)
			{
				return NULL;
			}

			if (type_id_array(value_type_id(v)) != 0)
			{
				value_type_destroy(v);

				return NULL;
			}

			args_count = value_type_count(v);

			v_array = value_to_array(v);

			for (iterator = 0; iterator < args_count; ++iterator)
			{
				/* This step is necessary in order to handle type castings */
				args[iterator] = metacall_value_copy(v_array[iterator]);
			}

			ret = metacallfv_s(f, args, args_count);

			/* This step is necessary in order to handle type castings (otherwise it generates leaks) */
			for (iterator = 0; iterator < args_count; ++iterator)
			{
				value_type_destroy(args[iterator]);
			}

			value_type_destroy(v);

			if (ret != NULL)
			{
				type t = signature_get_return(s);

				if (t != NULL)
				{
					type_id id = type_index(t);

					if (id != value_type_id(ret))
					{
						value cast_ret = value_type_cast(ret, id);

						if (cast_ret != NULL)
						{
							ret = cast_ret;
						}
					}
				}
			}

			return ret;
		}
	}

	return NULL;
}

void *metacallfmv(void *func, void *keys[], void *values[])
{
	function f = (function)func;

	if (f != NULL)
	{
		void *args[METACALL_ARGS_SIZE];

		signature s = function_signature(f);

		size_t iterator, args_count = signature_count(s);

		value ret;

		for (iterator = 0; iterator < args_count; ++iterator)
		{
			if (value_validate(keys[iterator]) != 0)
			{
				// TODO: Implement type error return a value
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid key at position %" PRIuS " when calling to metacallfmv", iterator);
				return NULL;
			}

			if (value_validate(values[iterator]) != 0)
			{
				// TODO: Implement type error return a value
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid value at position %" PRIuS " when calling to metacallfmv", iterator);
				return NULL;
			}

			type_id key_id = value_type_id((value)keys[iterator]);

			size_t index = METACALL_ARGS_SIZE;

			/* Obtain signature index */
			if (type_id_string(key_id) == 0)
			{
				const char *key = value_to_string(keys[iterator]);

				index = signature_get_index(s, key);
			}
			else if (type_id_integer(key_id) == 0)
			{
				value cast_key = value_type_cast((value)keys[iterator], TYPE_INT);

				int key_index;

				if (cast_key != NULL)
				{
					keys[iterator] = cast_key;
				}

				key_index = value_to_int((value)keys[iterator]);

				if (key_index >= 0 && key_index < METACALL_ARGS_SIZE)
				{
					index = (size_t)key_index;
				}
			}

			/* If index is valid, cast values and build arguments */
			if (index < METACALL_ARGS_SIZE)
			{
				type t = signature_get_type(s, iterator);

				if (t != NULL)
				{
					type_id id = type_index(t);

					if (id != value_type_id((value)values[iterator]))
					{
						value cast_arg = value_type_cast((value)values[iterator], id);

						if (cast_arg != NULL)
						{
							values[iterator] = cast_arg;
						}
					}
				}

				args[index] = values[iterator];
			}
			else
			{
				/* TODO: Handle properly exceptions */
				return NULL;
			}
		}

		ret = function_call(f, args, args_count);

		if (ret != NULL)
		{
			type t = signature_get_return(s);

			if (t != NULL)
			{
				type_id id = type_index(t);

				if (id != value_type_id(ret))
				{
					value cast_ret = value_type_cast(ret, id);

					return (cast_ret == NULL) ? ret : cast_ret;
				}
			}
		}

		return ret;
	}

	return NULL;
}

void *metacallfms(void *func, const char *buffer, size_t size, void *allocator)
{
	function f = (function)func;

	if (f != NULL)
	{
		signature s = function_signature(f);

		if (buffer == NULL || size == 0)
		{
			if (signature_count(s) == 0)
			{
				value ret = function_call(f, metacall_null_args, 0);

				if (ret != NULL)
				{
					type t = signature_get_return(s);

					if (t != NULL)
					{
						type_id id = type_index(t);

						if (id != value_type_id(ret))
						{
							value cast_ret = value_type_cast(ret, id);

							return (cast_ret == NULL) ? ret : cast_ret;
						}
					}
				}

				return ret;
			}

			return NULL;
		}
		else
		{
			void *keys[METACALL_ARGS_SIZE];
			void *values[METACALL_ARGS_SIZE];

			value *v_map, ret, v = (value)metacall_deserialize(metacall_serial(), buffer, size, allocator);

			size_t iterator, args_count;

			if (v == NULL)
			{
				return NULL;
			}

			if (type_id_map(value_type_id(v)) != 0)
			{
				value_type_destroy(v);
				return NULL;
			}

			args_count = signature_count(s);

			/* TODO: No optional arguments allowed, review in the future */
			if (args_count != value_type_count(v))
			{
				value_type_destroy(v);
				return NULL;
			}

			v_map = value_to_map(v);

			for (iterator = 0; iterator < args_count; ++iterator)
			{
				value element = v_map[iterator];
				value *v_element = value_to_array(element);

				/* This step is necessary in order to handle type castings */
				keys[iterator] = value_type_copy(v_element[0]);
				values[iterator] = value_type_copy(v_element[1]);
			}

			ret = metacallfmv(f, keys, values);

			/* This step is necessary in order to handle type castings (otherwise it generates leaks) */
			for (iterator = 0; iterator < args_count; ++iterator)
			{
				value_type_destroy(keys[iterator]);
				value_type_destroy(values[iterator]);
			}

			value_type_destroy(v);

			if (ret != NULL)
			{
				type t = signature_get_return(s);

				if (t != NULL)
				{
					type_id id = type_index(t);

					if (id != value_type_id(ret))
					{
						value cast_ret = value_type_cast(ret, id);

						if (cast_ret != NULL)
						{
							ret = cast_ret;
						}
					}
				}
			}

			return ret;
		}
	}

	return NULL;
}

int metacall_register(const char *name, void *(*invoke)(size_t, void *[], void *), void **func, enum metacall_value_id return_type, size_t size, ...)
{
	type_id types[METACALL_ARGS_SIZE];

	va_list va;

	va_start(va, size);

	size_t iterator;

	for (iterator = 0; iterator < size; ++iterator)
	{
		types[iterator] = (type_id)va_arg(va, int);
	}

	va_end(va);

	return loader_register(name, (loader_register_invoke)invoke, (function *)func, (type_id)return_type, size, (type_id *)types);
}

int metacall_registerv(const char *name, void *(*invoke)(size_t, void *[], void *), void **func, enum metacall_value_id return_type, size_t size, enum metacall_value_id types[])
{
	return loader_register(name, (loader_register_invoke)invoke, (function *)func, (type_id)return_type, size, (type_id *)types);
}

void *metacall_await(const char *name, void *args[], void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data)
{
	value f_val = loader_get(name);
	function f = NULL;

	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	signature s = function_signature(f);

	return function_await(f, args, signature_count(s), resolve_callback, reject_callback, data);
}

void *metacall_await_future(void *f, void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data)
{
	if (f != NULL)
	{
		return future_await((future)f, resolve_callback, reject_callback, data);
	}

	// TODO: Error handling
	return NULL;
}

void *metacall_await_s(const char *name, void *args[], size_t size, void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data)
{
	value f_val = loader_get(name);
	function f = NULL;

	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	return function_await(f, args, size, resolve_callback, reject_callback, data);
}

void *metacallfv_await(void *func, void *args[], void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data)
{
	function f = (function)func;

	signature s = function_signature(f);

	return function_await(func, args, signature_count(s), resolve_callback, reject_callback, data);
}

void *metacallfv_await_s(void *func, void *args[], size_t size, void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data)
{
	return function_await(func, args, size, resolve_callback, reject_callback, data);
}

void *metacallfv_await_struct_s(void *func, void *args[], size_t size, metacall_await_callbacks cb, void *data)
{
	return function_await(func, args, size, cb.resolve, cb.reject, data);
}

void *metacallfmv_await(void *func, void *keys[], void *values[], void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data)
{
	function f = (function)func;

	if (f != NULL)
	{
		signature s = function_signature(f);

		return metacallfmv_await_s(func, keys, values, signature_count(s), resolve_callback, reject_callback, data);
	}

	// TODO: Error handling
	return NULL;
}

/* TODO: Unify code between metacallfmv and metacallfmv_await_s */
void *metacallfmv_await_s(void *func, void *keys[], void *values[], size_t size, void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data)
{
	function f = (function)func;

	if (f != NULL)
	{
		void *args[METACALL_ARGS_SIZE];

		signature s = function_signature(f);

		size_t iterator;

		value ret;

		for (iterator = 0; iterator < size; ++iterator)
		{
			if (value_validate(keys[iterator]) != 0)
			{
				// TODO: Implement type error return a value
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid key at position %" PRIuS " when calling to metacallfmv_await_s", iterator);
				return NULL;
			}

			if (value_validate(values[iterator]) != 0)
			{
				// TODO: Implement type error return a value
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid value at position %" PRIuS " when calling to metacallfmv_await_s", iterator);
				return NULL;
			}

			type_id key_id = value_type_id((value)keys[iterator]);

			size_t index = METACALL_ARGS_SIZE;

			/* Obtain signature index */
			if (type_id_string(key_id) == 0)
			{
				const char *key = value_to_string(keys[iterator]);

				index = signature_get_index(s, key);
			}
			else if (type_id_integer(key_id) == 0)
			{
				value cast_key = value_type_cast((value)keys[iterator], TYPE_INT);

				int key_index;

				if (cast_key != NULL)
				{
					keys[iterator] = cast_key;
				}

				key_index = value_to_int((value)keys[iterator]);

				if (key_index >= 0 && key_index < METACALL_ARGS_SIZE)
				{
					index = (size_t)key_index;
				}
			}

			/* If index is valid, cast values and build arguments */
			if (index < METACALL_ARGS_SIZE)
			{
				type t = signature_get_type(s, iterator);

				if (t != NULL)
				{
					type_id id = type_index(t);

					if (id != value_type_id((value)values[iterator]))
					{
						value cast_arg = value_type_cast((value)values[iterator], id);

						if (cast_arg != NULL)
						{
							values[iterator] = cast_arg;
						}
					}
				}

				args[index] = values[iterator];
			}
			else
			{
				/* TODO: Handle properly exceptions */
				return NULL;
			}
		}

		ret = function_await(f, args, size, resolve_callback, reject_callback, data);

		if (ret != NULL)
		{
			type t = signature_get_return(s);

			if (t != NULL)
			{
				type_id id = type_index(t);

				if (id != value_type_id(ret))
				{
					value cast_ret = value_type_cast(ret, id);

					return (cast_ret == NULL) ? ret : cast_ret;
				}
			}
		}

		return ret;
	}

	return NULL;
}

void *metacallfs_await(void *func, const char *buffer, size_t size, void *allocator, void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data)
{
	function f = (function)func;

	if (f != NULL)
	{
		signature s = function_signature(f);

		if (buffer == NULL || size == 0)
		{
			if (signature_count(s) == 0)
			{
				value ret = function_call(f, metacall_null_args, 0);

				if (ret != NULL)
				{
					type t = signature_get_return(s);

					if (t != NULL)
					{
						type_id id = type_index(t);

						if (id != value_type_id(ret))
						{
							value cast_ret = value_type_cast(ret, id);

							return (cast_ret == NULL) ? ret : cast_ret;
						}
					}
				}

				return ret;
			}

			return NULL;
		}
		else
		{
			void *args[METACALL_ARGS_SIZE];

			value *v_array, ret, v = (value)metacall_deserialize(metacall_serial(), buffer, size, allocator);

			size_t iterator, args_count;

			if (v == NULL)
			{
				return NULL;
			}

			if (type_id_array(value_type_id(v)) != 0)
			{
				value_type_destroy(v);

				return NULL;
			}

			args_count = value_type_count(v);

			v_array = value_to_array(v);

			for (iterator = 0; iterator < args_count; ++iterator)
			{
				/* This step is necessary in order to handle type castings */
				args[iterator] = value_type_copy(v_array[iterator]);
			}

			ret = metacallfv_await(f, args, resolve_callback, reject_callback, data);

			/* This step is necessary in order to handle type castings (otherwise it generates leaks) */
			for (iterator = 0; iterator < args_count; ++iterator)
			{
				value_type_destroy(args[iterator]);
			}

			value_type_destroy(v);

			if (ret != NULL)
			{
				type t = signature_get_return(s);

				if (t != NULL)
				{
					type_id id = type_index(t);

					if (id != value_type_id(ret))
					{
						value cast_ret = value_type_cast(ret, id);

						if (cast_ret != NULL)
						{
							ret = cast_ret;
						}
					}
				}
			}

			return ret;
		}
	}

	return NULL;
}

/* TODO: Unify code between metacallfms and metacallfms_await */
void *metacallfms_await(void *func, const char *buffer, size_t size, void *allocator, void *(*resolve_callback)(void *, void *), void *(*reject_callback)(void *, void *), void *data)
{
	function f = (function)func;

	if (f != NULL)
	{
		signature s = function_signature(f);

		if (buffer == NULL || size == 0)
		{
			if (signature_count(s) == 0)
			{
				value ret = function_call(f, metacall_null_args, 0);

				if (ret != NULL)
				{
					type t = signature_get_return(s);

					if (t != NULL)
					{
						type_id id = type_index(t);

						if (id != value_type_id(ret))
						{
							value cast_ret = value_type_cast(ret, id);

							return (cast_ret == NULL) ? ret : cast_ret;
						}
					}
				}

				return ret;
			}

			return NULL;
		}
		else
		{
			void *keys[METACALL_ARGS_SIZE];
			void *values[METACALL_ARGS_SIZE];

			value *v_map, ret, v = (value)metacall_deserialize(metacall_serial(), buffer, size, allocator);

			size_t iterator, args_count;

			if (v == NULL)
			{
				return NULL;
			}

			if (type_id_map(value_type_id(v)) != 0)
			{
				value_type_destroy(v);

				return NULL;
			}

			args_count = signature_count(s);

			/* TODO: No optional arguments allowed, review in the future */
			if (args_count != value_type_count(v))
			{
				value_type_destroy(v);

				return NULL;
			}

			v_map = value_to_map(v);

			for (iterator = 0; iterator < args_count; ++iterator)
			{
				value element = v_map[iterator];
				value *v_element = value_to_array(element);

				/* This step is necessary in order to handle type castings */
				keys[iterator] = value_type_copy(v_element[0]);
				values[iterator] = value_type_copy(v_element[1]);
			}

			ret = metacallfmv_await(f, keys, values, resolve_callback, reject_callback, data);

			/* This step is necessary in order to handle type castings (otherwise it generates leaks) */
			for (iterator = 0; iterator < args_count; ++iterator)
			{
				value_type_destroy(keys[iterator]);
				value_type_destroy(values[iterator]);
			}

			value_type_destroy(v);

			if (ret != NULL)
			{
				type t = signature_get_return(s);

				if (t != NULL)
				{
					type_id id = type_index(t);

					if (id != value_type_id(ret))
					{
						value cast_ret = value_type_cast(ret, id);

						if (cast_ret != NULL)
						{
							ret = cast_ret;
						}
					}
				}
			}

			return ret;
		}
	}

	return NULL;
}

void *metacall_class(const char *name)
{
	value c_val = loader_get(name);
	klass c = NULL;

	if (value_type_id(c_val) == TYPE_CLASS)
	{
		c = value_to_class(c_val);
	}

	return c;
}

void *metacall_class_new(void *cls, const char *name, void *args[], size_t size)
{
	type_id *ids = metacall_type_ids(args, size);

	constructor ctor = class_constructor(cls, ids, size);

	object o = class_new(cls, name, ctor, args, size);

	if (ids != NULL)
	{
		free(ids);
	}

	if (o == NULL)
	{
		return NULL;
	}

	value v = value_create_object(o);

	if (v == NULL)
	{
		object_destroy(o);
	}

	return v;
}

void *metacall_class_static_get(void *cls, const char *key)
{
	return class_static_get(cls, key);
}

int metacall_class_static_set(void *cls, const char *key, void *v)
{
	return class_static_set(cls, key, v);
}

void *metacallv_method(void *target, const char *name, method_invoke_ptr call, vector v, void *args[], size_t size)
{
	if (v == NULL)
	{
		// TODO: Implement type error return a value
		log_write("metacall", LOG_LEVEL_ERROR, "Method %s in %p is not implemented (bad allocation)", name, target);
		return NULL;
	}

	if (vector_size(v) == 0)
	{
		// TODO: Implement type error return a value
		log_write("metacall", LOG_LEVEL_ERROR, "Method %s in %p is not implemented", name, target);
		vector_destroy(v);
		return NULL;
	}

	if (vector_size(v) > 1)
	{
		// TODO: Implement type error return a value
		log_write("metacall", LOG_LEVEL_ERROR, "Method %s in %p is overloaded, you should use 'metacallt_class' instead for disambiguate the call", name, target);
		vector_destroy(v);
		return NULL;
	}

	method m = vector_at_type(v, 0, method);

	if (m == NULL)
	{
		// TODO: Implement type error return a value
		log_write("metacall", LOG_LEVEL_ERROR, "Method %s in %p is invalid (NULL)", name, target);
		vector_destroy(v);
		return NULL;
	}

	signature s = method_signature(m);
	size_t iterator;

	for (iterator = 0; iterator < size; ++iterator)
	{
		if (value_validate(args[iterator]) != 0)
		{
			// TODO: Implement type error return a value
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid argument at position %" PRIuS " when calling to metacallv_method", iterator);
			vector_destroy(v);
			return NULL;
		}

		type t = signature_get_type(s, iterator);

		if (t != NULL)
		{
			type_id id = type_index(t);

			if (id != value_type_id((value)args[iterator]))
			{
				value cast_arg = value_type_cast((value)args[iterator], id);

				if (cast_arg != NULL)
				{
					args[iterator] = cast_arg;
				}
			}
		}
	}

	value ret = call(target, m, args, size);

	vector_destroy(v);

	if (ret != NULL)
	{
		type t = signature_get_return(s);

		if (t != NULL)
		{
			type_id id = type_index(t);

			if (id != value_type_id(ret))
			{
				value cast_ret = value_type_cast(ret, id);

				return (cast_ret == NULL) ? ret : cast_ret;
			}
		}
	}

	return ret;
}

void *metacallv_class(void *cls, const char *name, void *args[], size_t size)
{
	return metacallv_method(cls, name, (method_invoke_ptr)&class_static_call, class_static_methods(cls, name), args, size);
}

type_id *metacall_type_ids(void *args[], size_t size)
{
	type_id *ids = NULL;

	if (size > 0)
	{
		ids = (type_id *)malloc(sizeof(type_id) * size);

		for (size_t iterator = 0; iterator < size; ++iterator)
		{
			ids[iterator] = metacall_value_id(args[iterator]);
		}
	}

	return ids;
}

void *metacallt_class(void *cls, const char *name, const enum metacall_value_id ret, void *args[], size_t size)
{
	type_id *ids = metacall_type_ids(args, size);

	method m = class_static_method(cls, name, ret, ids, size);

	if (ids != NULL)
	{
		free(ids);
	}

	if (m == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Method %s in class <%p> is not implemented with the parameter types being received", name, cls);
		return NULL;
	}

	return class_static_call(cls, m, args, size);
}

void *metacallv_object(void *obj, const char *name, void *args[], size_t size)
{
	return metacallv_method(obj, name, (method_invoke_ptr)&object_call, object_methods(obj, name), args, size);
}

void *metacallt_object(void *obj, const char *name, const enum metacall_value_id ret, void *args[], size_t size)
{
	type_id *ids = NULL;

	if (size > 0)
	{
		ids = (type_id *)malloc(sizeof(type_id) * size);

		for (size_t iterator = 0; iterator < size; ++iterator)
		{
			ids[iterator] = metacall_value_id(args[iterator]);
		}
	}

	method m = object_method(obj, name, ret, ids, size);

	if (ids != NULL)
	{
		free(ids);
	}

	if (m == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Method %s in object <%p> is not implemented with the parameter types being received", name, obj);
		return NULL;
	}

	return object_call(obj, m, args, size);
}

void *metacall_object_get(void *obj, const char *key)
{
	return object_get(obj, key);
}

int metacall_object_set(void *obj, const char *key, void *v)
{
	return object_set(obj, key, v);
}

void *metacall_throwable_value(void *th)
{
	return throwable_value(th);
}

char *metacall_inspect(size_t *size, void *allocator)
{
	serial s;

	value v = loader_metadata();

	char *str;

	if (v == NULL)
	{
		v = value_create_map(NULL, 0);

		if (v == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid MetaCall inspect map creation");

			return NULL;
		}
	}

	s = serial_create(metacall_serial());

	str = serial_serialize(s, v, size, allocator);

	value_type_destroy(v);

	return str;
}

char *metacall_serialize(const char *name, void *v, size_t *size, void *allocator)
{
	serial s = serial_create(name);

	return serial_serialize(s, (value)v, size, (memory_allocator)allocator);
}

void *metacall_deserialize(const char *name, const char *buffer, size_t size, void *allocator)
{
	serial s = serial_create(name);

	return (void *)serial_deserialize(s, buffer, size, (memory_allocator)allocator);
}

int metacall_clear(void *handle)
{
	if (loader_impl_handle_validate(handle) != 0)
	{
		/* TODO: Implement error handling */
		log_write("metacall", LOG_LEVEL_ERROR, "Handle %p passed to metacall_clear is not valid", handle);
		return 1;
	}

	return loader_clear(handle);
}

int metacall_destroy(void)
{
	if (metacall_initialize_flag == 0)
	{
		/* Destroy loaders */
		loader_destroy();

		/* Destroy configurations */
		configuration_destroy();

		metacall_initialize_flag = 1;

		/* Print stats from functions, classes and objects */
		function_stats_debug();
		class_stats_debug();
		object_stats_debug();
		exception_stats_debug();

		/* Unregister backtrace */
		if (backtrace_destroy() != 0)
		{
			log_write("metacall", LOG_LEVEL_WARNING, "MetaCall backtrace could not be destroyed");
		}
	}

	return 0;
}

const struct metacall_version_type *metacall_version(void)
{
	static const struct metacall_version_type version = {
		METACALL_VERSION_MAJOR_ID,
		METACALL_VERSION_MINOR_ID,
		METACALL_VERSION_PATCH_ID,
		METACALL_VERSION_REVISION,
		METACALL_VERSION,
		METACALL_NAME_VERSION
	};

	return &version;
}

uint32_t metacall_version_hex_make(unsigned int major, unsigned int minor, unsigned int patch)
{
	const uint32_t version_hex = (major << 0x18) + (minor << 0x10) + patch;

	return version_hex;
}

uint32_t metacall_version_hex(void)
{
	static const uint32_t version_hex =
		(METACALL_VERSION_MAJOR_ID << 0x18) +
		(METACALL_VERSION_MINOR_ID << 0x10) +
		METACALL_VERSION_PATCH_ID;

	return version_hex;
}

const char *metacall_version_str(void)
{
	static const char version_string[] = METACALL_VERSION;

	return version_string;
}

const char *metacall_version_revision(void)
{
	static const char version_revision[] = METACALL_VERSION_REVISION;

	return version_revision;
}

const char *metacall_version_name(void)
{
	static const char version_name[] = METACALL_NAME_VERSION;

	return version_name;
}

const char *metacall_print_info(void)
{
	static const char metacall_info[] =
		"MetaCall Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef METACALL_STATIC_DEFINE
		"Compiled as static library type"
#else
		"Compiled as shared library type"
#endif

		"\n";

	return metacall_info;
}
