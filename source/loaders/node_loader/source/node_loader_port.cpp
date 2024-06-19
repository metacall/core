/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
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

/* -- Headers -- */

#include <node_loader/node_loader_impl.h>
#include <node_loader/node_loader_port.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <metacall/metacall.h>

#include <preprocessor/preprocessor_concatenation.h>
#include <preprocessor/preprocessor_stringify.h>

#include <cstdlib>
#include <cstring>

#include <node_api.h>

static const loader_tag node_loader_tag = "node";

napi_value node_loader_port_metacall(napi_env env, napi_callback_info info)
{
	size_t argc = 0;

	napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);

	if (argc == 0)
	{
		napi_throw_error(env, nullptr, "Invalid number of arguments");

		return nullptr;
	}

	napi_value *argv = new napi_value[argc];
	void **args = new void *[argc - 1];
	napi_value recv;

	napi_get_cb_info(env, info, &argc, argv, &recv, nullptr);

	size_t name_length;
	napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &name_length);

	char *name = new char[name_length + 1];

	if (name == nullptr)
	{
		napi_throw_error(env, nullptr, "Invalid function name allocation");

		return nullptr;
	}

	status = napi_get_value_string_utf8(env, argv[0], name, name_length + 1, &name_length);

	name[name_length] = '\0';

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl(node_loader_tag);
	loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	for (size_t args_count = 1; args_count < argc; ++args_count)
	{
		args[args_count - 1] = node_loader_impl_napi_to_value(node_impl, env, recv, argv[args_count]);
	}

	/* Call to the function */
	void *ret = metacallv_s(name, args, argc - 1);

	napi_value result = node_loader_impl_value_to_napi(node_impl, env, ret);

	if (metacall_value_id(ret) == METACALL_THROWABLE)
	{
		napi_throw(env, result);
	}

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, nullptr);

	for (size_t args_count = 0; args_count < argc - 1; ++args_count)
	{
		metacall_value_destroy(args[args_count]);
	}

	metacall_value_destroy(ret);

	delete[] argv;
	delete[] args;
	delete[] name;

	return result;
}

napi_value node_loader_port_metacall_await(napi_env env, napi_callback_info info)
{
	size_t argc = 0;

	napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);

	if (argc == 0)
	{
		napi_throw_error(env, nullptr, "Invalid number of arguments");

		return nullptr;
	}

	napi_value *argv = new napi_value[argc];
	void **args = new void *[argc - 1];
	napi_value recv;

	napi_get_cb_info(env, info, &argc, argv, &recv, nullptr);

	size_t name_length;

	napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &name_length);

	char *name = new char[name_length + 1];

	if (name == nullptr)
	{
		napi_throw_error(env, nullptr, "Invalid function name allocation");

		return nullptr;
	}

	status = napi_get_value_string_utf8(env, argv[0], name, name_length + 1, &name_length);

	name[name_length] = '\0';

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl(node_loader_tag);
	loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	for (size_t args_count = 1; args_count < argc; ++args_count)
	{
		args[args_count - 1] = node_loader_impl_napi_to_value(node_impl, env, recv, argv[args_count]);
	}

	/* Call to metacall await and wrap the promise into NodeJS land */
	napi_value promise = node_loader_impl_promise_await(node_impl, env, name, args, argc - 1);

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, nullptr);

	for (size_t args_count = 0; args_count < argc - 1; ++args_count)
	{
		metacall_value_destroy(args[args_count]);
	}

	delete[] argv;
	delete[] args;
	delete[] name;

	return promise;
}

napi_value node_loader_port_metacall_load_from_file(napi_env env, napi_callback_info info)
{
	/* TODO: Detect if input argument types are valid */

	const size_t args_size = 2;
	size_t argc = args_size, tag_length;
	napi_value argv[args_size];
	uint32_t paths_size, path_index = 0;

	napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

	napi_get_value_string_utf8(env, argv[0], nullptr, 0, &tag_length);

	if (tag_length == 0)
	{
		napi_throw_error(env, nullptr, "Invalid MetaCall tag");
		return nullptr;
	}

	char *tag = new char[tag_length + 1];

	napi_get_value_string_utf8(env, argv[0], tag, tag_length + 1, &tag_length);

	tag[tag_length] = '\0';

	napi_get_array_length(env, argv[1], &paths_size);

	char **paths = new char *[paths_size];

	for (uint32_t i = 0; i < paths_size; ++i)
	{
		napi_value path;
		size_t path_length;

		napi_get_element(env, argv[1], i, &path);

		napi_get_value_string_utf8(env, path, nullptr, 0, &path_length);

		if (path_length != 0)
		{
			paths[path_index] = new char[path_length + 1];

			napi_get_value_string_utf8(env, path, paths[path_index], path_length + 1, &path_length);

			paths[path_index][path_length] = '\0';

			++path_index;
		}
	}

	if (path_index == paths_size)
	{
		/* Obtain NodeJS loader implementation */
		loader_impl impl = loader_get_impl(node_loader_tag);
		loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

		/* Store current reference of the environment */
		node_loader_impl_env(node_impl, env);

		if (metacall_load_from_file(tag, const_cast<const char **>(paths), paths_size, NULL) != 0)
		{
			napi_throw_error(env, nullptr, "MetaCall could not load from file");
		}

		/* Release current reference of the environment */
		// node_loader_impl_env(node_impl, nullptr);
	}
	else
	{
		napi_throw_error(env, nullptr, "Invalid input paths");
	}

	delete[] tag;

	for (uint32_t i = 0; i < path_index; ++i)
	{
		delete[] paths[i];
	}

	delete[] paths;

	return nullptr;
}

napi_value node_loader_port_metacall_load_from_file_export(napi_env env, napi_callback_info info)
{
	/* TODO: Detect if input argument types are valid */

	const size_t args_size = 2;
	size_t argc = args_size, tag_length;
	napi_value argv[args_size];
	uint32_t paths_size, path_index = 0;

	napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

	napi_get_value_string_utf8(env, argv[0], nullptr, 0, &tag_length);

	if (tag_length == 0)
	{
		napi_throw_error(env, nullptr, "Invalid MetaCall tag");
		return nullptr;
	}

	char *tag = new char[tag_length + 1];

	napi_get_value_string_utf8(env, argv[0], tag, tag_length + 1, &tag_length);

	tag[tag_length] = '\0';

	napi_get_array_length(env, argv[1], &paths_size);

	char **paths = new char *[paths_size];

	for (uint32_t i = 0; i < paths_size; ++i)
	{
		napi_value path;
		size_t path_length;

		napi_get_element(env, argv[1], i, &path);

		napi_get_value_string_utf8(env, path, nullptr, 0, &path_length);

		if (path_length != 0)
		{
			paths[path_index] = new char[path_length + 1];

			napi_get_value_string_utf8(env, path, paths[path_index], path_length + 1, &path_length);

			paths[path_index][path_length] = '\0';

			++path_index;
		}
	}

	loader_impl_node node_impl = NULL;
	void *handle = NULL;

	if (path_index == paths_size)
	{
		/* Obtain NodeJS loader implementation */
		loader_impl impl = loader_get_impl(node_loader_tag);
		node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

		/* Store current reference of the environment */
		node_loader_impl_env(node_impl, env);

		if (metacall_load_from_file(tag, const_cast<const char **>(paths), paths_size, &handle) != 0)
		{
			napi_throw_error(env, nullptr, "MetaCall could not load from file");
		}

		/* Release current reference of the environment */
		// node_loader_impl_env(node_impl, nullptr);
	}
	else
	{
		napi_throw_error(env, nullptr, "Invalid input paths");
	}

	delete[] tag;

	for (uint32_t i = 0; i < path_index; ++i)
	{
		delete[] paths[i];
	}

	delete[] paths;

	void *exports = metacall_handle_export(handle);

	napi_value v_exports = node_loader_impl_value_to_napi(node_impl, env, exports);

	node_loader_impl_finalizer(env, v_exports, exports);

	return v_exports;
}

/**
*  @brief
*    Loads a script from string by tag
*
*  @param[in] env
*    N-API reference to the enviroment
*
*  @param[in] info
*    Reference to the call information
*
*  @return
*    TODO: Not implemented yet
*/
napi_value node_loader_port_metacall_load_from_memory(napi_env env, napi_callback_info info)
{
	const size_t args_size = 2;
	size_t argc = args_size, tag_length, script_length;
	napi_value argv[args_size];

	// Get arguments
	napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

	node_loader_impl_exception(env, status);

	// Get tag length
	status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &tag_length);

	node_loader_impl_exception(env, status);

	// Allocate tag
	char *tag = new char[tag_length + 1];

	if (tag == nullptr)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load from memory, tag allocation failed");
		return nullptr;
	}

	// Get tag
	status = napi_get_value_string_utf8(env, argv[0], tag, tag_length + 1, &tag_length);

	node_loader_impl_exception(env, status);

	// Get script length
	status = napi_get_value_string_utf8(env, argv[1], nullptr, 0, &script_length);

	node_loader_impl_exception(env, status);

	size_t script_size = script_length + 1;

	// Allocate script
	char *script = new char[script_size];

	if (script == nullptr)
	{
		delete[] tag;
		napi_throw_error(env, nullptr, "MetaCall could not load from memory, script allocation failed");
		return nullptr;
	}

	// Get script
	status = napi_get_value_string_utf8(env, argv[1], script, script_size, &script_length);

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl(node_loader_tag);
	loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	// Load script from memory
	if (metacall_load_from_memory(tag, script, script_size, NULL) != 0)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load from memory");
	}

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, nullptr);

	delete[] tag;
	delete[] script;

	/* TODO: Return value and logs */
	return nullptr;
}

napi_value node_loader_port_metacall_load_from_memory_export(napi_env env, napi_callback_info info)
{
	const size_t args_size = 2;
	size_t argc = args_size, tag_length, script_length;
	napi_value argv[args_size];

	// Get arguments
	napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

	node_loader_impl_exception(env, status);

	// Get tag length
	status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &tag_length);

	node_loader_impl_exception(env, status);

	// Allocate tag
	char *tag = new char[tag_length + 1];

	if (tag == nullptr)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load from memory, tag allocation failed");
		return nullptr;
	}

	// Get tag
	status = napi_get_value_string_utf8(env, argv[0], tag, tag_length + 1, &tag_length);

	node_loader_impl_exception(env, status);

	// Get script length
	status = napi_get_value_string_utf8(env, argv[1], nullptr, 0, &script_length);

	node_loader_impl_exception(env, status);

	size_t script_size = script_length + 1;

	// Allocate script
	char *script = new char[script_size];

	if (script == nullptr)
	{
		delete[] tag;
		napi_throw_error(env, nullptr, "MetaCall could not load from memory, script allocation failed");
		return nullptr;
	}

	// Get script
	status = napi_get_value_string_utf8(env, argv[1], script, script_size, &script_length);

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl(node_loader_tag);
	loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	void *handle = NULL;

	// Load script from memory
	if (metacall_load_from_memory(tag, script, script_size, &handle) != 0)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load from memory");
	}

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, nullptr);

	delete[] tag;
	delete[] script;

	void *exports = metacall_handle_export(handle);

	napi_value v_exports = node_loader_impl_value_to_napi(node_impl, env, exports);

	node_loader_impl_finalizer(env, v_exports, exports);

	return v_exports;
}

/**
*  @brief
*    Loads a script from configuration path
*
*  @param[in] env
*    N-API reference to the enviroment
*
*  @param[in] info
*    Reference to the call information
*/
napi_value node_loader_port_metacall_load_from_configuration(napi_env env, napi_callback_info info)
{
	const size_t args_size = 1;
	size_t argc = args_size, path_length;
	napi_value argv[args_size];

	// Get arguments
	napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

	node_loader_impl_exception(env, status);

	// Get tag length
	status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &path_length);

	node_loader_impl_exception(env, status);

	// Allocate path
	char *path = new char[path_length + 1];

	if (path == nullptr)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load from configuration, path allocation failed");
		return nullptr;
	}

	// Get path
	status = napi_get_value_string_utf8(env, argv[0], path, path_length + 1, &path_length);

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl(node_loader_tag);
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	// Load script from configuration
	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	if (metacall_load_from_configuration(path, NULL, allocator) != 0)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load from configuration");
	}

	metacall_allocator_destroy(allocator);

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, nullptr);

	delete[] path;

	/* TODO: Return value and logs */
	return nullptr;
}

napi_value node_loader_port_metacall_load_from_configuration_export(napi_env env, napi_callback_info info)
{
	const size_t args_size = 1;
	size_t argc = args_size, path_length;
	napi_value argv[args_size];

	// Get arguments
	napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

	node_loader_impl_exception(env, status);

	// Get tag length
	status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &path_length);

	node_loader_impl_exception(env, status);

	// Allocate path
	char *path = new char[path_length + 1];

	if (path == nullptr)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load from configuration, path allocation failed");
		return nullptr;
	}

	// Get path
	status = napi_get_value_string_utf8(env, argv[0], path, path_length + 1, &path_length);

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl(node_loader_tag);
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	// Load script from configuration
	void *handle = NULL;

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	if (metacall_load_from_configuration(path, &handle, allocator) != 0)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load from configuration");
	}

	metacall_allocator_destroy(allocator);

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, nullptr);

	delete[] path;

	void *exports = metacall_handle_export(handle);

	napi_value v_exports = node_loader_impl_value_to_napi(node_impl, env, exports);

	node_loader_impl_finalizer(env, v_exports, exports);

	return v_exports;
}

/* TODO: Add documentation */
napi_value node_loader_port_metacall_inspect(napi_env env, napi_callback_info)
{
	size_t size = 0;
	struct metacall_allocator_std_type std_ctx = { &malloc, &realloc, &free };
	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);
	char *inspect_str = metacall_inspect(&size, allocator);

	if (!(inspect_str != NULL && size != 0))
	{
		napi_throw_error(env, nullptr, "Invalid MetaCall inspect string");
	}

	napi_value result;
	napi_status status = napi_create_string_utf8(env, inspect_str, size - 1, &result);

	node_loader_impl_exception(env, status);

	metacall_allocator_free(allocator, inspect_str);

	metacall_allocator_destroy(allocator);

	return result;
}

/* TODO: Add documentation */
napi_value node_loader_port_metacall_logs(napi_env env, napi_callback_info)
{
	struct metacall_log_stdio_type log_stdio = { stdout };

	if (metacall_log(METACALL_LOG_STDIO, static_cast<void *>(&log_stdio)) != 0)
	{
		napi_throw_error(env, nullptr, "MetaCall failed to initialize debug logs");
	}

	return nullptr;
}

/* TODO: Review documentation */
// This functions sets the necessary js functions that could be called in NodeJs
void node_loader_port_exports(napi_env env, napi_value exports)
{
#define NODE_LOADER_PORT_DECL_FUNC(name) \
	do \
	{ \
		const char PREPROCESSOR_CONCAT(function_str_, name)[] = PREPROCESSOR_STRINGIFY(name); \
		napi_value PREPROCESSOR_CONCAT(function_, name); \
		napi_create_function(env, PREPROCESSOR_CONCAT(function_str_, name), sizeof(PREPROCESSOR_CONCAT(function_str_, name)) - 1, PREPROCESSOR_CONCAT(node_loader_port_, name), nullptr, &PREPROCESSOR_CONCAT(function_, name)); \
		napi_set_named_property(env, exports, PREPROCESSOR_CONCAT(function_str_, name), PREPROCESSOR_CONCAT(function_, name)); \
	} while (0)

#define NODE_LOADER_PORT_DECL_X_MACRO(x) \
	x(metacall); \
	x(metacall_await); \
	x(metacall_load_from_file); \
	x(metacall_load_from_file_export); \
	x(metacall_load_from_memory); \
	x(metacall_load_from_memory_export); \
	x(metacall_load_from_configuration); \
	x(metacall_load_from_configuration_export); \
	x(metacall_inspect); \
	x(metacall_logs);

	/* Declare all the functions */
	NODE_LOADER_PORT_DECL_X_MACRO(NODE_LOADER_PORT_DECL_FUNC)

#undef NODE_LOADER_PORT_DECL_FUNC
#undef NODE_LOADER_PORT_DECL_X_MACRO
}

/* TODO: Review documentation */
/* This function is called by NodeJs when the module is required */
napi_value node_loader_port_initialize(napi_env env, napi_value exports)
{
/* Note: This should not be necessary because we do not allow to use ports outside MetaCall */
#if 0
	if (metacall_initialize() != 0)
	{
		/* TODO: Show error message (when error handling is properly implemented in the core lib) */
		napi_throw_error(env, nullptr, "MetaCall failed to initialize");

		return nullptr;
	}
#endif

	node_loader_port_exports(env, exports);

	return exports;
}
