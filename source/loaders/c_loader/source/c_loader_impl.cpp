/*
 *	Loader Library by Parra Studios
 *	A plugin for loading c code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <portability/portability_path.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>
#include <reflect/reflect_value_type_id_size.h>

#include <dynlink/dynlink.h>

#include <log/log.h>

#include <metacall/metacall.h>

#if defined __has_include
	#if __has_include(<filesystem>)
		#include <filesystem>
namespace fs = std::filesystem;
	#elif __has_include(<experimental/filesystem>)
		#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
	#else
		#error "Missing the <filesystem> header."
	#endif
#else
	#error "C++ standard too old for compiling this file."
#endif

#include <map>
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
	std::string libtcc_runtime_path;

} * loader_impl_c;

typedef struct loader_impl_c_handle_base_type
{
public:
	std::vector<std::string> files;

	virtual ~loader_impl_c_handle_base_type() {}

	virtual bool recursive_includes() = 0;

	virtual int discover(loader_impl impl, context ctx) = 0;

	virtual const void *symbol(std::string &name) = 0;

	void add(const loader_path path, size_t size)
	{
		if (this->is_ld_script(path, size) == false)
		{
			std::string filename(path, size);

			this->files.push_back(filename);
		}
	}

private:
	bool is_ld_script(const loader_path path, size_t size)
	{
		static const char extension[] = ".ld";

		if (size > sizeof(extension))
		{
			for (size_t ext_it = 0, path_it = size - sizeof(extension); path_it < size - 1; ++ext_it, ++path_it)
			{
				if (path[path_it] != extension[ext_it])
				{
					return false;
				}
			}
		}

		return true;
	}

} * loader_impl_c_handle_base;

static void c_loader_impl_discover_symbols(void *ctx, const char *name, const void *addr);
static int c_loader_impl_discover_ast(loader_impl impl, loader_impl_c_handle_base c_handle, context ctx);

typedef struct loader_impl_c_handle_tcc_type : loader_impl_c_handle_base_type
{
public:
	TCCState *state;
	std::map<std::string, const void *> symbols;

	loader_impl_c_handle_tcc_type() :
		state(NULL) {}

	virtual ~loader_impl_c_handle_tcc_type()
	{
		if (this->state != NULL)
		{
			tcc_delete(this->state);
		}
	}

	bool recursive_includes()
	{
		return false;
	}

	bool initialize(loader_impl_c c_impl)
	{
		this->state = tcc_new();

		if (this->state == NULL)
		{
			return false;
		}

		/* JIT the code into memory */
		tcc_set_output_type(this->state, TCC_OUTPUT_MEMORY);

		/* Register runtime path for TCC (in order to find libtcc1.a and runtime objects) */
		if (!c_impl->libtcc_runtime_path.empty())
		{
			tcc_set_lib_path(this->state, c_impl->libtcc_runtime_path.c_str());
		}

		/* Register execution paths */
		for (auto exec_path : c_impl->execution_paths)
		{
			tcc_add_include_path(this->state, exec_path.c_str());
			tcc_add_library_path(this->state, exec_path.c_str());
		}

		/* TODO */
		/*
		#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
			tcc_enable_debug(this->state);
		#endif
		*/

		/* TODO: Add error handling */
		/* tcc_set_error_func */

		/* TODO: Add some warnings? */
		/* tcc_set_warning */

		/* tcc_add_include_path, tcc_add_library_path */
		const char *loader_lib_path = loader_library_path();
		const char *include_dir = "../include";
		char join_path[PORTABILITY_PATH_SIZE];
		char metacall_include_path[PORTABILITY_PATH_SIZE];

		size_t join_path_size = portability_path_join(loader_lib_path, strlen(loader_lib_path) + 1, include_dir, strlen(include_dir) + 1, join_path, PORTABILITY_PATH_SIZE);
		(void)portability_path_canonical(join_path, join_path_size, metacall_include_path, PORTABILITY_PATH_SIZE);

		/* Add metacall include path */
		tcc_add_include_path(this->state, metacall_include_path);

		/* Add metacall library path (in other to find metacall library) */
		if (!c_impl->libtcc_runtime_path.empty())
		{
			tcc_add_library_path(this->state, c_impl->libtcc_runtime_path.c_str());
		}

		return true;
	}

	virtual int discover(loader_impl impl, context ctx)
	{
		/* Get all symbols */
		tcc_list_symbols(this->state, static_cast<void *>(this), &c_loader_impl_discover_symbols);

		/* Parse the AST and register functions */
		return c_loader_impl_discover_ast(impl, this, ctx);
	}

	virtual const void *symbol(std::string &name)
	{
		if (this->symbols.count(name) == 0)
		{
			return NULL;
		}

		return this->symbols[name];
	}

} * loader_impl_c_handle_tcc;

typedef struct loader_impl_c_handle_dynlink_type : loader_impl_c_handle_base_type
{
public:
	dynlink lib;

	loader_impl_c_handle_dynlink_type() :
		lib(NULL) {}

	virtual ~loader_impl_c_handle_dynlink_type()
	{
		if (lib != NULL)
		{
			dynlink_unload(this->lib);
		}
	}

	bool recursive_includes()
	{
		return true;
	}

	bool initialize(loader_impl_c c_impl, const loader_path path)
	{
		std::string lib_path_str(path);
		fs::path lib_path(lib_path_str);

		if (lib_path.is_absolute())
		{
			fs::path lib_dir = lib_path.parent_path();
			std::string lib_name = fs::path(lib_path).filename().string();

			this->load_dynlink(lib_dir, lib_name.c_str());

			/* If the path is absolute, we assume the header is in the same folder */
			if (this->lib != NULL)
			{
				return this->add_header(lib_dir, lib_name);
			}
		}
		else
		{
			bool header_found = false;

			for (auto exec_path : c_impl->execution_paths)
			{
				fs::path absolute_path(exec_path);

				absolute_path /= lib_path.parent_path();

				std::string lib_name = lib_path.filename().string();

				/* If the path is relative, we keep trying the exec_paths until we find the header,
				* this is for solving situations where we have the header in /usr/local/include and
				* the library in /usr/local/lib
				*/
				if (this->lib == NULL)
				{
					this->load_dynlink(absolute_path, lib_name.c_str());
				}

				if (header_found == false)
				{
					header_found = this->add_header(absolute_path, lib_name);
				}

				if (this->lib != NULL && header_found == true)
				{
					return true;
				}
			}
		}

		return false;
	}

	virtual const void *symbol(std::string &name)
	{
		dynlink_symbol_addr symbol_address = NULL;

		if (dynlink_symbol(this->lib, name.c_str(), &symbol_address) != 0)
		{
			return NULL;
		}

		return (const void *)(symbol_address);
	}

private:
	void load_dynlink(fs::path &path, const char *library_name)
	{
		/* This function will try to check if the library exists before loading it,
		so we avoid error messages from dynlink when guessing the file path for relative load from file */
		dynlink_name_impl platform_name;

		dynlink_platform_name(library_name, platform_name);

		fs::path lib_path(path);

		lib_path /= platform_name;

		if (fs::exists(lib_path) == true)
		{
			this->lib = dynlink_load(path.string().c_str(), library_name, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
		}
	}

	bool add_header(fs::path path, std::string &lib_name)
	{
		static const char *extensions[] = {
			".h",
			".hh",
			".H",
			".hpp",
			".h++",
			".hxx",
			"" /* No extension is also valid */
		};

		path /= lib_name;

		for (auto extension : extensions)
		{
			fs::path header(path);

			header += extension;

			if (fs::exists(header))
			{
				std::string header_str = header.string();

				this->add(header_str.c_str(), header_str.length() + 1);

				return true;
			}
		}

		return false;
	}

	virtual int discover(loader_impl impl, context ctx)
	{
		/* Parse the AST and register functions */
		return c_loader_impl_discover_ast(impl, this, ctx);
	}

} * loader_impl_c_handle_dynlink;

typedef struct loader_impl_c_function_type
{
	loader_impl_c_function_type(const void *address) :
		ret_type(NULL), arg_types(NULL), values(NULL), address(address) {}

	ffi_cif cif;
	ffi_type *ret_type;
	ffi_type **arg_types;
	void **values;
	const void *address;

} * loader_impl_c_function;

typedef struct c_loader_impl_discover_visitor_data_type
{
	loader_impl impl;
	loader_impl_c_handle_base c_handle;
	scope sp;
	int result;

} * c_loader_impl_discover_visitor_data;

/* Retrieve the equivalent FFI type from type id */
static ffi_type *c_loader_impl_ffi_type(type_id id);

/* Retrieve the type from the Clang type */
static type c_loader_impl_discover_type(loader_impl impl, CXCursor &cursor, CXType &cx_type);

/* Convert CXString to std::string */
static std::string c_loader_impl_cxstring_to_str(const CXString &s);

/* Obtain signature from parameters (function pointers) for the closures */
static enum CXChildVisitResult c_loader_impl_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data);

/* Trampoline of FFI in order to call to the MetaCall function */
static void c_loader_impl_function_closure(ffi_cif *cif, void *ret, void *args[], void *user_data);

/* Iterator for the visitor function */
class c_loader_closure_visitor
{
public:
	loader_impl impl;
	std::vector<type> args;

	c_loader_closure_visitor(loader_impl impl) :
		impl(impl) {}
	~c_loader_closure_visitor() {}
};

/* Interface for implementing polymorphic C types */
class c_loader_type_impl
{
public:
	virtual ~c_loader_type_impl() = default;
};

class c_loader_closure_value;

class c_loader_closure_type : public c_loader_type_impl
{
protected:
	loader_impl impl;
	ffi_type **args;
	ffi_type *ret;
	size_t args_size;

	friend class c_loader_closure_value;

public:
	c_loader_closure_type(loader_impl impl) :
		impl(impl), args(nullptr), ret(NULL), args_size(0) {}

	int prepare(CXCursor cursor, CXType cx_type)
	{
		c_loader_closure_visitor closure_visitor(impl);

		if (clang_visitChildren(cursor, &c_loader_impl_visitor, static_cast<CXClientData>(&closure_visitor)) != 0)
		{
			return 1;
		}

		auto result_type = clang_getResultType(cx_type);
		type ret_type = c_loader_impl_discover_type(impl, cursor, result_type);

		ret = c_loader_impl_ffi_type(type_index(ret_type));
		args = new ffi_type *[closure_visitor.args.size()];

		for (type t : closure_visitor.args)
		{
			args[args_size++] = c_loader_impl_ffi_type(type_index(t));
		}

		return 0;
	}

	~c_loader_closure_type()
	{
		if (args != nullptr)
		{
			delete[] args;
		}
	}
};

/* Instance of cle closure type */
class c_loader_closure_value
{
private:
	ffi_cif cif;
	ffi_closure *closure;
	void *address;
	c_loader_closure_type *closure_type;

public:
	c_loader_closure_value(c_loader_closure_type *closure_type) :
		closure(NULL), address(NULL), closure_type(closure_type)
	{
		closure = static_cast<ffi_closure *>(ffi_closure_alloc(sizeof(ffi_closure), &address));
	}

	void *bind(function f)
	{
		if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, closure_type->args_size, closure_type->ret, closure_type->args) != FFI_OK)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid preparation of FFI CIF in callback (the pointer to the function be NULL, possibly producing a segfault)");
			return NULL;
		}

		if (ffi_prep_closure_loc(closure, &cif, c_loader_impl_function_closure, f, address) != FFI_OK)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid preparation of FFI closure in callback (the pointer to the function will be NULL, possibly producing a segfault)");
			return NULL;
		}

		return static_cast<void *>(&address);
	}

	~c_loader_closure_value()
	{
		if (closure != NULL)
		{
			ffi_closure_free(closure);
		}
	}
};

std::string c_loader_impl_cxstring_to_str(const CXString &s)
{
	std::string result = clang_getCString(s);
	clang_disposeString(s);
	return result;
}

enum CXChildVisitResult c_loader_impl_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
	c_loader_closure_visitor *closure_visitor = static_cast<c_loader_closure_visitor *>(client_data);

	(void)parent;

	CXCursorKind kind = clang_getCursorKind(cursor);
	CXType arg_type = clang_getCursorType(cursor);

	if (kind == CXCursor_ParmDecl)
	{
		type t = c_loader_impl_discover_type(closure_visitor->impl, cursor, arg_type);

		if (t == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to parse type %s", clang_getCString(clang_getTypeSpelling(arg_type)));
			return CXChildVisit_Break;
		}

		closure_visitor->args.push_back(t);
	}

	return CXChildVisit_Continue;
}

void c_loader_impl_function_closure(ffi_cif *cif, void *ret, void *args[], void *user_data)
{
	function f = static_cast<function>(user_data);
	signature s = function_signature(f);
	size_t args_size = signature_count(s);
	void **values = new void *[args_size];

	(void)cif;

	for (size_t args_count = 0; args_count < args_size; ++args_count)
	{
		type t = signature_get_type(s, args_count);
		type_id id = type_index(t);

		values[args_count] = value_type_create(args[args_count], value_type_id_size(id), id);
	}

	void *ret_val = metacallfv_s(f, values, args_size);
	size_t ret_size = value_type_size(ret_val);

	if (ret_size <= sizeof(ffi_arg))
	{
		memcpy(ret, value_data(ret_val), ret_size);
	}
	else
	{
		/* TODO: Not sure in the case that return size is greater than ffi_arg size */
		ret = NULL;
	}

	value_type_destroy(ret_val);

	for (size_t args_count = 0; args_count < args_size; ++args_count)
	{
		value_type_destroy(values[args_count]);
	}

	delete[] values;
}

static void c_loader_impl_discover_symbols(void *ctx, const char *name, const void *addr)
{
	loader_impl_c_handle_tcc c_handle = static_cast<loader_impl_c_handle_tcc>(ctx);

	c_handle->symbols.insert(std::pair<std::string, const void *>(name, addr));
}

static bool c_loader_impl_file_exists(const loader_path path)
{
	if (FILE *file = fopen(path, "r"))
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

ffi_type *c_loader_impl_ffi_type(type_id id)
{
	/* TODO: Add more types */
	switch (id)
	{
		case TYPE_BOOL:
			return &ffi_type_uchar;
		case TYPE_CHAR:
			return &ffi_type_schar;
		case TYPE_SHORT:
			return &ffi_type_sshort;
		case TYPE_INT:
			return &ffi_type_sint;
		case TYPE_LONG:
			return &ffi_type_slong;
		case TYPE_FLOAT:
			return &ffi_type_float;
		case TYPE_DOUBLE:
			return &ffi_type_double;
		case TYPE_PTR:
			return &ffi_type_pointer;
		case TYPE_FUNCTION:
			return &ffi_type_pointer;
	}

	return &ffi_type_void;
}

function_return function_c_interface_invoke(function func, function_impl impl, function_args args, size_t args_size)
{
	signature s = function_signature(func);

	if (args_size != signature_count(s))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid number of arguments when calling %s (canceling call in order to avoid a segfault)", function_name(func));
		return NULL;
	}

	loader_impl_c_function c_function = static_cast<loader_impl_c_function>(impl);
	std::vector<c_loader_closure_value *> closures;

	for (size_t args_count = 0; args_count < args_size; ++args_count)
	{
		type t = signature_get_type(s, args_count);
		type_id id = type_index(t);
		type_id value_id = value_type_id((value)args[args_count]);

		if (id != value_id)
		{
			log_write("metacall", LOG_LEVEL_ERROR,
				"Type mismatch in when calling %s in argument number %" PRIuS
				" (expected %s of type %s and received %s)."
				" Canceling call in order to avoid a segfault.",
				function_name(func),
				args_count,
				type_name(t),
				type_id_name(id),
				type_id_name(value_id));
			return NULL;
		}

		if (id == TYPE_FUNCTION)
		{
			c_loader_closure_value *closure = new c_loader_closure_value(static_cast<c_loader_closure_type *>(type_derived(t)));

			c_function->values[args_count] = closure->bind(value_to_function((value)args[args_count]));

			closures.push_back(closure);
		}
		else
		{
			c_function->values[args_count] = value_data((value)args[args_count]);
		}
	}

	type_id ret_id = type_index(signature_get_return(s));
	size_t ret_size = value_type_id_size(ret_id);
	void *ret = NULL;

	/* TODO: This if is not correct because the sizes of strings, objects, etc are
	relative to the pointer, not the value contents, we should review this */
	if (ret_size <= sizeof(ffi_arg))
	{
		ffi_arg result;

		ffi_call(&c_function->cif, FFI_FN(c_function->address), &result, c_function->values);

		ret = value_type_create(&result, ret_size, ret_id);
	}
	else
	{
		ret = value_type_create(NULL, ret_size, ret_id);

		ffi_call(&c_function->cif, FFI_FN(c_function->address), value_data(ret), c_function->values);
	}

	/* Clear allocated closures if any */
	for (c_loader_closure_value *closure : closures)
	{
		delete closure;
	}

	return ret;
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
		delete[] c_function->arg_types;
		delete[] c_function->values;
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

int type_c_interface_create(type t, type_impl impl)
{
	(void)t;
	(void)impl;

	return 0;
}

void type_c_interface_destroy(type t, type_impl impl)
{
	(void)t;

	if (impl != NULL)
	{
		c_loader_type_impl *type_impl = static_cast<c_loader_type_impl *>(impl);

		delete type_impl;
	}
}

type_interface type_c_singleton(void)
{
	static struct type_interface_type c_type_interface = {
		&type_c_interface_create,
		&type_c_interface_destroy
	};

	return &c_type_interface;
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
		{ TYPE_CHAR, "int8_t" },
		{ TYPE_CHAR, "uint8_t" },
		{ TYPE_CHAR, "int_least8_t" },
		{ TYPE_CHAR, "uint_least8_t" },
		{ TYPE_CHAR, "int_fast8_t" },
		{ TYPE_CHAR, "uint_fast8_t" },

		{ TYPE_SHORT, "short" },
		{ TYPE_SHORT, "int16_t" },
		{ TYPE_SHORT, "uint16_t" },
		{ TYPE_SHORT, "int_least16_t" },
		{ TYPE_SHORT, "uint_least16_t" },
		{ TYPE_SHORT, "int_fast16_t" },
		{ TYPE_SHORT, "uint_fast16_t" },

		{ TYPE_INT, "int" },
		{ TYPE_INT, "uint32_t" },
		{ TYPE_INT, "int32_t" },
		{ TYPE_INT, "int_least32_t" },
		{ TYPE_INT, "uint_least32_t" },
		{ TYPE_INT, "int_fast32_t" },
		{ TYPE_INT, "uint_fast32_t" },

		{ TYPE_LONG, "long" },
		{ TYPE_LONG, "long long" },
		{ TYPE_LONG, "uint64_t" },
		{ TYPE_LONG, "int64_t" },
		{ TYPE_LONG, "int_least64_t" },
		{ TYPE_LONG, "uint_least64_t" },
		{ TYPE_LONG, "int_fast64_t" },
		{ TYPE_LONG, "uint_fast64_t" },
		{ TYPE_LONG, "ptrdiff_t" },
		{ TYPE_LONG, "intptr_t" },
		{ TYPE_LONG, "uintptr_t" },

		{ TYPE_FLOAT, "float" },
		{ TYPE_DOUBLE, "double" },

		{ TYPE_NULL, "void" }

		/* TODO: Do more types */
	};

	size_t size = sizeof(type_id_name_pair) / sizeof(type_id_name_pair[0]);

	for (size_t index = 0; index < size; ++index)
	{
		type t = type_create(type_id_name_pair[index].id, type_id_name_pair[index].name, NULL, &type_c_singleton);

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

loader_impl_data c_loader_impl_initialize(loader_impl impl, configuration config)
{
	loader_impl_c c_impl;

	(void)impl;

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

	/* Store the configuration path for later use */
	value path = configuration_value_type(config, "loader_library_path", TYPE_STRING);

	if (path != NULL)
	{
		c_impl->libtcc_runtime_path = std::string(value_to_string(path), value_type_size(path));
	}

	/* Register initialization */
	loader_initialization_register(impl);

	return static_cast<loader_impl_data>(c_impl);
}

int c_loader_impl_execution_path(loader_impl impl, const loader_path path)
{
	loader_impl_c c_impl = static_cast<loader_impl_c>(loader_impl_get(impl));

	/* TODO: It would be interesting to support LD_LIBRARY_PATH or DYLD_LIBRARY_PATH
	* on startup for supporting standard execution paths */
	c_impl->execution_paths.push_back(path);

	return 0;
}

static type_id c_loader_impl_clang_type(loader_impl impl, CXCursor cursor, CXType cx_type, c_loader_type_impl **impl_type)
{
	switch (cx_type.kind)
	{
		/* We allow dynamically defining pointer types as opaque pointer type,
		this can be improved in the future when structs and functions (as parameters)
		are supported in order to fully support the types (AST),
		we can use clang_getPointeeType to achieve this */
		case CXType_Pointer: {
			CXType pointee_type = clang_getPointeeType(cx_type);

			/* Support for strings */
			if (pointee_type.kind == CXType_Char_S || pointee_type.kind == CXType_SChar ||
				pointee_type.kind == CXType_Char_U || pointee_type.kind == CXType_UChar)
			{
				return TYPE_STRING;
			}
			/* Support for function pointers */
			else if (pointee_type.kind == CXType_FunctionProto || pointee_type.kind == CXType_FunctionNoProto)
			{
				return c_loader_impl_clang_type(impl, cursor, pointee_type, impl_type);
			}

			return TYPE_PTR;
		}

		case CXType_FunctionProto:
		case CXType_FunctionNoProto: {
			c_loader_closure_type *closure_type = new c_loader_closure_type(impl);

			if (closure_type != nullptr)
			{
				if (closure_type->prepare(cursor, cx_type) == 0)
				{
					*impl_type = static_cast<c_loader_closure_type *>(closure_type);
					return TYPE_FUNCTION;
				}
				else
				{
					log_write("metacall", LOG_LEVEL_ERROR,
						"Function closure from parameter '%s' has failed to be prepared",
						c_loader_impl_cxstring_to_str(clang_getCursorSpelling(cursor)).c_str());
					delete closure_type;
				}
			}

			return TYPE_INVALID;
		}

		case CXType_Char_U:
		case CXType_UChar:
		case CXType_Char_S:
		case CXType_SChar:
			return TYPE_CHAR;

		case CXType_Bool:
			return TYPE_BOOL;

		case CXType_Int:
			return TYPE_INT;

		case CXType_Void:
			return TYPE_NULL;

		case CXType_Enum: {
			CXCursor referenced = clang_isReference(cursor.kind) ? clang_getCursorReferenced(cursor) : cursor;
			CXType enum_type = referenced.kind == CXCursor_TypedefDecl ? clang_getTypedefDeclUnderlyingType(referenced) : clang_getEnumDeclIntegerType(referenced);

			if (enum_type.kind == CXType_Invalid)
			{
				return TYPE_INVALID; /* TODO: Check this edge case */
			}

			return c_loader_impl_clang_type(impl, referenced, enum_type, impl_type);
		}

		/* Another possible problem may be type definitions, for example, we are expecting
		the type 'bool' but instead we get the type 'yeet' which is defined as: 'typedef bool yeet;'
		this would be of type CXType_Bool but it won't work because we check against string instead of Clang type.
		In order to avoid problems with this, we must get the canonical type */
		case CXType_Typedef:
			return c_loader_impl_clang_type(impl, cursor, clang_getCanonicalType(cx_type), impl_type);

		/* TODO: Add more types */
		default:
			return TYPE_INVALID;
	}
}

type c_loader_impl_discover_type(loader_impl impl, CXCursor &cursor, CXType &cx_type)
{
	auto type_str = c_loader_impl_cxstring_to_str(clang_getTypeSpelling(cx_type));
	type t = loader_impl_type(impl, type_str.c_str());

	if (t != NULL)
	{
		return t;
	}

	c_loader_type_impl *impl_type = NULL;
	type_id id = c_loader_impl_clang_type(impl, cursor, cx_type, &impl_type);

	t = type_create(id, type_str.c_str(), impl_type, &type_c_singleton);

	if (t == NULL)
	{
		return NULL;
	}

	if (loader_impl_type_define(impl, type_name(t), t) != 0)
	{
		type_destroy(t);
		return NULL;
	}

	return t;
}

static int c_loader_impl_discover_signature(loader_impl impl, loader_impl_c_handle_base c_handle, scope sp, CXCursor cursor)
{
	auto cursor_type = clang_getCursorType(cursor);
	auto func_name = c_loader_impl_cxstring_to_str(clang_getCursorSpelling(cursor));
	auto symbol_name = func_name;

#if (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
	symbol_name.insert(0, 1, '_');
#endif

	if (scope_get(sp, symbol_name.c_str()) != NULL)
	{
		log_write("metacall", LOG_LEVEL_WARNING, "Symbol '%s' redefined, skipping the function", func_name.c_str());
		return 0;
	}

	const void *address = c_handle->symbol(symbol_name);

	if (address == NULL)
	{
		log_write("metacall", LOG_LEVEL_WARNING, "Symbol '%s' not found, skipping the function", func_name.c_str());
		return 0;
	}

	loader_impl_c_function c_function = new loader_impl_c_function_type(address);

	int num_args = clang_Cursor_getNumArguments(cursor);
	size_t args_size = num_args < 0 ? (size_t)0 : (size_t)num_args;

	function f = function_create(func_name.c_str(), args_size, c_function, &function_c_singleton);
	signature s = function_signature(f);

	auto result_type = clang_getResultType(cursor_type);
	type ret_type = c_loader_impl_discover_type(impl, cursor, result_type);

	signature_set_return(s, ret_type);

	c_function->ret_type = c_loader_impl_ffi_type(type_index(ret_type));
	c_function->arg_types = new ffi_type *[args_size];
	c_function->values = new void *[args_size];

	for (size_t args_count = 0; args_count < args_size; ++args_count)
	{
		auto arg_cursor = clang_Cursor_getArgument(cursor, args_count);
		auto arg_name = c_loader_impl_cxstring_to_str(clang_getCursorSpelling(arg_cursor));
		auto arg_type = clang_getArgType(cursor_type, args_count);
		type t = c_loader_impl_discover_type(impl, arg_cursor, arg_type);

		signature_set(s, args_count, arg_name.c_str(), t);
		c_function->arg_types[args_count] = c_loader_impl_ffi_type(type_index(t));
	}

	if (ffi_prep_cif(&c_function->cif, FFI_DEFAULT_ABI, args_size, c_function->ret_type, c_function->arg_types) != FFI_OK)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to create the FFI CIF in function '%s', skipping the function", func_name.c_str());
		function_destroy(f);
		return 1;
	}

	value v = value_create_function(f);

	if (scope_define(sp, function_name(f), v) != 0)
	{
		value_type_destroy(v);
		return 1;
	}

	return 0;
}

static CXChildVisitResult c_loader_impl_discover_visitor(CXCursor cursor, CXCursor, void *data)
{
	c_loader_impl_discover_visitor_data visitor_data = static_cast<c_loader_impl_discover_visitor_data>(data);

	/* Include recursively when disabled, include only the header inlcuded is populated when enabled */
	if (visitor_data->c_handle->recursive_includes() == false)
	{
		if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
		{
			return CXChildVisit_Continue;
		}
	}

	CXCursorKind kind = clang_getCursorKind(cursor);

	if (kind == CXCursorKind::CXCursor_FunctionDecl)
	{
		if (c_loader_impl_discover_signature(visitor_data->impl, visitor_data->c_handle, visitor_data->sp, cursor) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to discover C function declaration '%s'", c_loader_impl_cxstring_to_str(clang_getCursorSpelling(cursor)).c_str());
			visitor_data->result = 1;
			return CXChildVisit_Break;
		}
	}

	return CXChildVisit_Continue;
}

static int c_loader_impl_discover_ast(loader_impl impl, loader_impl_c_handle_base c_handle, context ctx)
{
	loader_impl_c c_impl = static_cast<loader_impl_c>(loader_impl_get(impl));
	c_loader_impl_discover_visitor_data_type data = {
		impl,
		c_handle,
		context_scope(ctx),
		0
	};

	std::vector<std::string> includes;
	std::vector<const char *> command_line_args;

	/* Otherwise, check the execution paths */
	for (auto exec_path : c_impl->execution_paths)
	{
		includes.push_back("-I" + exec_path);
		command_line_args.push_back(includes.back().c_str());
	}

	for (std::string file : c_handle->files)
	{
		/* Define the command line arguments (simulating compiler flags) */
		CXIndex index = clang_createIndex(0, 1);
		CXTranslationUnit unit = clang_parseTranslationUnit(
			index,
			file.c_str(),
			command_line_args.data(), command_line_args.size(),
			nullptr, 0,
			CXTranslationUnit_None);

		if (unit == nullptr)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Unable to parse translation unit of: %s", file.c_str());
			clang_disposeIndex(index);
			return -1;
		}

		CXCursor cursor = clang_getTranslationUnitCursor(unit);
		clang_visitChildren(cursor, c_loader_impl_discover_visitor, static_cast<void *>(&data));

		clang_disposeTranslationUnit(unit);
		clang_disposeIndex(index);
	}

	return data.result;
}

loader_handle c_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size)
{
	loader_impl_c c_impl = static_cast<loader_impl_c>(loader_impl_get(impl));
	loader_impl_c_handle_tcc c_handle = new loader_impl_c_handle_tcc_type();

	if (c_handle->initialize(c_impl) == false)
	{
		goto error;
	}

	for (size_t iterator = 0; iterator < size; ++iterator)
	{
		size_t path_size = strnlen(paths[iterator], LOADER_PATH_SIZE) + 1;

		/* We assume it is a path so we load from path */
		if (portability_path_is_absolute(paths[iterator], path_size) == 0)
		{
			if (tcc_add_file(c_handle->state, paths[iterator]) == -1)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Failed to load file: %s", paths[iterator]);
				goto error;
			}

			c_handle->add(paths[iterator], path_size);
		}
		else
		{
			bool found = false;

			/* Otherwise, check the execution paths */
			for (auto exec_path : c_impl->execution_paths)
			{
				loader_path path;
				size_t path_size = portability_path_join(exec_path.c_str(), exec_path.length() + 1, paths[iterator], strnlen(paths[iterator], LOADER_PATH_SIZE) + 1, path, LOADER_PATH_SIZE);

				if (c_loader_impl_file_exists(path) == true)
				{
					if (tcc_add_file(c_handle->state, path) != -1)
					{
						c_handle->add(path, path_size);
						found = true;
						break;
					}
				}
			}

			if (found == false)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Failed to load file: %s", paths[iterator]);
				goto error;
			}
		}
	}

	if (tcc_relocate(c_handle->state, TCC_RELOCATE_AUTO) == -1)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "TCC failed to relocate");
		goto error;
	}

	return c_handle;

error:
	delete c_handle;
	return NULL;
}

loader_handle c_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size)
{
	loader_impl_c c_impl = static_cast<loader_impl_c>(loader_impl_get(impl));
	loader_impl_c_handle_tcc c_handle = new loader_impl_c_handle_tcc_type();

	/* Apparently TCC has an unsafe API for compiling strings */
	(void)size;

	if (c_handle->initialize(c_impl) == false)
	{
		goto error;
	}

	if (tcc_compile_string(c_handle->state, buffer) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to compile the buffer: %s", name);
		goto error;
	}

	if (tcc_relocate(c_handle->state, TCC_RELOCATE_AUTO) == -1)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "TCC failed to relocate");
		goto error;
	}

	/* TODO: Load the buffer with the parser while iterating after loading it with TCC */

	return c_handle;

error:
	delete c_handle;
	return NULL;
}

loader_handle c_loader_impl_load_from_package(loader_impl impl, const loader_path path)
{
	loader_impl_c c_impl = static_cast<loader_impl_c>(loader_impl_get(impl));
	loader_impl_c_handle_dynlink c_handle = new loader_impl_c_handle_dynlink_type();

	if (c_handle->initialize(c_impl, path) == false)
	{
		delete c_handle;
		return NULL;
	}

	return c_handle;
}

int c_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_c_handle_base c_handle = static_cast<loader_impl_c_handle_base>(handle);

	(void)impl;

	if (c_handle == NULL)
	{
		return 1;
	}

	delete c_handle;

	return 0;
}

int c_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_c_handle_base c_handle = static_cast<loader_impl_c_handle_base>(handle);

	return c_handle->discover(impl, ctx);
}

int c_loader_impl_destroy(loader_impl impl)
{
	loader_impl_c c_impl = static_cast<loader_impl_c>(loader_impl_get(impl));

	if (c_impl != NULL)
	{
		/* Destroy children loaders */
		loader_unload_children(impl);

		delete c_impl;

		return 0;
	}

	return 1;
}
