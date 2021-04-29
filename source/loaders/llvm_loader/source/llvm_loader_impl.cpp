/*
 *	Loader Library by Parra Studios
 *	A plugin for loading LLVM code at run-time into a process.
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

#include <llvm_loader/llvm_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <log/log.h>

// LLVM
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>

// Optimizations
#include <llvm/Analysis/BasicAliasAnalysis.h>
#include <llvm/Transforms/Scalar.h>

typedef struct loader_impl_llvm_function_type
{
	// TODO: The reference to LLVM function must be stored here
	void *todo;

} * loader_impl_llvm_function;

typedef struct loader_impl_llvm_handle_type
{
	// TODO: The references to LLVM modules must be stored here
	// This should contain a vector with all LLVM modules loaded (input.ll)
	void *todo;

} * loader_impl_llvm_handle;

typedef struct loader_impl_llvm_type
{
	// TODO: The reference to LLVM interpreter must be stored here
	void *todo;

} * loader_impl_llvm;

int type_llvm_interface_create(type t, type_impl impl)
{
	/* TODO */

	(void)t;
	(void)impl;

	return 0;
}

void type_llvm_interface_destroy(type t, type_impl impl)
{
	/* TODO */

	(void)t;
	(void)impl;
}

type_interface type_llvm_singleton(void)
{
	static struct type_interface_type llvm_type_interface = {
		&type_llvm_interface_create,
		&type_llvm_interface_destroy
	};

	return &llvm_type_interface;
}

int function_llvm_interface_create(function func, function_impl impl)
{
	/* TODO */

	(void)func;
	(void)impl;

	return 0;
}

function_return function_llvm_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;

	return NULL;
}

function_return function_llvm_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
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

void function_llvm_interface_destroy(function func, function_impl impl)
{
	/* TODO */

	(void)func;
	(void)impl;
}

function_interface function_llvm_singleton(void)
{
	static struct function_interface_type llvm_function_interface = {
		&function_llvm_interface_create,
		&function_llvm_interface_invoke,
		&function_llvm_interface_await,
		&function_llvm_interface_destroy
	};

	return &llvm_function_interface;
}

int llvm_loader_impl_register_types(loader_impl impl)
{
	static struct
	{
		type_id id;
		const char *name;
	} type_id_name_pair[] = {
		{ TYPE_INT, "i32" },
		{ TYPE_FLOAT, "float" }
		// TODO: Implement the rest of the types
	};

	for (auto &pair : type_id_name_pair)
	{
		// TODO: Do we need to pass the builtin?
		type builtin_type = type_create(pair.id, pair.name, /* builtin */ NULL, &type_llvm_singleton);

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

loader_impl_data llvm_loader_impl_initialize(loader_impl impl, configuration config)
{
	loader_impl_llvm llvm_impl = new loader_impl_llvm_type();

	if (llvm_impl == nullptr)
	{
		// TODO: Emit exception when exception handling is implemented
		return NULL;
	}

	/* TODO */
	// Initialize here all the fields of llvm_impl if any is required,
	// it is possible to initialize it in the constructor of the struct loader_impl_llvm_type,
	// or class if you refactor it

	/* Register the types */
	if (llvm_loader_impl_register_types(impl) != 0)
	{
		// TODO: Emit exception when exception handling is implemented
		delete llvm_impl;
		return NULL;
	}

	// TODO: Handle something with the configuration?
	(void)config;

	/* Register initialization */
	loader_initialization_register(impl);

	return llvm_impl;
}

int llvm_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	loader_impl_llvm llvm_impl = static_cast<loader_impl_llvm>(loader_impl_get(impl));
	loader_impl_llvm_handle llvm_handle = new loader_impl_llvm_handle_type();

	if (llvm_handle == nullptr)
	{
		// TODO: Emit exception when exception handling is implemented
		return NULL;
	}

	/* TODO */

	(void)llvm_impl;
	(void)path;

	return 0;
}

loader_handle llvm_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	loader_impl_llvm llvm_impl = static_cast<loader_impl_llvm>(loader_impl_get(impl));
	loader_impl_llvm_handle llvm_handle = new loader_impl_llvm_handle_type();

	if (llvm_handle == nullptr)
	{
		// TODO: Emit exception when exception handling is implemented
		return NULL;
	}

	/* TODO */
	// Load here the list of paths using modules to store them into loader_impl_llvm_handle_type

	(void)llvm_impl;
	(void)paths;
	(void)size;

	// TODO: Return here the pointer to loader_impl_llvm_handle_type
	return static_cast<loader_handle>(llvm_handle);
}

loader_handle llvm_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char *buffer, size_t size)
{
	loader_impl_llvm llvm_impl = static_cast<loader_impl_llvm>(loader_impl_get(impl));
	loader_impl_llvm_handle llvm_handle = new loader_impl_llvm_handle_type();

	if (llvm_handle == nullptr)
	{
		// TODO: Emit exception when exception handling is implemented
		return NULL;
	}

	/* TODO */
	// The same as previous module but you have to load from the string buffer instead of from file

	(void)llvm_impl;
	(void)name;
	(void)buffer;
	(void)size;

	// TODO: Return here the pointer to loader_impl_llvm_handle_type
	return static_cast<loader_handle>(llvm_handle);
}

loader_handle llvm_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	loader_impl_llvm llvm_impl = static_cast<loader_impl_llvm>(loader_impl_get(impl));
	loader_impl_llvm_handle llvm_handle = new loader_impl_llvm_handle_type();

	if (llvm_handle == nullptr)
	{
		// TODO: Emit exception when exception handling is implemented
		return NULL;
	}

	/* TODO */
	// The same as load_from_file but this should load from binary format instead of readable format (input.ll)

	(void)impl;
	(void)path;

	// TODO: Return here the pointer to loader_impl_llvm_handle_type
	return static_cast<loader_handle>(llvm_handle);
}

int llvm_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_llvm llvm_impl = static_cast<loader_impl_llvm>(loader_impl_get(impl));
	loader_impl_llvm_handle llvm_handle = static_cast<loader_impl_llvm_handle>(handle);

	/* TODO */
	// Clear the array of modules from the llvm_handle and free all the memory related to it
	(void)llvm_impl;

	// Then delete the LLVM handle itself
	delete llvm_handle;

	return 0;
}

int llvm_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_llvm llvm_impl = static_cast<loader_impl_llvm>(loader_impl_get(impl));
	loader_impl_llvm_handle llvm_handle = static_cast<loader_impl_llvm_handle>(handle);

	/* TODO */
	// Iterate all the modules loaded by this llvm_handle,
	// introspect the functions and structs and store them in ctx

	(void)llvm_impl;
	(void)llvm_handle;
	(void)ctx;

	return 0;
}

int llvm_loader_impl_destroy(loader_impl impl)
{
	loader_impl_llvm llvm_impl = static_cast<loader_impl_llvm>(loader_impl_get(impl));

	/* Destroy children loaders */
	loader_unload_children(impl);

	/* TODO */
	// Clear here the resources, it must free all memory related to llvm_impl.
	// The destruction of the llvm_impl contents (the bytecode interpreter for example),
	// must be done always after loader_unload_children

	// Then delete the LLVM loader itself
	delete llvm_impl;

	return 0;
}
