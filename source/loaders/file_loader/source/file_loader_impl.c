/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading file code at run-time into a process.
 *
 */

#include <file_loader/file_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <log/log.h>

#include <stdlib.h>

typedef struct loader_impl_file_type
{
	void * impl_file_data;

} * loader_impl_file;

typedef struct loader_impl_file_handle_type
{
	void * handle_file_data;

} * loader_impl_file_handle;

typedef struct loader_impl_file_function_type
{
	loader_impl_file_handle file_handle;
	void * function_file_data;

} * loader_impl_file_function;

int function_file_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

function_return function_file_interface_invoke(function func, function_impl impl, function_args args)
{
	loader_impl_file_function file_function = (loader_impl_file_function)impl;

	signature s = function_signature(func);

	const char * name = function_name(func);

	type ret_type = signature_get_return(s);

	const size_t args_size = signature_count(s);

	(void)file_function;

	log_write("metacall", LOG_LEVEL_DEBUG, "Invoking file function %s", name);

	if (args_size > 0)
	{
		size_t args_count;

		for (args_count = 0; args_count < args_size; ++args_count)
		{
			type t = signature_get_type(s, args_count);

			type_id id = type_index(t);

			log_write("metacall", LOG_LEVEL_DEBUG, "Type %p, %d", (void *)t, id);

			if (id == TYPE_BOOL)
			{
				boolean * value_ptr = (boolean *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Boolean value: %d", *value_ptr);
			}
			else if (id == TYPE_CHAR)
			{
				char * value_ptr = (char *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Char value: %c", *value_ptr);
			}
			else if (id == TYPE_SHORT)
			{
				short * value_ptr = (short *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Short value: %d", *value_ptr);
			}
			else if (id == TYPE_INT)
			{
				int * value_ptr = (int *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Int value: %d", *value_ptr);
			}
			else if (id == TYPE_LONG)
			{
				long * value_ptr = (long *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Long value: %ld", *value_ptr);
			}
			else if (id == TYPE_FLOAT)
			{
				float * value_ptr = (float *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Float value: %f", *value_ptr);
			}
			else if (id == TYPE_DOUBLE)
			{
				double * value_ptr = (double *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Double value: %f", *value_ptr);
			}
			else if (id == TYPE_STRING)
			{
				const char * value_ptr = (const char *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "String value: %s", value_ptr);
			}
			else if (id == TYPE_PTR)
			{
				void * value_ptr = (void *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Pointer value: %p", value_ptr);
			}
			else
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Unrecognized value: %p", args[args_count]);
			}
		}

		log_write("metacall", LOG_LEVEL_DEBUG, "Calling file function with arguments (%lu)", args_size);
	}
	else
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Calling file function without arguments");
	}

	if (ret_type != NULL)
	{
		type_id id = type_index(ret_type);

		log_write("metacall", LOG_LEVEL_DEBUG, "Return type %p, %d", (void *)ret_type, id);

		if (id == TYPE_BOOL)
		{
			boolean b = 1;

			return value_create_bool(b);
		}
		else if (id == TYPE_CHAR)
		{
			return value_create_char('A');
		}
		else if (id == TYPE_SHORT)
		{
			return value_create_short(124);
		}
		else if (id == TYPE_INT)
		{
			return value_create_int(1234);
		}
		else if (id == TYPE_LONG)
		{
			return value_create_long(90000L);
		}
		else if (id == TYPE_FLOAT)
		{
			return value_create_float(0.2f);
		}
		else if (id == TYPE_DOUBLE)
		{
			return value_create_double(3.1416);
		}
		else if (id == TYPE_STRING)
		{
			static const char str[] = "Hello World";

			return value_create_string(str, sizeof(str) / sizeof(str[0]));
		}
		else if (id == TYPE_PTR)
		{
			static int int_val = 15;

			return value_create_ptr(&int_val);
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Unrecognized return type");
		}
	}

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
	static struct function_interface_type file_interface =
	{
		&function_file_interface_create,
		&function_file_interface_invoke,
		&function_file_interface_destroy
	};

	return &file_interface;
}

int file_loader_impl_initialize_types(loader_impl impl)
{
	static struct
	{
		type_id id;
		const char * name;
	}
	type_id_name_pair[] =
	{
		{ TYPE_SHORT,	"Short"		},
		{ TYPE_INT,		"Integer"	},
		{ TYPE_LONG,	"Long"		},
		{ TYPE_BUFFER,	"Buffer"	},
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

loader_impl_data file_loader_impl_initialize(loader_impl impl, configuration config, loader_host host)
{
	loader_impl_file file_impl;

	(void)impl;
	(void)config;

	log_copy(host->log);

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

	file_impl->impl_file_data = NULL;

	return file_impl;
}

int file_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	(void)impl;
	(void)path;

	return 0;
}

loader_handle file_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	loader_impl_file_handle handle = malloc(sizeof(struct loader_impl_file_handle_type));

	(void)impl;

	if (handle != NULL)
	{
		size_t iterator;

		for (iterator = 0; iterator < size; ++iterator)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "File module %s loaded from file", paths[iterator]);
		}

		handle->handle_file_data = NULL;

		return (loader_handle)handle;
	}

	return NULL;
}

loader_handle file_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size)
{
	loader_impl_file_handle handle = malloc(sizeof(struct loader_impl_file_handle_type));

	(void)impl;
	(void)buffer;
	(void)size;

	if (handle != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "File module %s. loaded from memory", name);

		handle->handle_file_data = NULL;

		return (loader_handle)handle;
	}

	return NULL;
}

loader_handle file_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	loader_impl_file_handle handle = malloc(sizeof(struct loader_impl_file_handle_type));

	(void)impl;

	if (handle != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "File module %s loaded from package", path);

		handle->handle_file_data = NULL;

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
		free(file_handle);

		return 0;
	}

	return 1;
}

loader_impl_file_function file_function_create(loader_impl_file_handle file_handle)
{
	loader_impl_file_function file_function = malloc(sizeof(struct loader_impl_file_function_type));

	if (file_function != NULL)
	{
		file_function->file_handle = file_handle;

		file_function->function_file_data = NULL;

		return file_function;
	}

	return NULL;
}

int file_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_file file_impl = loader_impl_get(impl);

	loader_impl_file_handle file_handle = (loader_impl_file_handle)handle;

	loader_impl_file_function file_function = file_function_create(file_handle);

	scope sp = context_scope(ctx);

	(void)file_impl;

	log_write("metacall", LOG_LEVEL_DEBUG, "File module %p discovering", handle);

	if (file_function != NULL)
	{
		function f = function_create("two_doubles", 2, file_function, &function_file_singleton);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "Long"));

		signature_set(s, 0, "first_parameter", loader_impl_type(impl, "Long"));

		signature_set(s, 1, "second_parameter", loader_impl_type(impl, "Long"));

		scope_define(sp, function_name(f), f);
	}

	return 0;
}

int file_loader_impl_destroy(loader_impl impl)
{
	loader_impl_file file_impl = loader_impl_get(impl);

	if (file_impl != NULL)
	{
		free(file_impl);

		return 0;
	}

	return 1;
}
