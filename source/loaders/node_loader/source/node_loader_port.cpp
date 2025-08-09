/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <set>

#include <node_api.h>

static const loader_tag node_loader_tag = "node";
static std::set<value> metacall_value_reference_pointers;

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
		delete[] argv;
		delete[] args;
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

napi_value node_loader_port_metacallfms(napi_env env, napi_callback_info info)
{
	size_t argc = 0;

	napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);

	if (argc != 2)
	{
		napi_throw_error(env, nullptr, "Invalid number of arguments");
		return nullptr;
	}

	napi_value *argv = new napi_value[argc];
	napi_value recv;

	napi_get_cb_info(env, info, &argc, argv, &recv, nullptr);

	size_t name_length;
	napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &name_length);

	char *name = new char[name_length + 1];

	if (name == nullptr)
	{
		napi_throw_error(env, nullptr, "Invalid function name allocation");
		delete[] argv;
		return nullptr;
	}

	status = napi_get_value_string_utf8(env, argv[0], name, name_length + 1, &name_length);

	name[name_length] = '\0';

	node_loader_impl_exception(env, status);

	void *func = metacall_function(name);

	if (func == NULL)
	{
		napi_throw_error(env, nullptr, "The function does not exist");
		delete[] argv;
		delete[] name;
		return nullptr;
	}

	size_t buffer_length;
	status = napi_get_value_string_utf8(env, argv[1], nullptr, 0, &buffer_length);

	char *buffer = new char[buffer_length + 1];

	if (buffer == nullptr)
	{
		napi_throw_error(env, nullptr, "Invalid function buffer allocation");
		delete[] argv;
		delete[] name;
		return nullptr;
	}

	status = napi_get_value_string_utf8(env, argv[1], buffer, buffer_length + 1, &buffer_length);

	buffer[buffer_length] = '\0';

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl(node_loader_tag);
	loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	/* Call to the function */
	void *ret = metacallfms(func, buffer, buffer_length + 1, allocator);

	metacall_allocator_destroy(allocator);

	napi_value result = node_loader_impl_value_to_napi(node_impl, env, ret);

	if (metacall_value_id(ret) == METACALL_THROWABLE)
	{
		napi_throw(env, result);
	}

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, nullptr);

	metacall_value_destroy(ret);

	delete[] argv;
	delete[] name;
	delete[] buffer;

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
		delete[] argv;
		delete[] args;
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

/**
*  @brief
*    Define an execution path into a runtime
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
napi_value node_loader_port_metacall_execution_path(napi_env env, napi_callback_info info)
{
	const size_t args_size = 2;
	size_t argc = args_size, tag_length, path_length;
	napi_value argv[args_size];

	/* Get arguments */
	napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

	node_loader_impl_exception(env, status);

	/* Get tag length */
	status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &tag_length);

	node_loader_impl_exception(env, status);

	/* Allocate tag */
	char *tag = new char[tag_length + 1];

	if (tag == nullptr)
	{
		napi_throw_error(env, nullptr, "MetaCall could not define an execution path, tag allocation failed");
		return nullptr;
	}

	/* Get tag */
	status = napi_get_value_string_utf8(env, argv[0], tag, tag_length + 1, &tag_length);

	node_loader_impl_exception(env, status);

	/* Get path length */
	status = napi_get_value_string_utf8(env, argv[1], nullptr, 0, &path_length);

	node_loader_impl_exception(env, status);

	size_t path_size = path_length + 1;

	/* Allocate path */
	char *path = new char[path_size];

	if (path == nullptr)
	{
		napi_throw_error(env, nullptr, "MetaCall could not define an execution path, path allocation failed");
		delete[] tag;
		return nullptr;
	}

	/* Get path */
	status = napi_get_value_string_utf8(env, argv[1], path, path_size, &path_length);

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl(node_loader_tag);
	loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	/* Define execution path */
	if (metacall_execution_path(tag, path) != 0)
	{
		napi_throw_error(env, nullptr, "MetaCall could not define an execution path");
	}

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, nullptr);

	delete[] tag;
	delete[] path;

	/* TODO: Return value and logs */
	return nullptr;
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
		napi_throw_error(env, nullptr, "MetaCall could not load from memory, script allocation failed");
		delete[] tag;
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
		napi_throw_error(env, nullptr, "MetaCall could not load from memory, script allocation failed");
		delete[] tag;
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
*    Load a package by tag
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
napi_value node_loader_port_metacall_load_from_package(napi_env env, napi_callback_info info)
{
	const size_t args_size = 2;
	size_t argc = args_size, tag_length, package_length;
	napi_value argv[args_size];

	/* Get arguments */
	napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

	node_loader_impl_exception(env, status);

	/* Get tag length */
	status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &tag_length);

	node_loader_impl_exception(env, status);

	/* Allocate tag */
	char *tag = new char[tag_length + 1];

	if (tag == nullptr)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load from package, tag allocation failed");
		return nullptr;
	}

	/* Get tag */
	status = napi_get_value_string_utf8(env, argv[0], tag, tag_length + 1, &tag_length);

	node_loader_impl_exception(env, status);

	/* Get package length */
	status = napi_get_value_string_utf8(env, argv[1], nullptr, 0, &package_length);

	node_loader_impl_exception(env, status);

	size_t package_size = package_length + 1;

	/* Allocate package */
	char *package = new char[package_size];

	if (package == nullptr)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load from package, package allocation failed");
		delete[] tag;
		return nullptr;
	}

	/* Get package */
	status = napi_get_value_string_utf8(env, argv[1], package, package_size, &package_length);

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl(node_loader_tag);
	loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	/* Load the package */
	if (metacall_load_from_package(tag, package, NULL) != 0)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load a package");
	}

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, nullptr);

	delete[] tag;
	delete[] package;

	/* TODO: Return value and logs */
	return nullptr;
}

napi_value node_loader_port_metacall_load_from_package_export(napi_env env, napi_callback_info info)
{
	const size_t args_size = 2;
	size_t argc = args_size, tag_length, package_length;
	napi_value argv[args_size];

	/* Get arguments */
	napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

	node_loader_impl_exception(env, status);

	/* Get tag length */
	status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &tag_length);

	node_loader_impl_exception(env, status);

	/* Allocate tag */
	char *tag = new char[tag_length + 1];

	if (tag == nullptr)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load from memory, tag allocation failed");
		return nullptr;
	}

	/* Get tag */
	status = napi_get_value_string_utf8(env, argv[0], tag, tag_length + 1, &tag_length);

	node_loader_impl_exception(env, status);

	/* Get package length */
	status = napi_get_value_string_utf8(env, argv[1], nullptr, 0, &package_length);

	node_loader_impl_exception(env, status);

	size_t package_size = package_length + 1;

	/* Allocate package */
	char *package = new char[package_size];

	if (package == nullptr)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load from package, package allocation failed");
		delete[] tag;
		return nullptr;
	}

	/* Get package */
	status = napi_get_value_string_utf8(env, argv[1], package, package_size, &package_length);

	node_loader_impl_exception(env, status);

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl(node_loader_tag);
	loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	void *handle = NULL;

	/* Load package from package */
	if (metacall_load_from_package(tag, package, &handle) != 0)
	{
		napi_throw_error(env, nullptr, "MetaCall could not load from package");
	}

	/* Release current reference of the environment */
	// node_loader_impl_env(node_impl, nullptr);

	delete[] tag;
	delete[] package;

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

napi_value node_loader_port_metacall_logs(napi_env env, napi_callback_info)
{
	struct metacall_log_stdio_type log_stdio = { stdout };

	if (metacall_log(METACALL_LOG_STDIO, static_cast<void *>(&log_stdio)) != 0)
	{
		napi_throw_error(env, nullptr, "MetaCall failed to initialize debug logs");
	}

	return nullptr;
}

/* TODO: Add documentation */
napi_value node_loader_port_metacall_value_create_ptr(napi_env env, napi_callback_info info)
{
	const size_t args_size = 1;
	size_t argc = args_size;
	napi_value argv[args_size];
	void *v;

	// Get arguments
	napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

	node_loader_impl_exception(env, status);

	if (argc == 0)
	{
		v = NULL;
	}
	else if (argc == 1)
	{
		napi_valuetype arg_type;

		status = napi_typeof(env, argv[0], &arg_type);

		node_loader_impl_exception(env, status);

		if (arg_type == napi_undefined)
		{
			v = NULL;
		}
		else if (arg_type == napi_external)
		{
			// Copy the external pointer
			status = napi_get_value_external(env, argv[0], &v);

			node_loader_impl_exception(env, status);
		}
		else
		{
			napi_throw_type_error(env, nullptr, "Invalid MetaCall value create pointer, you need to pass undefined or external as a parameter");
		}
	}
	else
	{
		napi_throw_error(env, nullptr, "Invalid MetaCall value create pointer, you need to pass 0 or 1 parameters");
	}

	napi_value result_external;

	status = napi_create_external(env, &v, NULL, NULL, &result_external);

	node_loader_impl_exception(env, status);

	return result_external;
}

static void metacall_value_reference_finalize(napi_env env, void *finalize_data, void *finalize_hint)
{
	value v = finalize_data;
	(void)env;
	(void)finalize_hint;
	metacall_value_destroy(v);
	metacall_value_reference_pointers.erase(v);
}

/* TODO: Add documentation */
napi_value node_loader_port_metacall_value_reference(napi_env env, napi_callback_info info)
{
	const size_t args_size = 1;
	size_t argc = args_size;
	napi_value recv;
	napi_value argv[args_size];
	value v;

	// Get arguments
	napi_status status = napi_get_cb_info(env, info, &argc, argv, &recv, nullptr);

	node_loader_impl_exception(env, status);

	if (argc != 1)
	{
		napi_throw_type_error(env, NULL, "Invalid number of arguments, use it like: metacall_value_reference(obj);");
		return nullptr;
	}

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl(node_loader_tag);
	loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	v = node_loader_impl_napi_to_value(node_impl, env, recv, argv[0]);

	if (v == NULL)
	{
		napi_throw_error(env, NULL, "Failed to convert the JavaScript object to MetaCall value.");
		return nullptr;
	}

	napi_value result_external;

	status = napi_create_external(env, v, &metacall_value_reference_finalize, nullptr, &result_external);

	node_loader_impl_exception(env, status);

	metacall_value_reference_pointers.insert(v);

	return result_external;
}

/* TODO: Add documentation */
napi_value node_loader_port_metacall_value_dereference(napi_env env, napi_callback_info info)
{
	const size_t args_size = 1;
	size_t argc = args_size;
	napi_value recv;
	napi_value argv[args_size];
	value v;

	// Get arguments
	napi_status status = napi_get_cb_info(env, info, &argc, argv, &recv, nullptr);

	node_loader_impl_exception(env, status);

	if (argc != 1)
	{
		napi_throw_type_error(env, NULL, "Invalid number of arguments, use it like: metacall_value_dereference(ptr);");
		return nullptr;
	}

	napi_valuetype type;

	status = napi_typeof(env, argv[0], &type);
	node_loader_impl_exception(env, status);

	if (type != napi_external)
	{
		napi_throw_type_error(env, NULL, "Invalid parameter type in first argument must be a PyCapsule (i.e a previously allocated pointer)");
		return NULL;
	}

	// Get the external pointer
	status = napi_get_value_external(env, argv[0], &v);

	node_loader_impl_exception(env, status);

	// If it is not contained in the set, it is not a valid value
	if (metacall_value_reference_pointers.find(v) == metacall_value_reference_pointers.end())
	{
		napi_throw_type_error(env, NULL, "Invalid reference, argument must be a PyCapsule containing a MetaCall value, use it only with values returned by metacall_value_reference");
		return NULL;
	}

	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl(node_loader_tag);
	loader_impl_node node_impl = (loader_impl_node)loader_impl_get(impl);

	/* Store current reference of the environment */
	node_loader_impl_env(node_impl, env);

	/* Get the N-API value */
	napi_value result = node_loader_impl_value_to_napi(node_impl, env, v);

	return result;
}

napi_value node_loader_port_register_bootstrap_startup(napi_env env, napi_callback_info)
{
	/* Obtain NodeJS loader implementation */
	loader_impl impl = loader_get_impl(node_loader_tag);
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	/* Define environment, required to initialize the runtime properly */
	node_loader_impl_env(node_impl, env);

	/* Return all the values required for the bootstrap startup */
	return node_loader_impl_register_bootstrap_startup(node_impl, env);
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
	x(metacallfms); \
	x(metacall_await); \
	x(metacall_execution_path); \
	x(metacall_load_from_file); \
	x(metacall_load_from_file_export); \
	x(metacall_load_from_memory); \
	x(metacall_load_from_memory_export); \
	x(metacall_load_from_package); \
	x(metacall_load_from_package_export); \
	x(metacall_load_from_configuration); \
	x(metacall_load_from_configuration_export); \
	x(metacall_inspect); \
	x(metacall_value_create_ptr); \
	x(metacall_value_reference); \
	x(metacall_value_dereference); \
	x(metacall_logs); \
	x(register_bootstrap_startup);

	/* Declare all the functions */
	NODE_LOADER_PORT_DECL_X_MACRO(NODE_LOADER_PORT_DECL_FUNC)

#undef NODE_LOADER_PORT_DECL_FUNC
#undef NODE_LOADER_PORT_DECL_X_MACRO
}

/* TODO: Review documentation */
/* This function is called by NodeJs when the module is required */
napi_value node_loader_port_initialize(napi_env env, napi_value exports)
{
	node_loader_port_exports(env, exports);

	/* Unregister NAPI Hook */
	if (metacall_link_unregister(node_loader_tag, "node", "napi_register_module_v1") != 0)
	{
		// TODO: Handle error
	}

	return exports;
}
