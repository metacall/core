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
	std::map<std::string, const void *> symbols;

} * loader_impl_c_handle;

typedef struct loader_impl_c_function_type
{
	const void *address;

} * loader_impl_c_function;

// TODO: Remove this
////////////////////////////////////////////////////////////////////////////////////////
std::string Convert(const CXString &s)
{
	std::string result = clang_getCString(s);
	clang_disposeString(s);
	return result;
}
void print_function_prototype(CXCursor cursor)
{
	// TODO : Print data!
	auto type = clang_getCursorType(cursor);

	auto function_name = Convert(clang_getCursorSpelling(cursor));
	auto return_type = Convert(clang_getTypeSpelling(clang_getResultType(type)));

	int num_args = clang_Cursor_getNumArguments(cursor);
	for (int i = 0; i < num_args; ++i)
	{
		auto arg_cursor = clang_Cursor_getArgument(cursor, i);
		auto arg_name = Convert(clang_getCursorSpelling(arg_cursor));
		if (arg_name.empty())
		{
			arg_name = "no name!";
		}

		auto arg_data_type = Convert(clang_getTypeSpelling(clang_getArgType(type, i)));
	}
}
CXChildVisitResult functionVisitor(CXCursor cursor, CXCursor /* parent */, CXClientData /* clientData */)
{
	if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
		return CXChildVisit_Continue;

	CXCursorKind kind = clang_getCursorKind(cursor);
	if ((kind == CXCursorKind::CXCursor_FunctionDecl || kind == CXCursorKind::CXCursor_CXXMethod || kind == CXCursorKind::CXCursor_FunctionTemplate ||
			kind == CXCursorKind::CXCursor_Constructor))
	{
		print_function_prototype(cursor);
	}

	return CXChildVisit_Continue;
}
////////////////////////////////////////////////////////////////////////////////////////

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

function_return function_c_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;

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

	(void)impl;
	(void)ctx;

	if (c_handle != NULL)
	{
		/* List all symbol addresses */
		tcc_list_symbols(c_handle->state, static_cast<void *>(c_handle), &c_loader_impl_discover_symbols);

		/* TODO: Iterate through the AST and obtain function declarations (and structs later on) */
		/* Then, register them into MetaCall associating them to the addresses */
		return 0;
	}

	return 1;
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
