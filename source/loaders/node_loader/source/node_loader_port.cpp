/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
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

/* -- Headers -- */

#include <node_loader/node_loader_impl.h>
#include <node_loader/node_loader_port.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <metacall/metacall.h>

#include <cstring>

#include <node_api.h>

struct promise_context_type
{
	loader_impl_node node_impl;
	napi_env env;
	void *ret_future;
	napi_deferred deferred;
};

napi_value node_loader_port_call(napi_env env, napi_callback_info info)
{
	size_t argc = 0;

	napi_get_cb_info(env, info, &argc, NULL, NULL, NULL);

	if (argc == 0)
	{
		napi_throw_error(env, NULL, "Invalid number of arguments");

		return nullptr;
	}

	napi_value *argv = new napi_value[argc];
	void **args = new void *[argc - 1];
	napi_value recv;

	napi_get_cb_info(env, info, &argc, argv, &recv, NULL);

	size_t name_length;

	napi_status status = napi_get_value_string_utf8(env, argv[0], NULL, 0, &name_length);

	char *name = new char[name_length + 1];

	if (name == nullptr)
	{
		napi_throw_error(env, NULL, "Invalid function name allocation");

		return nullptr;
	}

	status = napi_get_value_string_utf8(env, argv[0], name, name_length + 1, &name_length);

	name[name_length] = '\0';

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl("node");
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

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, NULL);

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

napi_value node_loader_port_await(napi_env env, napi_callback_info info)
{
#if 0 // TODO
	size_t argc = 0;

	napi_get_cb_info(env, info, &argc, NULL, NULL, NULL);

	if (argc == 0)
	{
		napi_throw_error(env, NULL, "Invalid number of arguments");

		return nullptr;
	}

	napi_value *argv = new napi_value[argc];
	void **args = new void *[argc - 1];
	napi_value recv;

	napi_get_cb_info(env, info, &argc, argv, &recv, NULL);

	size_t name_length;

	napi_status status = napi_get_value_string_utf8(env, argv[0], NULL, 0, &name_length);

	char *name = new char[name_length + 1];

	if (name == nullptr)
	{
		napi_throw_error(env, NULL, "Invalid function name allocation");

		return nullptr;
	}

	status = napi_get_value_string_utf8(env, argv[0], name, name_length + 1, &name_length);

	name[name_length] = '\0';

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl("node");
	loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	for (size_t args_count = 1; args_count < argc; ++args_count)
	{
		args[args_count - 1] = node_loader_impl_napi_to_value(node_impl, env, recv, argv[args_count]);
	}

	promise_context_type *ctx = new promise_context_type();

	if (ctx == nullptr)
	{
		napi_throw_error(env, NULL, "Failed to allocate the promise context");

		return nullptr;
	}

	napi_value promise;

	/* Create the promise */
	status = napi_create_promise(env, &ctx->deferred, &promise);

	if (status != napi_ok)
	{
		napi_throw_error(env, NULL, "Failed to create the promise");

		delete ctx;

		return nullptr;
	}

	ctx->node_impl = node_impl;
	ctx->env = env;

	auto resolve = [](void *result, void *data) -> void * {
		promise_context_type *ctx = static_cast<promise_context_type *>(data);
		napi_value js_result = node_loader_impl_value_to_napi(ctx->node_impl, ctx->env, result);
		napi_status status = napi_resolve_deferred(ctx->env, ctx->deferred, js_result);

		if (status != napi_ok)



		return NULL;
	};

	auto reject = [](void *result, void *ctx) -> void * {
		promise_context_type *ctx = static_cast<promise_context_type *>(data);
		napi_value js_result = node_loader_impl_value_to_napi(ctx->node_impl, ctx->env, result);
		napi_status status = napi_reject_deferred(ctx->env, ctx->deferred, js_result);


		return NULL;
	};

	/* Await to the function */
	ctx->ret_future = metacall_await_s(name, args, argc - 1, resolve, reject, ctx);





	napi_value result = node_loader_impl_value_to_napi(node_impl, env, ret);

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, NULL);

	for (size_t args_count = 0; args_count < argc - 1; ++args_count)
	{
		metacall_value_destroy(args[args_count]);
	}

	metacall_value_destroy(ret);

	delete[] argv;
	delete[] args;
	delete[] name;

	return result;
#endif
}

napi_value node_loader_port_load_from_file(napi_env env, napi_callback_info info)
{
	/* TODO: Detect if input argument types are valid */

	const size_t args_size = 2;
	size_t argc = args_size, tag_length;
	napi_value argv[args_size];
	uint32_t paths_size, path_index = 0;
	char *tag;

	napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

	napi_get_value_string_utf8(env, argv[0], NULL, 0, &tag_length);

	if (tag_length == 0)
	{
		napi_throw_error(env, NULL, "Invalid MetaCall tag");
		return NULL;
	}

	tag = new char[tag_length + 1];

	napi_get_value_string_utf8(env, argv[0], tag, tag_length + 1, &tag_length);

	tag[tag_length] = '\0';

	napi_get_array_length(env, argv[1], &paths_size);

	char **paths = new char *[paths_size];

	for (uint32_t i = 0; i < paths_size; ++i)
	{
		napi_value path;
		size_t path_length;

		napi_get_element(env, argv[1], i, &path);

		napi_get_value_string_utf8(env, path, NULL, 0, &path_length);

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
		loader_impl impl = loader_get_impl("node");
		loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

		/* Store current reference of the environment */
		node_loader_impl_env(node_impl, env);

		if (metacall_load_from_file(tag, (const char **)paths, paths_size, NULL) != 0)
		{
			napi_throw_error(env, NULL, "MetaCall could not load from file");
		}

		/* Release current reference of the environment */
		// node_loader_impl_env(node_impl, NULL);
	}
	else
	{
		napi_throw_error(env, NULL, "Invalid input paths");
	}

	delete[] tag;

	for (uint32_t i = 0; i < path_index; ++i)
	{
		delete[] paths[i];
	}

	delete[] paths;

	return NULL;
}

napi_value node_loader_port_load_from_file_export(napi_env env, napi_callback_info info)
{
	/* TODO: Detect if input argument types are valid */

	const size_t args_size = 2;
	size_t argc = args_size, tag_length;
	napi_value argv[args_size];
	uint32_t paths_size, path_index = 0;
	char *tag;

	napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

	napi_get_value_string_utf8(env, argv[0], NULL, 0, &tag_length);

	if (tag_length == 0)
	{
		napi_throw_error(env, NULL, "Invalid MetaCall tag");
		return NULL;
	}

	tag = new char[tag_length + 1];

	napi_get_value_string_utf8(env, argv[0], tag, tag_length + 1, &tag_length);

	tag[tag_length] = '\0';

	napi_get_array_length(env, argv[1], &paths_size);

	char **paths = new char *[paths_size];

	for (uint32_t i = 0; i < paths_size; ++i)
	{
		napi_value path;
		size_t path_length;

		napi_get_element(env, argv[1], i, &path);

		napi_get_value_string_utf8(env, path, NULL, 0, &path_length);

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
		loader_impl impl = loader_get_impl("node");
		node_impl = (loader_impl_node)loader_impl_get(impl);

		/* Store current reference of the environment */
		node_loader_impl_env(node_impl, env);

		if (metacall_load_from_file(tag, (const char **)paths, paths_size, &handle) != 0)
		{
			napi_throw_error(env, NULL, "MetaCall could not load from file");
		}

		/* Release current reference of the environment */
		// node_loader_impl_env(node_impl, NULL);
	}
	else
	{
		napi_throw_error(env, NULL, "Invalid input paths");
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
napi_value node_loader_port_load_from_memory(napi_env env, napi_callback_info info)
{
	const size_t args_size = 2;
	size_t argc = args_size, tag_length, script_length, script_size;
	napi_value argv[args_size];
	napi_status status;
	char *tag, *script;

	// Get arguments
	status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

	node_loader_impl_exception(env, status);

	// Get tag length
	status = napi_get_value_string_utf8(env, argv[0], NULL, 0, &tag_length);

	node_loader_impl_exception(env, status);

	// Allocate tag
	tag = static_cast<char *>(malloc(sizeof(char) * (tag_length + 1)));

	if (tag == NULL)
	{
		napi_throw_error(env, NULL, "MetaCall could not load from memory, tag allocation failed");
		return NULL;
	}

	// Get tag
	status = napi_get_value_string_utf8(env, argv[0], tag, tag_length + 1, &tag_length);

	node_loader_impl_exception(env, status);

	// Get script length
	status = napi_get_value_string_utf8(env, argv[1], NULL, 0, &script_length);

	node_loader_impl_exception(env, status);

	script_size = script_length + 1;

	// Allocate script
	script = static_cast<char *>(malloc(sizeof(char) * script_size));

	if (script == NULL)
	{
		free(tag);
		napi_throw_error(env, NULL, "MetaCall could not load from memory, script allocation failed");
		return NULL;
	}

	// Get script
	status = napi_get_value_string_utf8(env, argv[1], script, script_size, &script_length);

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl("node");
	loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	// Load script from memory
	if (metacall_load_from_memory(tag, script, script_size, NULL) != 0)
	{
		napi_throw_error(env, NULL, "MetaCall could not load from memory");
	}

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, NULL);

	free(tag);
	free(script);

	/* TODO: Return value and logs */
	return NULL;
}

napi_value node_loader_port_load_from_memory_export(napi_env env, napi_callback_info info)
{
	const size_t args_size = 2;
	size_t argc = args_size, tag_length, script_length, script_size;
	napi_value argv[args_size];
	napi_status status;
	char *tag, *script;

	// Get arguments
	status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

	node_loader_impl_exception(env, status);

	// Get tag length
	status = napi_get_value_string_utf8(env, argv[0], NULL, 0, &tag_length);

	node_loader_impl_exception(env, status);

	// Allocate tag
	tag = static_cast<char *>(malloc(sizeof(char) * (tag_length + 1)));

	if (tag == NULL)
	{
		napi_throw_error(env, NULL, "MetaCall could not load from memory, tag allocation failed");
		return NULL;
	}

	// Get tag
	status = napi_get_value_string_utf8(env, argv[0], tag, tag_length + 1, &tag_length);

	node_loader_impl_exception(env, status);

	// Get script length
	status = napi_get_value_string_utf8(env, argv[1], NULL, 0, &script_length);

	node_loader_impl_exception(env, status);

	script_size = script_length + 1;

	// Allocate script
	script = static_cast<char *>(malloc(sizeof(char) * script_size));

	if (script == NULL)
	{
		free(tag);
		napi_throw_error(env, NULL, "MetaCall could not load from memory, script allocation failed");
		return NULL;
	}

	// Get script
	status = napi_get_value_string_utf8(env, argv[1], script, script_size, &script_length);

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl("node");
	loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	void *handle = NULL;

	// Load script from memory
	if (metacall_load_from_memory(tag, script, script_size, &handle) != 0)
	{
		napi_throw_error(env, NULL, "MetaCall could not load from memory");
	}

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, NULL);

	free(tag);
	free(script);

	void *exports = metacall_handle_export(handle);

	napi_value v_exports = node_loader_impl_value_to_napi(node_impl, env, exports);

	node_loader_impl_finalizer(env, v_exports, exports);

	return v_exports;
}

/* TODO: Add documentation */
napi_value node_loader_port_inspect(napi_env env, napi_callback_info)
{
	napi_value result;

	size_t size = 0;

	struct metacall_allocator_std_type std_ctx = { &malloc, &realloc, &free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	char *inspect_str = metacall_inspect(&size, allocator);

	napi_status status;

	if (!(inspect_str != NULL && size != 0))
	{
		napi_throw_error(env, NULL, "Invalid MetaCall inspect string");
	}

	status = napi_create_string_utf8(env, inspect_str, size - 1, &result);

	node_loader_impl_exception(env, status);

	metacall_allocator_free(allocator, inspect_str);

	metacall_allocator_destroy(allocator);

	return result;
}

/* TODO: Add documentation */
napi_value node_loader_port_logs(napi_env env, napi_callback_info)
{
	struct metacall_log_stdio_type log_stdio = { stdout };

	if (metacall_log(METACALL_LOG_STDIO, (void *)&log_stdio) != 0)
	{
		napi_throw_error(env, NULL, "MetaCall failed to initialize debug logs");
	}

	return NULL;
}

/* TODO: Review documentation */
// This functions sets the necessary js functions that could be called in NodeJs
void node_loader_port_exports(napi_env env, napi_value exports)
{
	const char function_metacall_str[] = "metacall";
	const char function_metacall_await_str[] = "metacall_await";
	const char function_load_from_file_str[] = "metacall_load_from_file";
	const char function_load_from_file_export_str[] = "metacall_load_from_file_export";
	const char function_load_from_memory_str[] = "metacall_load_from_memory";
	const char function_load_from_memory_export_str[] = "metacall_load_from_memory_export";
	const char function_inspect_str[] = "metacall_inspect";
	const char function_logs_str[] = "metacall_logs";

	napi_value function_metacall, function_metacall_await, function_load_from_file, function_load_from_file_export, function_load_from_memory, function_load_from_memory_export, function_inspect, function_logs;

	napi_create_function(env, function_metacall_str, sizeof(function_metacall_str) - 1, node_loader_port_call, NULL, &function_metacall);
	napi_create_function(env, function_metacall_await_str, sizeof(function_metacall_await_str) - 1, node_loader_port_await, NULL, &function_metacall_await);
	napi_create_function(env, function_load_from_file_str, sizeof(function_load_from_file_str) - 1, node_loader_port_load_from_file, NULL, &function_load_from_file);
	napi_create_function(env, function_load_from_file_export_str, sizeof(function_load_from_file_export_str) - 1, node_loader_port_load_from_file_export, NULL, &function_load_from_file_export);
	napi_create_function(env, function_load_from_memory_str, sizeof(function_load_from_memory_str) - 1, node_loader_port_load_from_memory, NULL, &function_load_from_memory);
	napi_create_function(env, function_load_from_memory_export_str, sizeof(function_load_from_memory_export_str) - 1, node_loader_port_load_from_memory_export, NULL, &function_load_from_memory_export);
	napi_create_function(env, function_inspect_str, sizeof(function_inspect_str) - 1, node_loader_port_inspect, NULL, &function_inspect);
	napi_create_function(env, function_logs_str, sizeof(function_logs_str) - 1, node_loader_port_logs, NULL, &function_logs);

	napi_set_named_property(env, exports, function_metacall_str, function_metacall);
	napi_set_named_property(env, exports, function_metacall_await_str, function_metacall_await);
	napi_set_named_property(env, exports, function_load_from_file_str, function_load_from_file);
	napi_set_named_property(env, exports, function_load_from_file_export_str, function_load_from_file_export);
	napi_set_named_property(env, exports, function_load_from_memory_str, function_load_from_memory);
	napi_set_named_property(env, exports, function_load_from_memory_export_str, function_load_from_memory_export);
	napi_set_named_property(env, exports, function_inspect_str, function_inspect);
	napi_set_named_property(env, exports, function_logs_str, function_logs);
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
		napi_throw_error(env, NULL, "MetaCall failed to initialize");

		return NULL;
	}
#endif

	node_loader_port_exports(env, exports);

	return exports;
}
