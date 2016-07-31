/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

/* -- Headers -- */

#include <metacall/metacall-version.h>
#include <metacall/metacall.h>

#include <loader/loader.h>

#include <reflect/value.h>
#include <reflect/function.h>

#include <string.h>
#include <stdio.h>

/* -- Definitions -- */

#define METACALL_ARGS_SIZE 0x10

/* -- Methods -- */

int metacall_initialize()
{
	size_t iterator;

	/* TODO: load a full path */
	loader_naming_name module_names[] =
	{
		/*"compiled.c", "spider.jsm",*/
		"divide.js"/*,*/
		/*"example.py",
		"hello.rb",*/
		/*"empty.mock"*/
	};

	for (iterator = 0; iterator < sizeof(module_names) / sizeof(module_names[0]); ++iterator)
	{
		if (loader_load(module_names[iterator]) != 0)
		{
			return 1;
		}
	}

	return 0;
}

size_t metacall_args_size()
{
	const size_t args_size = METACALL_ARGS_SIZE;

	return args_size;
}

int metacall_load(const char * path)
{
	return loader_load(path);
}

value metacallv(const char * name, void * args[])
{
	function f = (function)loader_get(name);

	if (f != NULL)
	{
		return function_call(f, args);
	}

	return NULL;
}

value metacall(const char * name, ...)
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
				args[iterator] = value_create_bool(va_arg(va, boolean));
			}
			if (id == TYPE_CHAR)
			{
				args[iterator] = value_create_char((char)va_arg(va, int));
			}
			else if (id == TYPE_INT)
			{
				args[iterator] = value_create_int(va_arg(va, int));
			}
			else if (id == TYPE_LONG)
			{
				args[iterator] = value_create_long(va_arg(va, long));
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

int metacall_destroy()
{
	return loader_unload();
}

void metacall_print_info()
{
	printf("MetaCall Library " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n");

	#ifdef METACALL_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif
}
