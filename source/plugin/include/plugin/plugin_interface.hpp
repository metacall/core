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

#ifndef PLUGIN_INTERFACE_HPP
#define PLUGIN_INTERFACE_HPP 1

/* Private interface for using it inside any loader, extension or plugin for registering functions */
/* TODO: Move this to source/metacall/include/metacall/private? */

#include <preprocessor/preprocessor.h>

#include <log/log.h>

#include <reflect/reflect.h>

#include <metacall/metacall.h>

#include <sstream>
#include <string>

#define EXTENSION_FUNCTION_IMPL_VOID(ret, name) \
	do \
	{ \
		if (metacall_register_loaderv(loader, context, PREPROCESSOR_STRINGIFY(name), name, ret, 0, NULL) != 0) \
		{ \
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: " PREPROCESSOR_STRINGIFY(name)); \
			return 1; \
		} \
	} while (0)

#define EXTENSION_FUNCTION_IMPL(ret, name, ...) \
	do \
	{ \
		enum metacall_value_id arg_types[] = { __VA_ARGS__ }; \
		if (metacall_register_loaderv(loader, context, PREPROCESSOR_STRINGIFY(name), name, ret, PREPROCESSOR_ARGS_COUNT(__VA_ARGS__), arg_types) != 0) \
		{ \
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register function: " PREPROCESSOR_STRINGIFY(name)); \
			return 1; \
		} \
	} while (0)

#define EXTENSION_FUNCTION(ret, name, ...) \
	PREPROCESSOR_IF(PREPROCESSOR_ARGS_EMPTY(__VA_ARGS__), \
		EXTENSION_FUNCTION_IMPL_VOID(ret, name), \
		EXTENSION_FUNCTION_IMPL(ret, name, __VA_ARGS__))

/* TODO: Move the log_write outside into the CLI or similar */
#define EXTENSION_FUNCTION_THROW(error) \
	do \
	{ \
		/* log_write("metacall", LOG_LEVEL_ERROR, error); */ \
		exception ex = exception_create_const(error, "PluginException", 0, ""); \
		throwable th = throwable_create(value_create_exception(ex)); \
		return value_create_throwable(th); \
	} while (0)

#define EXTENSION_FUNCTION_CHECK_ITERATOR(error, iterator, value) \
	if (metacall_value_id(args[iterator]) != value) \
	{ \
		std::stringstream ss; \
		ss << error ". The parameter number " PREPROCESSOR_STRINGIFY(PREPROCESSOR_ARGS_COUNT(iterator)) " requires a value of type " << metacall_value_id_name(value) << ", received: " << metacall_value_type_name(args[iterator]); \
		std::string error_msg = ss.str(); \
		EXTENSION_FUNCTION_THROW(error_msg.c_str()); \
	}

#define EXTENSION_FUNCTION_CHECK(error, ...) \
	do \
	{ \
		(void)data; /* TODO: Do something with data */ \
		/* Disable warning on args when no args */ \
		PREPROCESSOR_IF(PREPROCESSOR_ARGS_EMPTY(__VA_ARGS__), \
						(void)args; \
						, \
						PREPROCESSOR_EMPTY_SYMBOL()) \
		if (argc != PREPROCESSOR_ARGS_COUNT(__VA_ARGS__)) \
		{ \
			std::stringstream ss; \
			ss << error ". The required number of argumens is " PREPROCESSOR_STRINGIFY(PREPROCESSOR_ARGS_COUNT(__VA_ARGS__)) ", received: " << argc; \
			std::string error_msg = ss.str(); \
			EXTENSION_FUNCTION_THROW(error_msg.c_str()); \
		} \
		PREPROCESSOR_FOR(EXTENSION_FUNCTION_CHECK_ITERATOR, error, __VA_ARGS__) \
	} while (0)

#endif /* PLUGIN_INTERFACE_HPP */
