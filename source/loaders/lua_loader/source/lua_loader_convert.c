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

#include <log/log.h>
#include <reflect/reflect.h>

#include "lauxlib.h"
#include "lua.h"
// safe limit for recursion depth
#define LUA_LOADER_MAX_RECURSION_DEPTH 10

static int loader_impl_lua_is_integer(lua_State *L, int index)
{
	lua_Number n = lua_tonumber(L, index);
	lua_Integer i = (lua_Integer)n;
	return n == (lua_Number)i;
}

static int loader_impl_lua_is_array(lua_State *L, int index);
static value loader_impl_lua_lua_to_value_rec(lua_State *L, int index, int depth);

static int loader_impl_lua_is_array(lua_State *L, int index)
{
	int is_array = 1;
	size_t len = 0;
	size_t table_len;
	int top = lua_gettop(L);

	lua_pushvalue(L, index);

	/* Empty tables should be maps */
	table_len = (size_t)lua_objlen(L, -1);
	if (table_len == 0)
	{
		int has_entries = 0;
		lua_pushnil(L);
		if (lua_next(L, -2) != 0)
		{
			has_entries = 1;
			lua_pop(L, 2);
		}
		lua_settop(L, top);
		/* Empty table -> map (return 0/false), table with only hash entries -> let the loop decide */
		if (!has_entries)
		{
			return 0;
		}
		/* Has hash entries only, continue with loop to verify */
	}

	lua_pushnil(L);
	while (lua_next(L, -2) != 0)
	{
		if (lua_type(L, -2) != LUA_TNUMBER || !loader_impl_lua_is_integer(L, -2))
		{
			is_array = 0;
			lua_pop(L, 2);
			break;
		}

		lua_Integer key = lua_tointeger(L, -2);
		if (key <= 0 || (size_t)key != ++len)
		{
			is_array = 0;
			lua_pop(L, 2);
			break;
		}

		lua_pop(L, 1);
	}

	if (is_array && len != table_len)
	{
		is_array = 0;
	}

	lua_settop(L, top);
	return is_array;
}

static value loader_impl_lua_lua_to_value_rec(lua_State *L, int index, int depth)
{
	int lua_type_val;
	int abs_index;

	if (depth > LUA_LOADER_MAX_RECURSION_DEPTH)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Lua value conversion exceeded maximum recursion depth");
		return NULL;
	}

	abs_index = (index < 0) ? lua_gettop(L) + index + 1 : index;
	lua_type_val = lua_type(L, abs_index);

	switch (lua_type_val)
	{
		case LUA_TNIL: {
			return value_create_null();
		}

		case LUA_TBOOLEAN: {
			int b = lua_toboolean(L, abs_index);
			return value_create_bool((boolean)b);
		}

		case LUA_TNUMBER: {
			lua_Number n = lua_tonumber(L, abs_index);
			lua_Integer i = (lua_Integer)n;
			if (n == (lua_Number)i)
			{
				return value_create_int((int)i);
			}
			else
			{
				return value_create_double((double)n);
			}
		}

		case LUA_TSTRING: {
			size_t len;
			const char *str = lua_tolstring(L, abs_index, &len);
			return value_create_string(str, len);
		}

		case LUA_TTABLE: {
			int top = lua_gettop(L);
			value v = NULL;
			lua_pushvalue(L, abs_index);

			if (loader_impl_lua_is_array(L, -1))
			{
				size_t array_len = (size_t)lua_objlen(L, -1);
				value *array_values;

				v = value_create_array(NULL, array_len);
				if (v == NULL)
				{
					lua_settop(L, top);
					return NULL;
				}

				array_values = value_to_array(v);

				for (size_t i = 0; i < array_len; ++i)
				{
					lua_rawgeti(L, -1, (lua_Integer)(i + 1));
					array_values[i] = loader_impl_lua_lua_to_value_rec(L, -1, depth + 1);
					lua_pop(L, 1);

					if (array_values[i] == NULL)
					{
						value_type_destroy(v);
						lua_settop(L, top);
						return NULL;
					}
				}
			}
			else
			{
				size_t map_size = 0;
				value *map_values;

				lua_pushnil(L);
				while (lua_next(L, -2) != 0)
				{
					++map_size;
					lua_pop(L, 1);
				}

				v = value_create_map(NULL, map_size);
				if (v == NULL)
				{
					lua_settop(L, top);
					return NULL;
				}

				map_values = value_to_map(v);

				size_t map_index = 0;
				lua_pushnil(L);
				while (lua_next(L, -2) != 0)
				{
					value pair = value_create_array(NULL, 2);
					if (pair == NULL)
					{
						lua_pop(L, 1);
						value_type_destroy(v);
						lua_settop(L, top);
						return NULL;
					}

					value *pair_values = value_to_array(pair);
					pair_values[0] = loader_impl_lua_lua_to_value_rec(L, -2, depth + 1);
					pair_values[1] = loader_impl_lua_lua_to_value_rec(L, -1, depth + 1);

					if (pair_values[0] == NULL || pair_values[1] == NULL)
					{
						value_type_destroy(pair);
						lua_pop(L, 1);
						value_type_destroy(v);
						lua_settop(L, top);
						return NULL;
					}

					map_values[map_index++] = pair;
					lua_pop(L, 1);
				}
			}

			lua_settop(L, top);
			return v;
		}

		case LUA_TFUNCTION: {
			lua_pushvalue(L, abs_index);
			int ref = luaL_ref(L, LUA_REGISTRYINDEX);
			return value_create_int(ref);
		}

		case LUA_TUSERDATA:
		case LUA_TLIGHTUSERDATA: {
			void *ptr = lua_touserdata(L, abs_index);
			return value_create_ptr(ptr);
		}

		default: {
			log_write("metacall", LOG_LEVEL_WARNING, "Unsupported Lua type: %s", lua_typename(L, lua_type_val));
			return value_create_null();
		}
	}
}

value loader_impl_lua_lua_to_value(lua_State *L, int index)
{
	return loader_impl_lua_lua_to_value_rec(L, index, 0);
}

type_id loader_impl_lua_type_from_lua(lua_State *L, int index)
{
	int lua_type_val = lua_type(L, index);

	switch (lua_type_val)
	{
		case LUA_TNIL:
			return TYPE_NULL;

		case LUA_TBOOLEAN:
			return TYPE_BOOL;

		case LUA_TNUMBER: {
			lua_Number n = lua_tonumber(L, index);
			lua_Integer i = (lua_Integer)n;
			if (n == (lua_Number)i)
			{
				return TYPE_INT;
			}
			return TYPE_DOUBLE;
		}

		case LUA_TSTRING:
			return TYPE_STRING;

		case LUA_TTABLE: {
			int top = lua_gettop(L);
			lua_pushvalue(L, index);
			int is_arr = loader_impl_lua_is_array(L, -1);
			lua_settop(L, top);
			return is_arr ? TYPE_ARRAY : TYPE_MAP;
		}

		case LUA_TFUNCTION:
			return TYPE_FUNCTION;

		case LUA_TUSERDATA:
		case LUA_TLIGHTUSERDATA:
			return TYPE_PTR;

		default:
			return TYPE_NULL;
	}
}

int loader_impl_lua_value_to_lua(lua_State *L, value v)
{
	if (v == NULL)
	{
		lua_pushnil(L);
		return 0;
	}

	type_id id = value_type_id(v);

	switch (id)
	{
		case TYPE_NULL: {
			lua_pushnil(L);
			break;
		}

		case TYPE_BOOL: {
			boolean b = value_to_bool(v);
			lua_pushboolean(L, b ? 1 : 0);
			break;
		}

		case TYPE_CHAR: {
			char c = value_to_char(v);
			lua_pushlstring(L, &c, 1);
			break;
		}

		case TYPE_SHORT: {
			short s = value_to_short(v);
			lua_pushinteger(L, (lua_Integer)s);
			break;
		}

		case TYPE_INT: {
			int i = value_to_int(v);
			lua_pushinteger(L, (lua_Integer)i);
			break;
		}

		case TYPE_LONG: {
			long l = value_to_long(v);
			lua_pushinteger(L, (lua_Integer)l);
			break;
		}

		case TYPE_FLOAT: {
			float f = value_to_float(v);
			lua_pushnumber(L, (lua_Number)f);
			break;
		}

		case TYPE_DOUBLE: {
			double d = value_to_double(v);
			lua_pushnumber(L, (lua_Number)d);
			break;
		}

		case TYPE_STRING: {
			const char *str = value_to_string(v);
			if (str == NULL)
			{
				lua_pushnil(L);
			}
			else
			{
				size_t len = value_type_size(v);
				if (len > 0 && str[len - 1] == '\0')
				{
					--len;
				}
				lua_pushlstring(L, str, len);
			}
			break;
		}

		case TYPE_BUFFER: {
			size_t len = value_type_size(v);
			void *ptr = value_to_buffer(v);
			lua_pushlstring(L, (const char *)ptr, len);
			break;
		}

		case TYPE_ARRAY: {
			size_t count = value_type_count(v);
			value *values = value_to_array(v);
			lua_createtable(L, (int)count, 0);
			for (size_t i = 0; i < count; ++i)
			{
				loader_impl_lua_value_to_lua(L, values[i]);
				lua_rawseti(L, -2, (lua_Integer)(i + 1));
			}
			break;
		}

		case TYPE_MAP: {
			size_t count = value_type_count(v);
			value *entries = value_to_map(v);
			lua_createtable(L, 0, (int)count);
			for (size_t i = 0; i < count; ++i)
			{
				value *pair = value_to_array(entries[i]);
				loader_impl_lua_value_to_lua(L, pair[0]);
				loader_impl_lua_value_to_lua(L, pair[1]);
				lua_settable(L, -3);
			}
			break;
		}

		case TYPE_PTR: {
			void *ptr = value_to_ptr(v);
			lua_pushlightuserdata(L, ptr);
			break;
		}

		case TYPE_FUNCTION: {
			int ref = value_to_int(v);
			lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
			break;
		}

		default: {
			log_write("metacall", LOG_LEVEL_WARNING, "Unsupported MetaCall type: %s", type_id_name(id));
			lua_pushnil(L);
			break;
		}
	}

	return 0;
}
