/*
 *	Loader Library by Parra Studios
 *	A plugin for loading lua code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <lua_loader/lua_loader_convert.h>
#include <lua_loader/lua_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>
#include <reflect/reflect_value.h>

#include <log/log.h>

#include <adt/adt_set.h>
#include <adt/adt_vector.h>

#include <portability/portability_path.h>

#include <stdlib.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

typedef struct loader_impl_lua_type
{
	lua_State *vm;
	int error_handler_ref;
	vector execution_paths;
} *loader_impl_lua;

typedef struct loader_impl_lua_handle_type
{
	char *name;
	int env_ref;
} *loader_impl_lua_handle;

typedef struct loader_impl_lua_function_type
{
	int func_ref;
	char *name;
	loader_impl impl;
	loader_impl_lua_handle handle;
} *loader_impl_lua_function;

static int lua_loader_error_handler(lua_State *L)
{
	lua_getglobal(L, "debug");
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 1;
	}
	lua_getfield(L, -1, "traceback");
	lua_remove(L, -2);
	if (!lua_isfunction(L, -1))
	{
		lua_pop(L, 1);
		return 1;
	}
	lua_pushvalue(L, 1);
	lua_pushinteger(L, 2);
	lua_call(L, 2, 1);
	return 1;
}

static int lua_loader_impl_handle_env_create(lua_State *L, int *env_ref)
{
	if (env_ref == NULL)
	{
		return 1;
	}

	lua_newtable(L);
	lua_newtable(L);

	lua_pushvalue(L, LUA_GLOBALSINDEX);
	lua_setfield(L, -2, "__index");

	lua_setmetatable(L, -2);

	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "_G");

	*env_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	if (*env_ref == LUA_REFNIL)
	{
		return 1;
	}

	return 0;
}

static int lua_loader_impl_initialize_types(loader_impl impl)
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
		{ TYPE_BUFFER, "string" },
		{ TYPE_PTR, "userdata" },
		{ TYPE_NULL, "nil" },
		{ TYPE_FUNCTION, "function" },
		{ TYPE_ARRAY, "table" },
		{ TYPE_MAP, "table" },
		{ TYPE_PTR, "any" } /* Generic any type for dynamic typing */
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
	loader_impl_lua lua_impl = (loader_impl_lua)malloc(sizeof(struct loader_impl_lua_type));

	(void)config;

	if (lua_impl == NULL)
	{
		return NULL;
	}

	if (lua_loader_impl_initialize_types(impl) != 0)
	{
		free(lua_impl);
		return NULL;
	}

	lua_impl->vm = luaL_newstate();
	if (lua_impl->vm == NULL)
	{
		free(lua_impl);
		return NULL;
	}

	lua_impl->execution_paths = vector_create_type(char *);
	if (lua_impl->execution_paths == NULL)
	{
		lua_close(lua_impl->vm);
		free(lua_impl);
		return NULL;
	}

	luaL_openlibs(lua_impl->vm);

	lua_pushcfunction(lua_impl->vm, &lua_loader_error_handler);
	lua_impl->error_handler_ref = luaL_ref(lua_impl->vm, LUA_REGISTRYINDEX);

	loader_initialization_register(impl);

	return lua_impl;
}

int lua_loader_impl_execution_path(loader_impl impl, const loader_path path)
{
	loader_impl_lua lua_impl = (loader_impl_lua)loader_impl_get(impl);
	lua_State *L = lua_impl->vm;
	size_t path_len = strlen(path);
	char *path_copy;

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");

	const char *current_path = lua_tostring(L, -1);
	if (current_path == NULL)
	{
		lua_pop(L, 2);
		return 1;
	}

	size_t current_len = strlen(current_path);
	size_t new_len = current_len + 1 + path_len + 4;
	char *new_path = (char *)malloc(new_len);

	if (new_path == NULL)
	{
		lua_pop(L, 2);
		return 1;
	}

	snprintf(new_path, new_len, "%s;%s/?.lua", current_path, path);

	lua_pushstring(L, new_path);
	lua_setfield(L, -3, "path");
	lua_pop(L, 2);

	free(new_path);

	/* Store path in execution_paths vector for relative file lookups */
	path_copy = strdup(path);
	if (path_copy == NULL)
	{
		return 1;
	}

	vector_push_back(lua_impl->execution_paths, &path_copy);

	return 0;
}

static int lua_loader_impl_try_load_file(lua_State *L, const char *path, int env_ref)
{
	if (luaL_loadfile(L, path) != 0)
	{
		return 1;
	}

	lua_rawgeti(L, LUA_REGISTRYINDEX, env_ref);

	if (lua_setfenv(L, -2) == 0)
	{
		lua_pop(L, 1);
		lua_pushstring(L, "failed to set chunk environment");
		return 1;
	}

	if (lua_pcall(L, 0, 0, 0) != 0)
	{
		return 1;
	}

	return 0;
}

loader_handle lua_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size, void *data)
{
	loader_impl_lua lua_impl = (loader_impl_lua)loader_impl_get(impl);
	loader_impl_lua_handle handle;
	size_t iterator;
	char error_message[512];

	(void)data;

	if (size == 0)
	{
		return NULL;
	}

	handle = (loader_impl_lua_handle)malloc(sizeof(struct loader_impl_lua_handle_type));
	if (handle == NULL)
	{
		return NULL;
	}

	handle->name = NULL;
	handle->env_ref = LUA_REFNIL;
	error_message[0] = '\0';

	if (lua_loader_impl_handle_env_create(lua_impl->vm, &handle->env_ref) != 0)
	{
		free(handle);
		return NULL;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		const char *path = paths[iterator];
		int loaded = 0;

		/* Try to load directly first (absolute or relative to CWD) */
		if (lua_loader_impl_try_load_file(lua_impl->vm, path, handle->env_ref) == 0)
		{
			loaded = 1;
		}
		else
		{
			const char *error_text = lua_tostring(lua_impl->vm, -1);
			if (error_text != NULL)
			{
				snprintf(error_message, sizeof(error_message), "%s", error_text);
			}
			else
			{
				error_message[0] = '\0';
			}
			lua_pop(lua_impl->vm, 1); /* Pop error message */

			/* Try execution paths for relative paths */
			size_t path_count = vector_size(lua_impl->execution_paths);
			for (size_t i = 0; i < path_count; ++i)
			{
				char **exec_path = vector_at(lua_impl->execution_paths, i);
				loader_path join_path;
				size_t join_size = portability_path_join(*exec_path, strnlen(*exec_path, LOADER_PATH_SIZE) + 1,
					path, strnlen(path, LOADER_PATH_SIZE) + 1, join_path, LOADER_PATH_SIZE);

				if (join_size > 0 && join_size <= LOADER_PATH_SIZE)
				{
					if (lua_loader_impl_try_load_file(lua_impl->vm, join_path, handle->env_ref) == 0)
					{
						path = join_path;
						loaded = 1;
						break;
					}
					else
					{
						const char *error_text = lua_tostring(lua_impl->vm, -1);
						if (error_text != NULL)
						{
							snprintf(error_message, sizeof(error_message), "%s", error_text);
						}
						else
						{
							error_message[0] = '\0';
						}
						lua_pop(lua_impl->vm, 1); /* Pop error message */
					}
				}
			}
		}

		if (!loaded)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Lua failed to load file %s: %s",
				paths[iterator], error_message[0] != '\0' ? error_message : "unknown error");
			luaL_unref(lua_impl->vm, LUA_REGISTRYINDEX, handle->env_ref);
			free(handle);
			return NULL;
		}

		if (handle->name == NULL)
		{
			handle->name = strdup(path);
		}

		log_write("metacall", LOG_LEVEL_DEBUG, "Lua module %s loaded from file", path);
	}

	return (loader_handle)handle;
}

loader_handle lua_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size, void *data)
{
	loader_impl_lua lua_impl = (loader_impl_lua)loader_impl_get(impl);
	loader_impl_lua_handle handle;
	size_t buffer_size = size;

	(void)data;

	/* Exclude null terminator if present, luaL_loadbuffer expects source code only */
	if (buffer_size > 0 && buffer[buffer_size - 1] == '\0')
	{
		buffer_size--;
	}

	handle = (loader_impl_lua_handle)malloc(sizeof(struct loader_impl_lua_handle_type));
	if (handle == NULL)
	{
		return NULL;
	}

	handle->name = NULL;
	handle->env_ref = LUA_REFNIL;

	if (lua_loader_impl_handle_env_create(lua_impl->vm, &handle->env_ref) != 0)
	{
		free(handle);
		return NULL;
	}

	if (luaL_loadbuffer(lua_impl->vm, buffer, buffer_size, name) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Lua failed to load buffer %s: %s",
			name, lua_tostring(lua_impl->vm, -1));
		lua_pop(lua_impl->vm, 1);
		luaL_unref(lua_impl->vm, LUA_REGISTRYINDEX, handle->env_ref);
		free(handle);
		return NULL;
	}

	lua_rawgeti(lua_impl->vm, LUA_REGISTRYINDEX, handle->env_ref);

	if (lua_setfenv(lua_impl->vm, -2) == 0)
	{
		lua_pop(lua_impl->vm, 1);
		luaL_unref(lua_impl->vm, LUA_REGISTRYINDEX, handle->env_ref);
		free(handle);
		return NULL;
	}

	if (lua_pcall(lua_impl->vm, 0, 0, 0) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Lua failed to execute buffer %s: %s",
			name, lua_tostring(lua_impl->vm, -1));
		lua_pop(lua_impl->vm, 1);
		luaL_unref(lua_impl->vm, LUA_REGISTRYINDEX, handle->env_ref);
		free(handle);
		return NULL;
	}

	handle->name = strdup(name);

	log_write("metacall", LOG_LEVEL_DEBUG, "Lua module %s loaded from memory", name);

	return (loader_handle)handle;
}

loader_handle lua_loader_impl_load_from_package(loader_impl impl, const loader_path path, void *data)
{
	(void)impl;
	(void)path;
	(void)data;

	log_write("metacall", LOG_LEVEL_WARNING, "Lua load_from_package not implemented");

	return NULL;
}

int lua_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_lua_handle lua_handle = (loader_impl_lua_handle)handle;

	(void)impl;

	if (lua_handle != NULL)
	{
		loader_impl_lua lua_impl = (loader_impl_lua)loader_impl_get(impl);

		if (lua_handle->env_ref != LUA_REFNIL)
		{
			luaL_unref(lua_impl->vm, LUA_REGISTRYINDEX, lua_handle->env_ref);
		}

		if (lua_handle->name != NULL)
		{
			free(lua_handle->name);
		}
		free(lua_handle);
		return 0;
	}

	return 1;
}

static int function_lua_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;
	return 0;
}

static function_return function_lua_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	loader_impl_lua_function lua_func = (loader_impl_lua_function)impl;
	loader_impl_lua lua_impl = (loader_impl_lua)loader_impl_get(lua_func->impl);
	lua_State *L = lua_impl->vm;
	signature s = function_signature(func);
	type ret_type = signature_get_return(s);
	size_t args_count;
	value ret = NULL;
	int top;
	int capture_results;

	lua_rawgeti(L, LUA_REGISTRYINDEX, lua_impl->error_handler_ref);
	int errfunc_idx = lua_gettop(L);
	top = errfunc_idx;
	capture_results = (ret_type != NULL && type_index(ret_type) != TYPE_NULL);

	lua_rawgeti(L, LUA_REGISTRYINDEX, lua_func->func_ref);

	for (args_count = 0; args_count < size; ++args_count)
	{
		value arg_value = args[args_count];
		loader_impl_lua_value_to_lua(L, arg_value);
	}

	int nresults = capture_results ? LUA_MULTRET : 0;

	int status = lua_pcall(L, (int)size, nresults, errfunc_idx);

	if (status != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Lua function %s error: %s",
			lua_func->name, lua_tostring(L, -1));
		lua_pop(L, 1);
		lua_remove(L, errfunc_idx);
		return NULL;
	}

	if (capture_results && lua_gettop(L) > errfunc_idx)
	{
		int result_count = lua_gettop(L) - errfunc_idx;

		if (result_count == 1)
		{
			ret = loader_impl_lua_lua_to_value(L, -1);
			lua_pop(L, 1);
		}
		else if (result_count > 1)
		{
			ret = value_create_array(NULL, (size_t)result_count);

			if (ret != NULL)
			{
				value *ret_values = value_to_array(ret);
				int first_result_idx = top + 1;

				for (int i = 0; i < result_count; ++i)
				{
					ret_values[i] = loader_impl_lua_lua_to_value(L, first_result_idx + i);
					if (ret_values[i] == NULL)
					{
						value_type_destroy(ret);
						ret = NULL;
						break;
					}
				}
			}

			lua_pop(L, result_count);
		}
	}

	lua_remove(L, errfunc_idx);

	return ret;
}

static function_return function_lua_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	(void)func;
	(void)impl;
	(void)args;
	(void)size;
	(void)resolve_callback;
	(void)reject_callback;
	(void)context;

	log_write("metacall", LOG_LEVEL_WARNING, "Lua async/await not implemented");

	return NULL;
}

static void function_lua_interface_destroy(function func, function_impl impl)
{
	loader_impl_lua_function lua_func = (loader_impl_lua_function)impl;
	loader_impl_lua lua_impl = (loader_impl_lua)loader_impl_get(lua_func->impl);

	(void)func;

	if (lua_func != NULL)
	{
		if (lua_func->func_ref != LUA_REFNIL)
		{
			luaL_unref(lua_impl->vm, LUA_REGISTRYINDEX, lua_func->func_ref);
		}
		if (lua_func->name != NULL)
		{
			free(lua_func->name);
		}
		free(lua_func);
	}
}

static function_interface function_lua_singleton(void)
{
	static struct function_interface_type lua_interface = {
		&function_lua_interface_create,
		&function_lua_interface_invoke,
		&function_lua_interface_await,
		&function_lua_interface_destroy
	};

	return &lua_interface;
}

int lua_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_lua lua_impl = (loader_impl_lua)loader_impl_get(impl);
	loader_impl_lua_handle lua_handle = (loader_impl_lua_handle)handle;
	lua_State *L = lua_impl->vm;
	scope sp = context_scope(ctx);
	int top = lua_gettop(L);

	lua_rawgeti(L, LUA_REGISTRYINDEX, lua_handle->env_ref);
	int env_table_idx = lua_gettop(L);
	lua_pushnil(L);

	while (lua_next(L, env_table_idx) != 0)
	{
		if (lua_isfunction(L, -1) && lua_type(L, -2) == LUA_TSTRING)
		{
			const char *func_name = lua_tostring(L, -2);
			loader_impl_lua_function lua_func;

			if (scope_get(sp, func_name) != NULL)
			{
				lua_pop(L, 1);
				continue;
			}

			lua_func = (loader_impl_lua_function)malloc(sizeof(struct loader_impl_lua_function_type));
			if (lua_func == NULL)
			{
				lua_pop(L, 2);
				lua_settop(L, top);
				return 1;
			}

			lua_pushvalue(L, -1);
			lua_func->func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
			lua_func->name = strdup(func_name);
			lua_func->impl = impl;
			lua_func->handle = (loader_impl_lua_handle)handle;

			function f = function_create(func_name, 0, lua_func, &function_lua_singleton);
			if (f == NULL)
			{
				luaL_unref(L, LUA_REGISTRYINDEX, lua_func->func_ref);
				free(lua_func->name);
				free(lua_func);
				lua_pop(L, 2);
				lua_settop(L, top);
				return 1;
			}

			signature s = function_signature(f);
			signature_set_return(s, loader_impl_type(impl, "any"));

			/* Get parameter count using debug.getinfo */
			int nparams = 0;
			int isvararg = 0;
			int getinfo_top = lua_gettop(L);
			lua_getglobal(L, "debug");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "getinfo");
				if (lua_isfunction(L, -1))
				{
					lua_pushvalue(L, -3);	/* Push the function */
					lua_pushstring(L, "u"); /* 'u' for nups in Lua 5.1/LuaJIT */
					if (lua_pcall(L, 2, 1, 0) == 0 && lua_istable(L, -1))
					{
						lua_getfield(L, -1, "nparams");
						if (lua_isnumber(L, -1))
						{
							nparams = (int)lua_tointeger(L, -1);
						}
						lua_pop(L, 1);

						/* Check if function is variadic */
						lua_getfield(L, -1, "isvararg");
						if (lua_isboolean(L, -1))
						{
							isvararg = lua_toboolean(L, -1);
						}
						lua_pop(L, 1);
					}
					/* Pop result table or error */
					lua_pop(L, 1);
				}
				else
				{
					lua_pop(L, 1); /* Pop getinfo if not a function */
				}
			}
			lua_pop(L, 1);				/* Pop debug table */
			lua_settop(L, getinfo_top); /* Restore stack to known state */

			if (isvararg)
			{
				nparams = 64; /* Allow up to 64 arguments for variadic functions */
			}

			/* Set signature parameters */
			for (int i = 0; i < nparams; i++)
			{
				signature_set(s, i, "", loader_impl_type(impl, "any"));
			}

			value v = value_create_function(f);
			if (scope_define(sp, function_name(f), v) != 0)
			{
				value_type_destroy(v);
				lua_pop(L, 2);
				lua_settop(L, top);
				return 1;
			}
		}
		lua_pop(L, 1);
	}

	lua_settop(L, top);

	return 0;
}

int lua_loader_impl_destroy(loader_impl impl)
{
	loader_impl_lua lua_impl = (loader_impl_lua)loader_impl_get(impl);

	if (lua_impl != NULL)
	{
		loader_unload_children(impl);

		if (lua_impl->vm != NULL)
		{
			lua_close(lua_impl->vm);
		}

		/* Clean up execution_paths vector */
		if (lua_impl->execution_paths != NULL)
		{
			size_t path_count = vector_size(lua_impl->execution_paths);
			for (size_t i = 0; i < path_count; ++i)
			{
				char **path = vector_at(lua_impl->execution_paths, i);
				free(*path);
			}
			vector_destroy(lua_impl->execution_paths);
		}

		free(lua_impl);
		return 0;
	}

	return 1;
}
