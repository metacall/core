/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

 /* -- Headers -- */

#include <metacall/metacall-version.h>
#include <metacall/metacall-plugins.h>
#include <metacall/metacall.h>

#include <loader/loader.h>

#include <reflect/reflect_value_type.h>
#include <reflect/reflect_context.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_value_type_cast.h>
#include <reflect/reflect_function.h>

#include <configuration/configuration.h>

#include <log/log.h>

#include <serial/serial.h>

#include <string.h>

/* -- Definitions -- */

#define METACALL_ARGS_SIZE	0x10
#define METACALL_SERIAL		"rapid_json"

/* -- Global Variables -- */

void * metacall_null_args[1];

/* -- Private Variables -- */

static int metacall_initialize_flag = 1;

/* Private Methods */

static const char * metacall_serial(void);

/* -- Methods -- */

const char * metacall_serial()
{
	static const char metacall_serial_str[] = METACALL_SERIAL;

	return metacall_serial_str;
}

int metacall_initialize()
{
	loader l = loader_singleton();

	if (metacall_initialize_flag == 0)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall already initialized <%p>", (void *)l);

		return 0;
	}
	else
	{
		/* TODO: Initialize by config or default */
		#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
			if (log_configure("metacall",
				log_policy_format_text(),
				log_policy_schedule_sync(),
				log_policy_storage_sequential(),
				log_policy_stream_stdio(stdout)) != 0)
			{
				return 1;
			}
		#endif

		log_write("metacall", LOG_LEVEL_DEBUG, "Initializing MetaCall <%p>", (void *)l);
	}

	metacall_null_args[0] = NULL;

	if (configuration_initialize(metacall_serial(), NULL) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid MetaCall configuration initialization");

		return 1;
	}

	#ifndef LOADER_LAZY
		loader_initialize();
	#endif

	metacall_initialize_flag = 0;

	return 0;
}

size_t metacall_args_size()
{
	const size_t args_size = METACALL_ARGS_SIZE;

	return args_size;
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

int metacall_load_from_configuration(const char * path, void ** handle)
{
	return loader_load_from_configuration(path, handle);
}

void * metacallv(const char * name, void * args[])
{
	return metacallfv(loader_get(name), args);
}

void * metacall(const char * name, ...)
{
	function f = loader_get(name);

	if (f != NULL)
	{
		void * args[METACALL_ARGS_SIZE];

		value ret = NULL;

		signature s = function_signature(f);

		size_t iterator;

		va_list va;

		va_start(va, name);

		for (iterator = 0; iterator < signature_count(s); ++iterator)
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

		ret = function_call(f, args);

		for (iterator = 0; iterator < signature_count(s); ++iterator)
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
	function f = loader_get(name);

	if (f != NULL)
	{
		void * args[METACALL_ARGS_SIZE];

		value ret = NULL;

		signature s = function_signature(f);

		size_t iterator;

		va_list va;

		va_start(va, ids);

		for (iterator = 0; iterator < signature_count(s); ++iterator)
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

		ret = function_call(f, args);

		for (iterator = 0; iterator < signature_count(s); ++iterator)
		{
			value_destroy(args[iterator]);
		}

		return ret;
	}

	return NULL;
}

void * metacall_function(const char * name)
{
	return (void *)loader_get(name);
}

void * metacall_handle(const char * tag, const char * name)
{
	return (void *)loader_get_handle(tag, name);
}

const char * metacall_handle_id(void * handle)
{
	return loader_handle_id(handle);
}

void * metacallfv(void * func, void * args[])
{
	function f = (function)func;

	if (f != NULL)
	{
		signature s = function_signature(f);

		size_t iterator;

		value ret;

		for (iterator = 0; iterator < signature_count(s); ++iterator)
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

		ret = function_call(f, args);

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

		size_t iterator;

		va_list va;

		va_start(va, func);

		for (iterator = 0; iterator < signature_count(s); ++iterator)
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

		ret = function_call(f, args);

		for (iterator = 0; iterator < signature_count(s); ++iterator)
		{
			value_destroy(args[iterator]);
		}

		return ret;
	}

	return NULL;
}

int metacall_register(const char * name, void * (*invoke)(void * []), enum metacall_value_id return_type, size_t size, ...)
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

	return loader_register(name, (loader_register_invoke)invoke, (type_id)return_type, size, (type_id *)types);
}

char * metacall_inspect(size_t * size)
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

	str = serial_serialize(s, v, size);

	value_type_destroy(v);

	return str;
}


char * metacall_serialize(void * v, size_t * size)
{
	serial s = serial_create(metacall_serial());

	return serial_serialize(s, (value)v, size);
}

void * metacall_deserialize(const char * buffer, size_t size)
{
	serial s = serial_create(metacall_serial());

	return (void *)serial_deserialize(s, buffer, size);
}

int metacall_clear(void * handle)
{
	return loader_clear(handle);
}

int metacall_destroy()
{
	configuration_destroy();

	if (loader_unload() != 0)
	{
		return 1;
	}

	metacall_initialize_flag = 1;

	return 0;
}

const char * metacall_print_info()
{
	static const char metacall_info[] =
		"MetaCall Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef METACALL_STATIC_DEFINE
			"Compiled as static library type"
		#else
			"Compiled as shared library type"
		#endif

		"\n";

	return metacall_info;
}
