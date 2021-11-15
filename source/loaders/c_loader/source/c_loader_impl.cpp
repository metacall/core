/*
 *	Loader Library by Parra Studios
 *	A plugin for loading c code at run-time into a process.
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

#include <c_loader/c_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <log/log.h>

#include <iterator>
#include <map>
#include <new>
#include <string>
#include <vector>

#include <cstring>

/* LibFFI */
#include <ffi.h>

/* LibTCC */
#include <libtcc.h>

/* LibClang */
#include <clang-c/CXString.h>
#include <clang-c/Index.h>

typedef struct loader_impl_c_type
{
	std::vector<std::string> execution_paths;

} * loader_impl_c;

typedef struct loader_impl_c_handle_type
{
	TCCState *state;
	std::vector<std::string> files;
	std::map<std::string, const void *> symbols;

} * loader_impl_c_handle;

typedef struct loader_impl_c_function_type
{
	ffi_cif cif;
	ffi_type *ret_type;
	ffi_type **arg_types;
	const void *address;

} * loader_impl_c_function;

typedef struct c_loader_impl_discover_visitor_data_type
{
	loader_impl impl;
	loader_impl_c_handle c_handle;
	scope sp;

} * c_loader_impl_discover_visitor_data;

static loader_impl_c_handle c_loader_impl_handle_create(loader_impl_c c_impl)
{
	loader_impl_c_handle c_handle = new loader_impl_c_handle_type();

	if (c_handle == nullptr)
	{
		return nullptr;
	}

	c_handle->state = tcc_new();

	if (c_handle->state == NULL)
	{
		delete c_handle;
		return nullptr;
	}

	tcc_set_output_type(c_handle->state, TCC_OUTPUT_MEMORY);

	/* TODO */
	/*
	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		tcc_enable_debug(c_handle->state);
	#endif
	*/

	/* TODO: Add error handling */
	/* tcc_set_error_func */

	/* TODO: Add some warnings? */
	/* tcc_set_warning */

	/* TODO: Take the c_loader and add the execution paths for include (and library?) folders */
	/* tcc_add_include_path, tcc_add_library_path */
	(void)c_impl;

	return c_handle;
}

static void c_loader_impl_handle_destroy(loader_impl_c_handle c_handle)
{
	tcc_delete(c_handle->state);
	delete c_handle;
}

static bool c_loader_impl_file_exists(const std::string &filename)
{
	if (FILE *file = fopen(filename.c_str(), "r"))
	{
		fclose(file);
		return true;
	}

	return false;
}

int function_c_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

ffi_type *get_ffi_type(type_id id)
{
	switch (id)
	{
		case TYPE_CHAR:
			return &ffi_type_schar;
			break;
		case TYPE_INT:
			return &ffi_type_sint;
			break;
		case TYPE_FLOAT:
			return &ffi_type_float;
			break;
		case TYPE_DOUBLE:
			return &ffi_type_double;
			break;

			/* TODO: Add more*/
	}

	return &ffi_type_void;
}
function_return function_c_interface_invoke(function func, function_impl impl, function_args args, size_t args_size)
{
	loader_impl_c_function c_function = (loader_impl_c_function)(impl);

	signature s = function_signature(func);

	type function_type = signature_get_return(s);
	type_id ret_id = type_index(function_type);
	c_function->ret_type = get_ffi_type(ret_id);

	const size_t signature_args_size = signature_count(s);

	for (size_t args_count = 0; args_count < args_size; args_count++)
	{
		type t = args_count < signature_args_size ? signature_get_type(s, args_count) : NULL;
		type_id id = t == NULL ? value_type_id((value)args[args_count]) : type_index(t);
		c_function->arg_types[args_count] = get_ffi_type(id);
	}

	if (ffi_prep_cif(&c_function->cif, FFI_DEFAULT_ABI, args_size, c_function->ret_type, c_function->arg_types) == FFI_OK)
	{
		ffi_call(&c_function->cif, FFI_FN(c_function->address), c_function->ret_type, args);
	}
	else
	{
		log_write("metacall", LOG_LEVEL_ERROR, "FFI call interface prep failed");
		return NULL;
	}
	return NULL;
}

function_return function_c_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
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
	static struct function_interface_type c_interface = {
		&function_c_interface_create,
		&function_c_interface_invoke,
		&function_c_interface_await,
		&function_c_interface_destroy
	};

	return &c_interface;
}

int c_loader_impl_initialize_types(loader_impl impl)
{
	static struct
	{
		type_id id;
		const char *name;
	} type_id_name_pair[] = {
		{ TYPE_BOOL, "bool" },
		{ TYPE_CHAR, "char" },
		{ TYPE_SHORT, "short" },
		{ TYPE_INT, "int" },
		{ TYPE_LONG, "long" },
		{ TYPE_FLOAT, "float" },
		{ TYPE_DOUBLE, "double" },

		// TODO: Do more types (and the unsigned versions too?)
	};

	size_t size = sizeof(type_id_name_pair) / sizeof(type_id_name_pair[0]);

	for (size_t index = 0; index < size; ++index)
	{
		type t = type_create(type_id_name_pair[index].id, type_id_name_pair[index].name, NULL, NULL);

		if (t != NULL)
		{
			if (loader_impl_type_define(impl, type_name(t), t) != 0)
			{
				return 1;
			}
		}
	}

	return 0;
}

loader_impl_data c_loader_impl_initialize(loader_impl impl, configuration config)
{
	loader_impl_c c_impl;

	(void)impl;
	(void)config;

	c_impl = new loader_impl_c_type();

	if (c_impl == nullptr)
	{
		return NULL;
	}

	if (c_loader_impl_initialize_types(impl) != 0)
	{
		delete c_impl;
		return NULL;
	}

	/* Register initialization */
	loader_initialization_register(impl);

	return static_cast<loader_impl_data>(c_impl);
}

int c_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	loader_impl_c c_impl = static_cast<loader_impl_c>(loader_impl_get(impl));

	c_impl->execution_paths.push_back(path);

	return 0;
}

static std::string c_loader_impl_cxstring_to_str(const CXString &s)
{
	std::string result = clang_getCString(s);
	clang_disposeString(s);
	return result;
}

static void c_loader_impl_discover_signature(loader_impl impl, loader_impl_c_handle c_handle, scope sp, CXCursor cursor)
{
	auto cursor_type = clang_getCursorType(cursor);
	auto func_name = c_loader_impl_cxstring_to_str(clang_getCursorSpelling(cursor));
	auto return_type = c_loader_impl_cxstring_to_str(clang_getTypeSpelling(clang_getResultType(cursor_type)));

	if (c_handle->symbols.count(func_name) == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Symbol '%s' not found, skipping the function", func_name.c_str());
		return;
	}

	loader_impl_c_function c_function = new loader_impl_c_function_type();

	c_function->address = c_handle->symbols[func_name];
	//get_ffi_type(return_type, c_function->ret_type);

	int num_args = clang_Cursor_getNumArguments(cursor);
	size_t args_count = num_args < 0 ? (size_t)0 : (size_t)num_args;

	function f = function_create(func_name.c_str(), args_count, c_function, &function_c_singleton);
	signature s = function_signature(f);

	signature_set_return(s, loader_impl_type(impl, return_type.c_str()));

	c_function->arg_types = new ffi_type *[args_count];

	for (size_t arg = 0; arg < args_count; ++arg)
	{
		auto arg_cursor = clang_Cursor_getArgument(cursor, arg);
		auto arg_name = c_loader_impl_cxstring_to_str(clang_getCursorSpelling(arg_cursor));
		auto arg_type = c_loader_impl_cxstring_to_str(clang_getTypeSpelling(clang_getArgType(cursor_type, arg)));

		signature_set(s, arg, arg_name.c_str(), loader_impl_type(impl, arg_type.c_str()));
	}

	scope_define(sp, function_name(f), value_create_function(f));
}

static CXChildVisitResult c_loader_impl_discover_visitor(CXCursor cursor, CXCursor, void *data)
{
	c_loader_impl_discover_visitor_data visitor_data = static_cast<c_loader_impl_discover_visitor_data>(data);

	if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
		return CXChildVisit_Continue;

	CXCursorKind kind = clang_getCursorKind(cursor);

	if (kind == CXCursorKind::CXCursor_FunctionDecl)
	{
		c_loader_impl_discover_signature(visitor_data->impl, visitor_data->c_handle, visitor_data->sp, cursor);
	}

	return CXChildVisit_Continue;
}

static int c_loader_impl_discover_ast(loader_impl impl, loader_impl_c_handle c_handle, context ctx)
{
	c_loader_impl_discover_visitor_data_type data = {
		impl,
		c_handle,
		context_scope(ctx)
	};

	for (std::string file : c_handle->files)
	{
		CXIndex index = clang_createIndex(0, 0);
		CXTranslationUnit unit = clang_parseTranslationUnit(
			index,
			file.c_str(), nullptr, 0,
			nullptr, 0,
			CXTranslationUnit_None);

		if (unit == nullptr)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Unable to parse translation unit of: %s", file.c_str());
			return -1;
		}

		CXCursor cursor = clang_getTranslationUnitCursor(unit);
		clang_visitChildren(cursor, c_loader_impl_discover_visitor, static_cast<void *>(&data));

		clang_disposeTranslationUnit(unit);
		clang_disposeIndex(index);
	}

	return 0;
}

loader_handle c_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	loader_impl_c c_impl = static_cast<loader_impl_c>(loader_impl_get(impl));
	loader_impl_c_handle c_handle = c_loader_impl_handle_create(c_impl);

	if (c_handle == nullptr)
	{
		return NULL;
	}

	/* TODO: Load the files (.c and .h) with the parser while iterating after loading them with TCC */

	for (size_t iterator = 0; iterator < size; ++iterator)
	{
		/* We assume it is a path so we load from path */
		if (loader_path_is_absolute(paths[iterator]) == 0)
		{
			if (tcc_add_file(c_handle->state, paths[iterator]) == -1)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Failed to load file: %s", paths[iterator]);
				c_loader_impl_handle_destroy(c_handle);
				return NULL;
			}

			std::string filename(paths[iterator]);
			c_handle->files.push_back(filename);
		}
		else
		{
			bool found = false;

			/* Otherwise, check the execution paths */
			for (auto exec_path : c_impl->execution_paths)
			{
				loader_naming_path path;
				size_t path_size = loader_path_join(exec_path.c_str(), exec_path.length() + 1, paths[iterator], strlen(paths[iterator]), path);
				std::string filename(path, path_size);

				if (c_loader_impl_file_exists(filename) == true && tcc_add_file(c_handle->state, path) != -1)
				{
					c_handle->files.push_back(filename);
					found = true;
					break;
				}
			}

			if (found == false)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Failed to load file: %s", paths[iterator]);
				c_loader_impl_handle_destroy(c_handle);
				return NULL;
			}
		}
	}

	if (tcc_relocate(c_handle->state, TCC_RELOCATE_AUTO) == -1)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "TCC failed to relocate");
		c_loader_impl_handle_destroy(c_handle);
		return NULL;
	}

	return c_handle;
}

loader_handle c_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char *buffer, size_t size)
{
	loader_impl_c c_impl = static_cast<loader_impl_c>(loader_impl_get(impl));
	loader_impl_c_handle c_handle = c_loader_impl_handle_create(c_impl);

	/* Apparently TCC has an unsafe API for compiling strings */
	(void)size;

	if (c_handle == nullptr)
	{
		return NULL;
	}

	if (tcc_compile_string(c_handle->state, buffer) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to compile the buffer: %s", name);
		c_loader_impl_handle_destroy(c_handle);
		return NULL;
	}

	if (tcc_relocate(c_handle->state, TCC_RELOCATE_AUTO) == -1)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "TCC failed to relocate");
		c_loader_impl_handle_destroy(c_handle);
		return NULL;
	}

	/* TODO: Load the buffer with the parser while iterating after loading it with TCC */

	return c_handle;
}

loader_handle c_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	/* TODO: Define what to do with this */
	/*
	loader_impl_c_handle c_handle = c_loader_impl_handle_create();

	if (c_handle == nullptr)
	{
		return NULL;
	}

	(void)impl;

	return c_handle;
	*/

	(void)impl;
	(void)path;

	return NULL;
}

int c_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_c_handle c_handle = static_cast<loader_impl_c_handle>(handle);

	(void)impl;

	if (c_handle != NULL)
	{
		c_loader_impl_handle_destroy(c_handle);

		return 0;
	}

	return 1;
}

static void c_loader_impl_discover_symbols(void *ctx, const char *name, const void *addr)
{
	loader_impl_c_handle c_handle = static_cast<loader_impl_c_handle>(ctx);

	c_handle->symbols.insert(std::pair<std::string, const void *>(name, addr));
}

int c_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_c_handle c_handle = static_cast<loader_impl_c_handle>(handle);

	/* Get all symbols */
	tcc_list_symbols(c_handle->state, static_cast<void *>(c_handle), &c_loader_impl_discover_symbols);

	/* Parse the AST and register functions */
	return c_loader_impl_discover_ast(impl, c_handle, ctx);
}

int c_loader_impl_destroy(loader_impl impl)
{
	loader_impl_c c_impl = static_cast<loader_impl_c>(loader_impl_get(impl));

	if (c_impl != NULL)
	{
		/* Destroy children loaders */
		loader_unload_children(impl, 0);

		delete c_impl;

		return 0;
	}

	return 1;
}
