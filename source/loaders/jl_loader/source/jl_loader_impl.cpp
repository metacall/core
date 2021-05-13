/*
 *	Loader Library by Parra Studios
 *	A plugin for loading Julia code at run-time into a process.
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

#include <jl_loader/jl_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>
#include <loader/loader_path.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <log/log.h>

// TODO: Add Julia headers here
#include <julia.h>

typedef struct loader_impl_jl_function_type
{
	// TODO: The reference to Julia function must be stored here
	void *todo;

} * loader_impl_jl_function;

typedef struct loader_impl_jl_handle_type
{
	// TODO: The references to Julia scripts must be stored here
	// This should contain a vector with all Julia scripts loaded
	void *todo;

} * loader_impl_jl_handle;

typedef struct loader_impl_jl_type
{
	// TODO: The reference to Julia runtime instance must be stored here
	// This structure holds any data that must live between initialize and destroy
	void *todo;

} * loader_impl_jl;

int type_jl_interface_create(type t, type_impl impl)
{
	/* TODO */

	(void)t;
	(void)impl;

	return 0;
}

void type_jl_interface_destroy(type t, type_impl impl)
{
	/* TODO */

	(void)t;
	(void)impl;
}

type_interface type_jl_singleton(void)
{
	static struct type_interface_type jl_type_interface = {
		&type_jl_interface_create,
		&type_jl_interface_destroy
	};

	return &jl_type_interface;
}

int function_jl_interface_create(function func, function_impl impl)
{
	/* TODO */

	(void)func;
	(void)impl;

	return 0;
}

function_return function_jl_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;

	return NULL;
}

function_return function_jl_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
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

void function_jl_interface_destroy(function func, function_impl impl)
{
	/* TODO */

	(void)func;
	(void)impl;
}

function_interface function_jl_singleton(void)
{
	static struct function_interface_type jl_function_interface = {
		&function_jl_interface_create,
		&function_jl_interface_invoke,
		&function_jl_interface_await,
		&function_jl_interface_destroy
	};

	return &jl_function_interface;
}

int jl_loader_impl_register_types(loader_impl impl)
{
	// TODO: Implement the rest of the types
	// https://docs.julialang.org/en/v1/manual/types/
	static struct
	{
		type_id id;
		const char *name;
	} type_id_name_pair[] = {
		// TODO: Boolean
		// TODO: Char
		// TODO: We don't support unsigned / signed differenciation (yet?) 
		{ TYPE_CHAR, "Int8" },
		{ TYPE_CHAR, "UInt8" },
		{ TYPE_SHORT, "Int16" },
		{ TYPE_SHORT, "UInt16" },
		{ TYPE_INT, "Int32" },
		{ TYPE_INT, "UInt32" },
		{ TYPE_LONG, "Int64" },
		{ TYPE_LONG, "UInt64" },
		// TODO: We don't support 128 bits types (yet?) 
		// { TYPE_LONG_LONG, "Int128" },
		// { TYPE_LONG_LONG, "UInt128" },
		// TODO: We don't support 16 bits floating point (yet?)
		{ TYPE_FLOAT, "Float32" }
		{ TYPE_DOUBLE, "Float64" }
		// TODO: String

	};

	for (auto &pair : type_id_name_pair)
	{
		// TODO: Do we need to pass the builtin?
		type builtin_type = type_create(pair.id, pair.name, /* builtin */ NULL, &type_jl_singleton);

		if (builtin_type == NULL)
		{
			// TODO: Emit exception when exception handling is implemented
			return 1;
		}

		if (loader_impl_type_define(impl, type_name(builtin_type), builtin_type) != 0)
		{
			// TODO: Emit exception when exception handling is implemented
			return 1;
		}
	}

	return 0;
}

loader_impl_data jl_loader_impl_initialize(loader_impl impl, configuration config)
{
	loader_impl_jl jl_impl = new loader_impl_jl_type();

	if (jl_impl == nullptr)
	{
		// TODO: Emit exception when exception handling is implemented
		return NULL;
	}

	// TODO:
	// Initialize Julia runtime here

	/* Register the types */
	if (jl_loader_impl_register_types(impl) != 0)
	{
		// TODO: Emit exception when exception handling is implemented
		delete jl_impl;
		return NULL;
	}

	// TODO: Handle something with the configuration?
	(void)config;

	/* Register initialization */
	loader_initialization_register(impl);

	return jl_impl;
}

int jl_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	loader_impl_jl jl_impl = static_cast<loader_impl_jl>(loader_impl_get(impl));

	// TODO:
	// Add the path to the execution path list of Julia,
	// usually means the list of paths where Julia looks for a script or library

	(void)jl_impl;
	(void)path;

	return 0;
}

loader_handle jl_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	loader_impl_jl jl_impl = static_cast<loader_impl_jl>(loader_impl_get(impl));
	loader_impl_jl_handle jl_handle = new loader_impl_jl_handle_type();

	if (jl_handle == nullptr)
	{
		// TODO: Emit exception when exception handling is implemented
		return NULL;
	}

	// TODO: Load here the list of paths of files and then store them into loader_impl_jl_handle_type

	(void)jl_impl;
	(void)paths;
	(void)size;

	// TODO: Return here the pointer to loader_impl_jl_handle_type
	return static_cast<loader_handle>(jl_handle);
}

loader_handle jl_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char *buffer, size_t size)
{
	loader_impl_jl jl_impl = static_cast<loader_impl_jl>(loader_impl_get(impl));
	loader_impl_jl_handle jl_handle = new loader_impl_jl_handle_type();

	if (jl_handle == nullptr)
	{
		// TODO: Emit exception when exception handling is implemented
		return NULL;
	}

	// TODO: The same load_from_file but you have to load from the string buffer instead of from file
	// This is similar to an eval in JavaScript

	(void)jl_impl;
	(void)name;
	(void)buffer;
	(void)size;

	// TODO: Return here the pointer to loader_impl_jl_handle_type
	return static_cast<loader_handle>(jl_handle);
}

loader_handle jl_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	loader_impl_jl jl_impl = static_cast<loader_impl_jl>(loader_impl_get(impl));
	loader_impl_jl_handle jl_handle = new loader_impl_jl_handle_type();

	if (jl_handle == nullptr)
	{
		// TODO: Emit exception when exception handling is implemented
		return NULL;
	}

	// TODO: The same as load_from_file but this should load from binary format instead of readable format
	// Usually this is for loading DLLs or precompiled binaries, not sure if Julia supports this
	// If Julia does not support this feature, just leave it empty and return NULL

	(void)impl;
	(void)path;

	// TODO: Return here the pointer to loader_impl_jl_handle_type
	return static_cast<loader_handle>(jl_handle);
}

int jl_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_jl jl_impl = static_cast<loader_impl_jl>(loader_impl_get(impl));
	loader_impl_jl_handle jl_handle = static_cast<loader_impl_jl_handle>(handle);

	// TODO: Clear the array of modules from the jl_handle and free all the memory related to it
	(void)jl_impl;

	// Then delete the Julia handle itself
	delete jl_handle;

	return 0;
}

int jl_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_jl jl_impl = static_cast<loader_impl_jl>(loader_impl_get(impl));
	loader_impl_jl_handle jl_handle = static_cast<loader_impl_jl_handle>(handle);

	// TODO:
	// Iterate all the modules loaded by this jl_handle,
	// introspect the functions and structs and store them into ctx

	(void)jl_impl;
	(void)jl_handle;
	(void)ctx;

	return 0;
}

int jl_loader_impl_destroy(loader_impl impl)
{
	loader_impl_jl jl_impl = static_cast<loader_impl_jl>(loader_impl_get(impl));

	/* Destroy children loaders */
	loader_unload_children(impl);

	/* TODO */
	// Clear here the resources, it must free all memory related to jl_impl.
	// The destruction of the jl_impl contents must be done always after loader_unload_children

	// Then delete the Julia loader itself
	delete jl_impl;

	return 0;
}
