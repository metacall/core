/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading c code at run-time into a process.
 *
 */

#include <c_loader/c_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/type.h>
#include <reflect/function.h>
#include <reflect/scope.h>
#include <reflect/context.h>

#include <adt/hash_map.h>
#include <adt/hash_map_str.h>

#include <new>
#include <iostream>

#include <ffi.h>

#include <llvm/IR/Module.h>

#include <clang/AST/AST.h>

#define C_LOADER_FILE_MAX_SIZE	((size_t)(2 * 1024 * 1024)) /* 2 MB */

typedef struct loader_impl_c_type
{
	void * todo;

} * loader_impl_c;

typedef struct loader_impl_c_handle_type
{
/*
	char * buffer;
	size_t buffer_size;
*/
	void * todo;

} * loader_impl_c_handle;

typedef struct loader_impl_c_function_type
{
	void * todo;

} * loader_impl_c_function;

int function_c_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

void function_c_interface_invoke(function func, function_impl impl, function_args args)
{
	(void)func;
	(void)impl;
	(void)args;
}

void function_c_interface_destroy(function func, function_impl impl)
{
	loader_impl_c_function c_function = static_cast<loader_impl_c_function>(impl);

	(void)func;

	if (c_function != NULL)
	{
		delete c_function;
	}
}

function_interface function_c_singleton()
{
	static struct function_interface_type c_interface =
	{
		&function_c_interface_create,
		&function_c_interface_invoke,
		&function_c_interface_destroy
	};

	return &c_interface;
}

loader_impl_data c_loader_impl_initialize(loader_impl impl)
{
	loader_impl_c c_impl = new loader_impl_c_type();

	(void)impl;

	if (c_impl != nullptr)
	{
		return static_cast<loader_impl_data>(c_impl);
	}

	return NULL;
}

int c_loader_impl_execution_path(loader_impl impl, loader_naming_path path)
{
	(void)impl;
	(void)path;

	return 0;
}

loader_handle c_loader_impl_load(loader_impl impl, loader_naming_path path, loader_naming_name name)
{
	loader_impl_c_handle c_handle = new loader_impl_c_handle_type();

	(void)impl;

	if (c_handle != nullptr)
	{
		return static_cast<loader_handle>(c_handle);

		/*
		FILE * file;

		file = fopen(path, "r");

		if (file != NULL)
		{
			size_t size = 0;

			if (fseek(file, 0, SEEK_END) == 0)
			{
				long int tell_size = ftell(file);

				if (tell_size > 0)
				{
					size = (size_t)tell_size;
				}
			}

			if (size > 0 && size < C_LOADER_FILE_MAX_SIZE && fseek(file, 0, SEEK_SET) == 0)
			{
				char * buffer = malloc(sizeof(char) * size);

				if (buffer != NULL)
				{
					size_t result = fread(buffer, sizeof(char), size, file);

					if (result == size)
					{
						fclose(file);

						c_handle->buffer = buffer;
						c_handle->buffer_size = size;

						printf("C module <%s> correctly loaded\n", name);

						return (loader_handle)c_handle;
					}

					free(buffer);
				}
			}

			fclose(file);
		}

		free(c_handle);
		*/
	}

	return NULL;
}

int c_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_c_handle c_handle = static_cast<loader_impl_c_handle>(handle);

	(void)impl;

	if (c_handle != NULL)
	{
		/*
		if (c_handle->buffer != NULL)
		{
			free(c_handle->buffer);

			c_handle->buffer = NULL;
		}

		c_handle->buffer_size = 0;
		*/

		delete c_handle;

		return 0;
	}

	return 1;
}

int c_loader_impl_discover_func(loader_impl impl, loader_handle handle, context ctx, function f)
{
	(void)impl;
	(void)handle;
	(void)ctx;
	(void)f;

	return 0;
}

int c_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_c_handle c_handle = static_cast<loader_impl_c_handle>(handle);

	(void)impl;
	(void)ctx;

	if (c_handle != NULL)
	{


		return 0;
	}

	return 1;
}

int c_loader_impl_destroy(loader_impl impl)
{
	loader_impl_c c_impl = static_cast<loader_impl_c>(loader_impl_get(impl));

	if (c_impl != NULL)
	{
		delete c_impl;

		return 0;
	}

	return 1;
}
