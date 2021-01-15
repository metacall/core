/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

 /* -- Headers -- */

#include <metacall/metacall_version.h>
#include <metacall/metacall_loaders.h>
#include <metacall/metacall.h>

#include <loader/loader.h>

#include <reflect/reflect.h>

#include <configuration/configuration.h>

#include <log/log.h>

#include <serial/serial.h>

#include <stdio.h>
#include <string.h>

/* -- Definitions -- */

#define METACALL_ARGS_SIZE	0x10
#define METACALL_SERIAL		"rapid_json"

/* -- Global Variables -- */

void * metacall_null_args[1];

/* -- Private Variables -- */

static int metacall_initialize_flag = 1;
static int metacall_log_null_flag = 1;
static int metacall_config_flags = 0;

/* -- Methods -- */

const char * metacall_serial()
{
	static const char metacall_serial_str[] = METACALL_SERIAL;

	return metacall_serial_str;
}

void metacall_log_null()
{
	metacall_log_null_flag = 0;
}

void metacall_flags(int flags)
{
	metacall_config_flags = flags;
}

int metacall_initialize()
{
	loader l = loader_singleton();

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
		log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall already initialized <%p>", (void *)l);

		return 0;
	}

	log_write("metacall", LOG_LEVEL_DEBUG, "Initializing MetaCall <%p>", (void *)l);

	metacall_null_args[0] = NULL;

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
			value * level = configuration_value(config, "log_level");

			if (level != NULL)
			{
				const char * level_str = (const char *)value_to_string(level);

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

	loader_initialize();

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

int metacall_is_initialized(const char * tag)
{
	return loader_is_initialized(tag);
}

size_t metacall_args_size()
{
	const size_t args_size = METACALL_ARGS_SIZE;

	return args_size;
}

int metacall_execution_path(const char * tag, const char * path)
{
	loader_naming_path path_impl;

	if (tag == NULL || path == NULL)
	{
		return 1;
	}

	strncpy(path_impl, path, LOADER_NAMING_PATH_SIZE);

	return loader_execution_path(tag, path_impl);
}

int metacall_execution_path_s(const char * tag, size_t tag_length, const char * path, size_t path_length)
{
	loader_naming_path path_impl;
	loader_naming_tag tag_impl;

	if (tag == NULL || tag_length == 0 || tag_length >= LOADER_NAMING_TAG_SIZE || path == NULL || path_length == 0 || path_length >= LOADER_NAMING_PATH_SIZE)
	{
		return 1;
	}

	strncpy(path_impl, path, path_length);
	strncpy(tag_impl, tag, tag_length);

	path_impl[path_length] = '\0';
	tag_impl[tag_length] = '\0';

	return loader_execution_path(tag_impl, path_impl);
}

int metacall_load_from_file(const char * tag, const char * paths[], size_t size, void ** handle)
{
	loader_naming_path path_impl[LOADER_LOAD_FROM_FILES_SIZE];

	size_t iterator;

	for (iterator = 0; iterator < size; ++iterator)
	{
		strncpy(path_impl[iterator], paths[iterator], LOADER_NAMING_PATH_SIZE);
	}

	return loader_load_from_file(tag, (const loader_naming_path *)path_impl, size, handle);
}

int metacall_load_from_memory(const char * tag, const char * buffer, size_t size, void ** handle)
{
	return loader_load_from_memory(tag, buffer, size, handle);
}

int metacall_load_from_package(const char * tag, const char * path, void ** handle)
{
	return loader_load_from_package(tag, path, handle);
}

int metacall_load_from_configuration(const char * path, void ** handle, void * allocator)
{
	return loader_load_from_configuration(path, handle, allocator);
}

void * metacallv(const char * name, void * args[])
{
	value f_val = loader_get(name);

	function f = NULL;
	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	return metacallfv(f, args);
}

void * metacallv_s(const char * name, void * args[], size_t size)
{
	value f_val = loader_get(name);

	function f = NULL;
	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	return metacallfv_s(f, args, size);
}

void * metacallhv(void * handle, const char * name, void * args[])
{
	(void)handle;
	(void)name;
	(void)args;

	/* TODO */

	return NULL;
}

void * metacall(const char * name, ...)
{
	value f_val = loader_get(name);

	function f = NULL;
	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	if (f != NULL)
	{
		void * args[METACALL_ARGS_SIZE];

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
				const char * str = va_arg(va, const char *);

				args[iterator] = value_create_string(str, strlen(str));
			}
			else if (id == TYPE_PTR)
			{
				args[iterator] = value_create_ptr(va_arg(va, const void *));
			}
			else
			{
				args[iterator] = NULL;
			}
		}

		va_end(va);

		ret = function_call(f, args, args_count);

		for (iterator = 0; iterator < args_count; ++iterator)
		{
			value_destroy(args[iterator]);
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

void * metacallt(const char * name, const enum metacall_value_id ids[], ...)
{
	value f_val = loader_get(name);

	function f = NULL;
	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	if (f != NULL)
	{
		void * args[METACALL_ARGS_SIZE];

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
				const char * str = va_arg(va, const char *);

				args[iterator] = value_create_string(str, strlen(str));
			}
			else if (id == TYPE_PTR)
			{
				args[iterator] = value_create_ptr(va_arg(va, const void *));
			}
			else
			{
				args[iterator] = NULL;
			}
		}

		va_end(va);

		ret = function_call(f, args, args_count);

		for (iterator = 0; iterator < args_count; ++iterator)
		{
			value_destroy(args[iterator]);
		}

		return ret;
	}

	return NULL;
}

void * metacallt_s(const char * name, const enum metacall_value_id ids[], size_t size, ...)
{
	value f_val = loader_get(name);

	function f = NULL;
	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	if (f != NULL)
	{
		void * args[METACALL_ARGS_SIZE];

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
				const char * str = va_arg(va, const char *);

				args[iterator] = value_create_string(str, strlen(str));
			}
			else if (id == TYPE_PTR)
			{
				args[iterator] = value_create_ptr(va_arg(va, const void *));
			}
			else
			{
				args[iterator] = NULL;
			}
		}

		va_end(va);

		ret = function_call(f, args, size);

		for (iterator = 0; iterator < size; ++iterator)
		{
			value_destroy(args[iterator]);
		}

		return ret;
	}

	return NULL;
}

void * metacall_function(const char * name)
{
	value f_val = loader_get(name);

	function f = NULL;

	if (value_type_id(f_val) == TYPE_FUNCTION)
	{
		f = value_to_function(f_val);
	}

	return f;
}

size_t metacall_function_size(void * func)
{
	function f = (function)func;

	if (f != NULL)
	{
		signature s = function_signature(f);

		return signature_count(s);
	}

	return 0;
}

void * metacall_handle(const char * tag, const char * name)
{
	return (void *)loader_get_handle(tag, name);
}

const char * metacall_handle_id(void * handle)
{
	if (handle == NULL)
	{
		return NULL;
	}

	return loader_handle_id(handle);
}

void * metacall_handle_export(void * handle)
{
	if (handle == NULL)
	{
		return NULL;
	}

	return loader_handle_export(handle);
}

void * metacallfv(void * func, void * args[])
{
	function f = (function)func;

	if (f != NULL)
	{
		signature s = function_signature(f);

		return metacallfv_s(func, args, signature_count(s));
	}

	return NULL;
}

void * metacallfv_s(void * func, void * args[], size_t size)
{
	function f = (function)func;

	if (f != NULL)
	{
		signature s = function_signature(f);

		size_t iterator;

		value ret;

		for (iterator = 0; iterator < size; ++iterator)
		{
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

void * metacallf(void * func, ...)
{
	function f = (function)func;

	if (f != NULL)
	{
		void * args[METACALL_ARGS_SIZE];

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
				const char * str = va_arg(va, const char *);

				args[iterator] = value_create_string(str, strlen(str));
			}
			else if (id == TYPE_PTR)
			{
				args[iterator] = value_create_ptr(va_arg(va, const void *));
			}
			else
			{
				args[iterator] = NULL;
			}
		}

		va_end(va);

		ret = function_call(f, args, args_count);

		for (iterator = 0; iterator < args_count; ++iterator)
		{
			value_destroy(args[iterator]);
		}

		return ret;
	}

	return NULL;
}

void * metacallfs(void * func, const char * buffer, size_t size, void * allocator)
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
			void * args[METACALL_ARGS_SIZE];

			value * v_array, ret, v = (value)metacall_deserialize(metacall_serial(), buffer, size, allocator);

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
				args[iterator] = v_array[iterator];
			}

			ret = metacallfv_s(f, args, args_count);

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

			value_destroy(v);

			return ret;
		}
	}

	return NULL;
}

void * metacallfmv(void * func, void * keys[], void * values[])
{
	function f = (function)func;

	if (f != NULL)
	{
		void * args[METACALL_ARGS_SIZE];

		signature s = function_signature(f);

		size_t iterator, args_count = signature_count(s);

		value ret;

		for (iterator = 0; iterator < args_count; ++iterator)
		{
			type_id key_id = value_type_id((value)keys[iterator]);

			size_t index = METACALL_ARGS_SIZE;

			/* Obtain signature index */
			if (type_id_string(key_id) == 0)
			{
				const char * key = value_to_string(keys[iterator]);

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

void * metacallfms(void * func, const char * buffer, size_t size, void * allocator)
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
			void * keys[METACALL_ARGS_SIZE];
			void * values[METACALL_ARGS_SIZE];

			value * v_map, ret, v = (value)metacall_deserialize(metacall_serial(), buffer, size, allocator);

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

				value * v_element = value_to_array(element);

				keys[iterator] = v_element[0];
				values[iterator] = v_element[1];
			}

			ret = metacallfmv(f, keys, values);

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

			for (iterator = 0; iterator < args_count; ++iterator)
			{
				/* Due to casting, destroy must be done to arrays instead of to the map */
				value_destroy(keys[iterator]);
				value_destroy(values[iterator]);
				value_destroy(v_map[iterator]);
			}

			value_destroy(v);

			return ret;
		}
	}

	return NULL;
}

int metacall_register(const char * name, void * (*invoke)(size_t, void * [], void *), void ** func, enum metacall_value_id return_type, size_t size, ...)
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

void * metacall_await(const char * name, void * args[], void * (*resolve_callback)(void *, void *), void * (*reject_callback)(void *, void *), void * data)
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

void * metacallfv_await(void * func, void * args[], void * (*resolve_callback)(void *, void *), void * (*reject_callback)(void *, void *), void * data)
{
	function f = (function)func;

	signature s = function_signature(f);

	return function_await(func, args, signature_count(s), resolve_callback, reject_callback, data);
}

void * metacallfs_await(void * func, const char * buffer, size_t size, void * allocator, void * (*resolve_callback)(void *, void *), void * (*reject_callback)(void *, void *), void * data)
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
			void * args[METACALL_ARGS_SIZE];

			value * v_array, ret, v = (value)metacall_deserialize(metacall_serial(), buffer, size, allocator);

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
				args[iterator] = v_array[iterator];
			}

			ret = metacallfv_await(f, args, resolve_callback, reject_callback, data);

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

			value_destroy(v);

			return ret;
		}
	}

	return NULL;
}

/* TODO: Unify code between metacallfmv and metacallfmv_await */
void * metacallfmv_await(void * func, void * keys[], void * values[], void * (*resolve_callback)(void *, void *), void * (*reject_callback)(void *, void *), void * data)
{
	function f = (function)func;

	if (f != NULL)
	{
		void * args[METACALL_ARGS_SIZE];

		signature s = function_signature(f);

		size_t iterator, args_count = signature_count(s);

		value ret;

		for (iterator = 0; iterator < args_count; ++iterator)
		{
			type_id key_id = value_type_id((value)keys[iterator]);

			size_t index = METACALL_ARGS_SIZE;

			/* Obtain signature index */
			if (type_id_string(key_id) == 0)
			{
				const char * key = value_to_string(keys[iterator]);

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

		ret = function_await(f, args, args_count, resolve_callback, reject_callback, data);

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

/* TODO: Unify code between metacallfms and metacallfms_await */
void * metacallfms_await(void * func, const char * buffer, size_t size, void * allocator, void * (*resolve_callback)(void *, void *), void * (*reject_callback)(void *, void *), void * data)
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
			void * keys[METACALL_ARGS_SIZE];
			void * values[METACALL_ARGS_SIZE];

			value * v_map, ret, v = (value)metacall_deserialize(metacall_serial(), buffer, size, allocator);

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

				value * v_element = value_to_array(element);

				keys[iterator] = v_element[0];
				values[iterator] = v_element[1];
			}

			ret = metacallfmv_await(f, keys, values, resolve_callback, reject_callback, data);

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

			for (iterator = 0; iterator < args_count; ++iterator)
			{
				/* Due to casting, destroy must be done to arrays instead of to the map */
				value_destroy(keys[iterator]);
				value_destroy(values[iterator]);
				value_destroy(v_map[iterator]);
			}

			value_destroy(v);

			return ret;
		}
	}

	return NULL;
}

void * metacall_class(const char * name)
{
	value c_val = loader_get(name);

	klass c = NULL;
	if (value_type_id(c_val) == TYPE_CLASS)
	{
		c = value_to_class(c_val);
	}

	return c;
}

void * metacall_class_new(void * cls, const char * name, void * args[], size_t argc)
{
	object o = class_new(cls, name, args, argc);

	value v;

	if (o == NULL)
	{
		return NULL;
	}

	v = value_create_object(o);

	if (v == NULL)
	{
		object_destroy(o);
	}

	return v;
}

void * metacall_class_static_get(void * cls, const char * key)
{
	return class_static_get(cls, key);
}

int metacall_class_static_set(void * cls, const char * key, void * v)
{
	return class_static_set(cls, key, v);
}

void * metacallv_class(void * cls, const char * name, void * args[], size_t argc)
{
	return class_static_call(cls, name, args, argc);
}

void * metacallv_object(void * obj, const char * name, void * args[], size_t argc)
{
	return object_call(obj, name, args, argc);
}

void * metacall_object_get(void * obj, const char * key)
{
	return object_get(obj, key);
}

int metacall_object_set(void * obj, const char * key, void * v)
{
	return object_set(obj, key, v);
}

char * metacall_inspect(size_t * size, void * allocator)
{
	serial s;

	value v = loader_metadata();

	char * str;

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

char * metacall_serialize(const char * name, void * v, size_t * size, void * allocator)
{
	serial s = serial_create(name);

	return serial_serialize(s, (value)v, size, (memory_allocator)allocator);
}

void * metacall_deserialize(const char * name, const char * buffer, size_t size, void * allocator)
{
	serial s = serial_create(name);

	return (void *)serial_deserialize(s, buffer, size, (memory_allocator)allocator);
}

int metacall_clear(void * handle)
{
	return loader_clear(handle);
}

int metacall_destroy()
{
	if (loader_unload() != 0)
	{
		return 1;
	}

	configuration_destroy();

	metacall_initialize_flag = 1;

	return 0;
}

const char * metacall_print_info()
{
	static const char metacall_info[] =
		"MetaCall Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef METACALL_STATIC_DEFINE
			"Compiled as static library type"
		#else
			"Compiled as shared library type"
		#endif

		"\n";

	return metacall_info;
}
