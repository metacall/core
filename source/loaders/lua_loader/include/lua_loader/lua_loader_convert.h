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

#ifndef LUA_LOADER_CONVERT_H
#define LUA_LOADER_CONVERT_H 1

#include <lua_loader/lua_loader_api.h>

#include <reflect/reflect_type_id.h>
#include <reflect/reflect_value.h>

#include <lua.h>

#ifdef __cplusplus
extern "C" {
#endif

LUA_LOADER_API int loader_impl_lua_value_to_lua(lua_State *L, value v);

LUA_LOADER_API value loader_impl_lua_lua_to_value(lua_State *L, int index);

LUA_LOADER_API type_id loader_impl_lua_type_from_lua(lua_State *L, int index);

#ifdef __cplusplus
}
#endif

#endif /* LUA_LOADER_CONVERT_H */
