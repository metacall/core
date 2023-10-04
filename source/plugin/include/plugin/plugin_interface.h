/*
 *	Plugin Library by Parra Studios
 *	A library for plugins at run-time into a process.
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

#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H 1

/* Private interface for using it inside any loader, extension or plugin for registering functions */
/* TODO: Move this to source/metacall/include/metacall/private? */

#include <preprocessor/preprocessor.h>

#include <log/log.h>

#include <metacall/metacall.h>

#define EXTENSION_FUNCTION_IMPL_VOID(ret, name)                                                                   \
	do                                                                                                            \
	{                                                                                                             \
		if (metacall_register_loaderv(loader, context, PREPROCESSOR_STRINGIFY(name), name, ret, 0, NULL) != 0)    \
		{                                                                                                         \
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: " PREPROCESSOR_STRINGIFY(name)); \
			return 1;                                                                                             \
		}                                                                                                         \
	} while (0)

#define EXTENSION_FUNCTION_IMPL(ret, name, ...)                                                                                                        \
	do                                                                                                                                                 \
	{                                                                                                                                                  \
		enum metacall_value_id arg_types[] = { __VA_ARGS__ };                                                                                          \
		if (metacall_register_loaderv(loader, context, PREPROCESSOR_STRINGIFY(name), name, ret, PREPROCESSOR_ARGS_COUNT(__VA_ARGS__), arg_types) != 0) \
		{                                                                                                                                              \
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: " PREPROCESSOR_STRINGIFY(name));                                      \
			return 1;                                                                                                                                  \
		}                                                                                                                                              \
	} while (0)

#define EXTENSION_FUNCTION(ret, name, ...)                \
	PREPROCESSOR_IF(PREPROCESSOR_ARGS_EMPTY(__VA_ARGS__), \
		EXTENSION_FUNCTION_IMPL_VOID(ret, name),          \
		EXTENSION_FUNCTION_IMPL(ret, name, __VA_ARGS__))

#endif /* PLUGIN_INTERFACE_H */
