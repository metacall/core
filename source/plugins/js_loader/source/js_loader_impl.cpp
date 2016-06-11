/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading javascript code at run-time into a process.
 *
 */

#include <js_loader/js_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/type.h>
#include <reflect/function.h>
#include <reflect/scope.h>
#include <reflect/context.h>

#include <adt/hash_map.h>
#include <adt/hash_map_str.h>

#include <new>
#include <iostream>

/*
#include <v8.h>
*/

using namespace std;

/*
using namespace v8;
*/

typedef struct loader_impl_js_type
{
	void * todo;

} * loader_impl_js;

typedef struct loader_impl_js_handle_type
{
	void * todo;

} * loader_impl_js_handle;

int function_js_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

void function_js_interface_invoke(function func, function_impl impl, function_args args)
{
	(void)func;
	(void)impl;
	(void)args;
}

void function_js_interface_destroy(function func, function_impl impl)
{
	(void)func;
	(void)impl;
}

function_interface function_js_singleton()
{
	static struct function_interface_type js_interface =
	{
		&function_js_interface_create,
		&function_js_interface_invoke,
		&function_js_interface_destroy
	};

	return &js_interface;
}

loader_impl_data js_loader_impl_initialize(loader_impl impl)
{
	loader_impl_js js_impl = new loader_impl_js_type();

	(void)impl;

	if (js_impl != nullptr)
	{
		return static_cast<loader_impl_data>(js_impl);
	}

	return NULL;
}

int js_loader_impl_execution_path(loader_impl impl, loader_naming_path path)
{
	(void)impl;
	(void)path;

	return 0;
}

loader_handle js_loader_impl_load(loader_impl impl, loader_naming_path path, loader_naming_name name)
{
	loader_impl_js_handle js_handle = new loader_impl_js_handle_type();

	(void)impl;
	(void)path;
	(void)name;

	if (js_handle != nullptr)
	{
		return static_cast<loader_handle>(js_handle);
	}

	return NULL;
}

int js_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_js_handle js_handle = static_cast<loader_impl_js_handle>(handle);

	(void)impl;

	if (js_handle != nullptr)
	{
		delete js_handle;

		return 0;
	}

	return 1;
}

int js_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	/* loader_impl_js_handle js_handle = (loader_impl_js_handle)handle; */

	(void)impl;
	(void)handle;
	(void)ctx;

	return 0;
}

int js_loader_impl_destroy(loader_impl impl)
{
	loader_impl_js js_impl = static_cast<loader_impl_js>(loader_impl_get(impl));

	if (js_impl != nullptr)
	{
		delete js_impl;

		return 0;
	}

	return 1;
}
