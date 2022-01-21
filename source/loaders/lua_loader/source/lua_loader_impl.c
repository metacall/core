/*
 *	Loader Library by Parra Studios
 *	A plugin for loading lua code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <lua_loader/lua_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <log/log.h>

#include <stdlib.h>

/*
	Note that the expected include convention is #include "lua.h"
	and not #include <lua/lua.h>. This is because, the lua location is not
	standardized and may exist in locations other than lua/
*/
#include "lua.h"

typedef struct loader_impl_lua_type
{
	lua_State *vm;

} * loader_impl_lua;

typedef struct loader_impl_lua_handle_type
{
	/* TODO: This probably won't be needed as Lua does not use any
		descriptor for the handles (aka scripts), all of them are loaded
		into global scope and they do not have references
	*/
	void *handle_lua_data;

} * loader_impl_lua_handle;

typedef struct loader_impl_lua_function_type
{
	/* TODO */
	loader_impl_lua_handle lua_handle;
	void *function_lua_data;

} * loader_impl_lua_function;

int function_lua_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

function_return function_lua_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	loader_impl_lua_function lua_function = (loader_impl_lua_function)impl;

	signature s = function_signature(func);

	const char *name = function_name(func);

	type ret_type = signature_get_return(s);

	(void)lua_function;

	log_write("metacall", LOG_LEVEL_DEBUG, "Invoking lua function %s", name);

	/* TODO: Convert properly the arguments */
	if (size > 0)
	{
		size_t args_count;

		for (args_count = 0; args_count < size; ++args_count)
		{
			/* This is only in case Lua adds types in the signature */
			type t = signature_get_type(s, args_count);

			type_id id = TYPE_INVALID;

			if (t == NULL)
			{
				id = value_type_id((value)args[args_count]);
			}
			else
			{
				id = type_index(t);
			}

			log_write("metacall", LOG_LEVEL_DEBUG, "Type %p, %d", (void *)t, id);

			if (id == TYPE_BOOL)
			{
				boolean *value_ptr = (boolean *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Boolean value: %d", *value_ptr);
			}
			else if (id == TYPE_CHAR)
			{
				char *value_ptr = (char *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Char value: %c", *value_ptr);
			}
			else if (id == TYPE_SHORT)
			{
				short *value_ptr = (short *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Short value: %d", *value_ptr);
			}
			else if (id == TYPE_INT)
			{
				int *value_ptr = (int *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Int value: %d", *value_ptr);
			}
			else if (id == TYPE_LONG)
			{
				long *value_ptr = (long *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Long value: %ld", *value_ptr);
			}
			else if (id == TYPE_FLOAT)
			{
				float *value_ptr = (float *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Float value: %f", *value_ptr);
			}
			else if (id == TYPE_DOUBLE)
			{
				double *value_ptr = (double *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Double value: %f", *value_ptr);
			}
			else if (id == TYPE_STRING)
			{
				const char *value_ptr = (const char *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "String value: %s", value_ptr);
			}
			else if (id == TYPE_PTR)
			{
				void *value_ptr = (void *)(args[args_count]);

				log_write("metacall", LOG_LEVEL_DEBUG, "Pointer value: %p", value_ptr);
			}
			else
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Unrecognized value: %p", args[args_count]);
			}
		}

		log_write("metacall", LOG_LEVEL_DEBUG, "Calling lua function with arguments (%lu)", size);
	}
	else
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Calling lua function without arguments");
	}

	/* TODO: Execute the call */

	/* TODO: Convert properly the return value */
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

			return value_create_string(str, sizeof(str) - 1);
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

function_return function_lua_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
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

void function_lua_interface_destroy(function func, function_impl impl)
{
	loader_impl_lua_function lua_function = (loader_impl_lua_function)impl;

	(void)func;

	if (lua_function != NULL)
	{
		free(lua_function);
	}
}

function_interface function_lua_singleton(void)
{
	static struct function_interface_type lua_interface = {
		&function_lua_interface_create,
		&function_lua_interface_invoke,
		&function_lua_interface_await,
		&function_lua_interface_destroy
	};

	return &lua_interface;
}

int lua_loader_impl_initialize_types(loader_impl impl)
{
	static struct
	{
		type_id id;
		const char *name;
	} type_id_name_pair[] = {
		{ TYPE_BOOL, "boolean" },
		{ TYPE_CHAR, "string" },
		{ TYPE_SHORT, "number" },
		{ TYPE_INT, "number" },
		{ TYPE_LONG, "number" },
		{ TYPE_FLOAT, "number" },
		{ TYPE_DOUBLE, "number" },
		{ TYPE_STRING, "string" },
		{ TYPE_PTR, "userdata" },
		{ TYPE_NULL, "nil" },
		{ TYPE_FUNCTION, "function" },
		{ TYPE_MAP, "table" }
		/* TODO: thread */
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

loader_impl_data lua_loader_impl_initialize(loader_impl impl, configuration config)
{
	loader_impl_lua lua_impl;

	(void)config;

	lua_impl = malloc(sizeof(struct loader_impl_lua_type));

	if (lua_impl == NULL)
	{
		return NULL;
	}

	if (lua_loader_impl_initialize_types(impl) != 0)
	{
		free(lua_impl);

		return NULL;
	}

	/* Initialize Lua VM */
	lua_impl->vm = luaL_newstate();

	if (lua_impl->vm == NULL)
	{
		free(lua_impl);

		return NULL;
	}

	/* Open all standard libraries into current Lua state */
	luaL_openlibs(lua_impl->vm);

	/* Register initialization */
	loader_initialization_register(impl);

	return lua_impl;
}

int lua_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	/* TODO */
	(void)impl;
	(void)path;

	return 0;
}

loader_handle lua_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	loader_impl_lua_handle handle = malloc(sizeof(struct loader_impl_lua_handle_type));

	(void)impl;

	if (handle != NULL)
	{
		size_t iterator;

		/* TODO */

		for (iterator = 0; iterator < size; ++iterator)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Lua module %s loaded from file", paths[iterator]);
		}

		handle->handle_lua_data = NULL;

		return (loader_handle)handle;
	}

	return NULL;
}

loader_handle lua_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char *buffer, size_t size)
{
	loader_impl_lua lua_impl = loader_impl_get(impl);
	loader_impl_lua_handle handle = malloc(sizeof(struct loader_impl_lua_handle_type));

	if (handle != NULL)
	{
		if (luaL_loadbuffer(lua_impl->vm, buffer, size, name) != 0)
		{
			log_write("metacall", LOG_LEVEL_Error, "Lua module %s failed to load: %s", name, lua_tostring(lua_impl->vm, -1));
			free(handle);
			return NULL;
		}

		log_write("metacall", LOG_LEVEL_DEBUG, "Lua module %s. loaded from memory", name);

		return (loader_handle)handle;
	}

	return NULL;
}

loader_handle lua_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	loader_impl_lua_handle handle = malloc(sizeof(struct loader_impl_lua_handle_type));

	(void)impl;

	if (handle != NULL)
	{
		/* TODO */

		log_write("metacall", LOG_LEVEL_DEBUG, "Lua module %s loaded from package", path);

		handle->handle_lua_data = NULL;

		return (loader_handle)handle;
	}

	return NULL;
}

int lua_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_lua_handle lua_handle = (loader_impl_lua_handle)handle;

	(void)impl;

	if (lua_handle != NULL)
	{
		/* TODO */

		free(lua_handle);

		return 0;
	}

	return 1;
}

loader_impl_lua_function lua_function_create(loader_impl_lua_handle lua_handle)
{
	loader_impl_lua_function lua_function = malloc(sizeof(struct loader_impl_lua_function_type));

	if (lua_function != NULL)
	{
		/* TODO */

		lua_function->lua_handle = lua_handle;

		lua_function->function_lua_data = NULL;

		return lua_function;
	}

	return NULL;
}

int lua_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_lua lua_impl = loader_impl_get(impl);

	loader_impl_lua_handle lua_handle = (loader_impl_lua_handle)handle;

	loader_impl_lua_function lua_function;

	scope sp = context_scope(ctx);

	(void)lua_impl;

	log_write("metacall", LOG_LEVEL_DEBUG, "Lua module %p discovering", handle);

	/* TODO: Introspect all functions */

	/* This is an example of how to create a function */
	lua_function = lua_function_create(lua_handle);

	if (lua_function != NULL)
	{
		function f = function_create("my_empty_func", 0, lua_function, &function_lua_singleton);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "Integer"));

		scope_define(sp, function_name(f), value_create_function(f));
	}

	return 0;
}

int lua_loader_impl_destroy(loader_impl impl)
{
	loader_impl_lua lua_impl = loader_impl_get(impl);

	if (lua_impl != NULL)
	{
		/* Destroy children loaders */
		loader_unload_children(impl, 0);

		/* Destroy Lua VM */
		lua_close(lua_impl->vm);

		free(lua_impl);

		return 0;
	}

	return 1;
}
