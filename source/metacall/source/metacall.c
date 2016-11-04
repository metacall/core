/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
#include <reflect/reflect_function.h>

#include <string.h>

/* -- Definitions -- */

#define METACALL_ARGS_SIZE 0x10

/* -- Methods -- */

int metacall_initialize()
{
	size_t iterator;

	/* TODO: load a full path */
	loader_naming_name module_names[] =
	{
		#if defined(OPTION_BUILD_PLUGINS_C)
			"compiled.c",
		#endif /* OPTION_BUILD_PLUGINS_C */

		#if defined(OPTION_BUILD_PLUGINS_CS)
			"hello.cs",
		#endif /* OPTION_BUILD_PLUGINS_CS */

		#if defined(OPTION_BUILD_PLUGINS_JSM)
			"spider.jsm",
		#endif /* OPTION_BUILD_PLUGINS_JSM */

		#if defined(OPTION_BUILD_PLUGINS_JS)
			"divide.js",
		#endif /* OPTION_BUILD_PLUGINS_JS */

		#if defined(OPTION_BUILD_PLUGINS_MOCK)
			"empty.mock",
		#endif /* OPTION_BUILD_PLUGINS_MOCK */

		#if defined(OPTION_BUILD_PLUGINS_PY)
			"example.py",
		#endif /* OPTION_BUILD_PLUGINS_PY */

		#if defined(OPTION_BUILD_PLUGINS_RB)
			"hello.rb",
		#endif /* OPTION_BUILD_PLUGINS_RB */
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

const char * metacall_print_info()
{
	static const char metacall_info[] =
		"MetaCall Library " METACALL_VERSION "\n"
		"Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef METACALL_STATIC_DEFINE
		"Compiled as static library type"
#else
		"Compiled as shared library type"
#endif

		"\n";

	return metacall_info;
}
