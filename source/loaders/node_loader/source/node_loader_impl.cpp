/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading nodejs code at run-time into a process.
 *
 */

#if defined(WIN32) || defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <io.h>
#	ifndef dup
#		define dup _dup
#	endif
#	ifndef dup2
#		define dup2 _dup2
#	endif
#	ifndef STDIN_FILENO
#		define STDIN_FILENO _fileno(stdin)
#	endif
#	ifndef STDOUT_FILENO
#		define STDOUT_FILENO _fileno(stdout)
#	endif
#	ifndef STDERR_FILENO
#		define STDERR_FILENO _fileno(stderr)
#	endif
#else
#	include <unistd.h>
#endif

#include <node_loader/node_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

/* TODO: Make logs thread safe */
#include <log/log.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>

#include <new>
#include <string>
#include <fstream>
#include <streambuf>

#include <libplatform/libplatform.h>
#include <v8.h> /* version: 6.2.414.50 */

#ifdef ENABLE_DEBUGGER_SUPPORT
#	include <v8-debug.h>
#endif /* ENALBLE_DEBUGGER_SUPPORT */

#include <uv.h>

#include <node.h>
#include <node_api.h>

/* TODO:
	To solve the deadlock we have to make MetaCall fork tolerant.
	The problem is that when Linux makes a fork it uses fork-one strategy, this means
	that only the caller thread is cloned, the others are not, so the NodeJS thread pool
	does not survive. When the thread pool tries to continue it blocks the whole application.
	To solve this, we have to:
		- Change all mutex and conditions by a binary sempahore.
		- Move all calls to MetaCall in async functions to outside of the async methods, passing result data in
			the async data structure (async_object.data) because MetaCall is not thread safe and it can induce
			other threads to overwrite data improperly.
		- Make a detour to function fork. Intercept the function fork to shutdown all runtimes in the parent
			and then re-initialize all of them in parent and child after the fork. pthread_atfork is not sufficient
			because it is a bug on the POSIX standard (too many limitations are related to this technique).
*/

typedef struct loader_impl_node_type
{
	napi_env env;
	napi_ref global_ref;
	napi_ref function_table_object_ref;

	uv_thread_t thread_id;
	uv_loop_t * thread_loop;

	uv_async_t async_initialize;
	uv_async_t async_load_from_file;
	uv_async_t async_clear;
	uv_async_t async_discover;
	uv_async_t async_func_call;
	uv_async_t async_func_destroy;
	uv_async_t async_destroy;

	int result;

} * loader_impl_node;

typedef struct loader_impl_node_function_type
{
	loader_impl_node node_impl;
	napi_ref func_ref;
	napi_value * argv;

} * loader_impl_node_function;

typedef struct loader_impl_async_load_from_file_type
{
	loader_impl_node node_impl;
	const loader_naming_path * paths;
	size_t size;
	napi_ref handle_ref;

} * loader_impl_async_load_from_file;

typedef struct loader_impl_async_clear_type
{
	loader_impl_node node_impl;
	napi_ref handle_ref;

} * loader_impl_async_clear;

typedef struct loader_impl_async_discover_type
{
	loader_impl_node node_impl;
	napi_ref handle_ref;
	context ctx;

} * loader_impl_async_discover;

typedef struct loader_impl_async_func_call_type
{
	loader_impl_node node_impl;
	function func;
	loader_impl_node_function node_func;
	void ** args;
	function_return ret;

} * loader_impl_async_func_call;

typedef struct loader_impl_async_func_destroy_type
{
	loader_impl_node node_impl;
	loader_impl_node_function node_func;

} * loader_impl_async_func_destroy;

static uv_mutex_t node_impl_mutex;
static uv_cond_t node_impl_cond;

/* Function */
int function_node_interface_create(function func, function_impl impl);

function_return function_node_interface_invoke(function func, function_impl impl, function_args args);

void function_node_interface_destroy(function func, function_impl impl);

function_interface function_node_singleton(void);

/* Async */
void node_loader_impl_async_initialize(uv_async_t * async);

void node_loader_impl_async_func_call(uv_async_t * async);

void node_loader_impl_async_func_destroy(uv_async_t * async);

void node_loader_impl_async_load_from_file(uv_async_t * async);

void node_loader_impl_async_clear(uv_async_t * async);

void node_loader_impl_async_discover(uv_async_t * async);

void node_loader_impl_async_destroy(uv_async_t * async);

/* Loader */
void * node_loader_impl_register(void * node_impl_ptr, void * env_ptr, void * function_table_object_ptr);

void node_loader_impl_thread(void * data);

void node_loader_impl_walk(uv_handle_t * handle, void * data);

/* Standard Input Output descriptors */
static int stdin_copy, stdout_copy, stderr_copy;

int function_node_interface_create(function func, function_impl impl)
{
	loader_impl_node_function node_func = (loader_impl_node_function)impl;

	signature s = function_signature(func);

	const size_t args_size = signature_count(s);

	node_func->argv = static_cast<napi_value *>(malloc(sizeof(napi_value) * args_size));

	return (node_func->argv == NULL);
}

function_return function_node_interface_invoke(function func, function_impl impl, function_args args)
{
	loader_impl_node_function node_func = (loader_impl_node_function)impl;

	if (node_func != NULL)
	{
		loader_impl_node node_impl = node_func->node_impl;

		struct loader_impl_async_func_call_type async_data =
		{
			node_impl,
			func,
			node_func,
			static_cast<void **>(args),
			NULL
		};

		node_impl->async_func_call.data = static_cast<void *>(&async_data);

		/* Execute function call async callback */
		uv_async_send(&node_impl->async_func_call);

		/* Wait until function is called */
		uv_mutex_lock(&node_impl_mutex);

		uv_cond_wait(&node_impl_cond, &node_impl_mutex);

		uv_mutex_unlock(&node_impl_mutex);

		return async_data.ret;
	}

	return NULL;
}

void function_node_interface_destroy(function func, function_impl impl)
{
	loader_impl_node_function node_func = (loader_impl_node_function)impl;

	(void)func;

	if (node_func != NULL)
	{
		loader_impl_node node_impl = node_func->node_impl;

		struct loader_impl_async_func_destroy_type async_data =
		{
			node_impl,
			node_func
		};

		node_impl->async_func_destroy.data = static_cast<void *>(&async_data);

		/* Execute function destroy async callback */
		uv_async_send(&node_impl->async_func_destroy);

		/* Wait until function is destroyed */
		uv_mutex_lock(&node_impl_mutex);

		uv_cond_wait(&node_impl_cond, &node_impl_mutex);

		uv_mutex_unlock(&node_impl_mutex);

		free(node_func->argv);

		free(node_func);
	}
}

function_interface function_node_singleton()
{
	static struct function_interface_type node_function_interface =
	{
		&function_node_interface_create,
		&function_node_interface_invoke,
		&function_node_interface_destroy
	};

	return &node_function_interface;
}

void node_loader_impl_async_initialize(uv_async_t * async)
{
	loader_impl_node node_impl;

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl_mutex);

	node_impl = *(static_cast<loader_impl_node *>(async->data));

	(void)node_impl;

	/* Signal start condition */
	uv_cond_signal(&node_impl_cond);

	uv_mutex_unlock(&node_impl_mutex);
}

napi_value node_loader_impl_value(napi_env env, void * arg)
{
	value arg_value = static_cast<value>(arg);

	type_id id = value_type_id(arg_value);

	napi_status status;

	napi_value v;

	if (id == TYPE_BOOL)
	{
		boolean bool_value = value_to_bool(arg_value);

		status = napi_get_boolean(env, (bool_value == 0) ? false : true, &v);

		assert(status == napi_ok);
	}
	else if (id == TYPE_CHAR)
	{
		char char_value = value_to_char(arg_value);

		status = napi_create_int32(env, static_cast<int32_t>(char_value), &v);

		assert(status == napi_ok);
	}
	else if (id == TYPE_SHORT)
	{
		short short_value = value_to_short(arg_value);

		status = napi_create_int32(env, static_cast<int32_t>(short_value), &v);

		assert(status == napi_ok);
	}
	else if (id == TYPE_INT)
	{
		int int_value = value_to_int(arg_value);

		/* TODO: Check integer overflow */
		status = napi_create_int32(env, static_cast<int32_t>(int_value), &v);

		assert(status == napi_ok);
	}
	else if (id == TYPE_LONG)
	{
		long long_value = value_to_long(arg_value);

		/* TODO: Check integer overflow */
		status = napi_create_int64(env, static_cast<int64_t>(long_value), &v);

		assert(status == napi_ok);
	}
	else if (id == TYPE_FLOAT)
	{
		float float_value = value_to_float(arg_value);

		status = napi_create_double(env, static_cast<double>(float_value), &v);

		assert(status == napi_ok);
	}
	else if (id == TYPE_DOUBLE)
	{
		double double_value = value_to_double(arg_value);

		status = napi_create_double(env, double_value, &v);

		assert(status == napi_ok);
	}
	else if (id == TYPE_STRING)
	{
		const char * str_value = value_to_string(arg_value);

		size_t length = value_type_size(arg_value) - 1;

		status = napi_create_string_utf8(env, str_value, length, &v);

		assert(status == napi_ok);
	}
	else if (id == TYPE_BUFFER)
	{
		void * buff_value = value_to_buffer(arg_value);

		size_t size = value_type_size(arg_value);

		status = napi_create_buffer(env, size, &buff_value, &v);

		assert(status == napi_ok);
	}
	else if (id == TYPE_ARRAY)
	{
		value * array_value = value_to_array(arg_value);

		size_t array_size = value_type_size(arg_value) / sizeof(const value);

		uint32_t iterator;

		status = napi_create_array_with_length(env, array_size, &v);

		assert(status == napi_ok);

		for (iterator = 0; iterator < array_size; ++iterator)
		{
			/* TODO: Review recursion overflow */
			napi_value element_v = node_loader_impl_value(env, static_cast<void *>(array_value[iterator]));

			status = napi_set_element(env, v, iterator, element_v);

			assert(status == napi_ok);
		}
	}
	else if (id == TYPE_MAP)
	{
		value * map_value = value_to_map(arg_value);

		size_t iterator, map_size = value_type_size(arg_value) / sizeof(const value);

		status = napi_create_object(env, &v);

		assert(status == napi_ok);

		for (iterator = 0; iterator < map_size; ++iterator)
		{
			value * pair_value = value_to_array(map_value[iterator]);

			const char * key = value_to_string(pair_value[0]);

			/* TODO: Review recursion overflow */
			napi_value element_v = node_loader_impl_value(env, static_cast<void *>(pair_value[1]));

			status = napi_set_named_property(env, v, key, element_v);

			assert(status == napi_ok);
		}
	}
	/* TODO */
	/*
	else if (id == TYPE_PTR)
	{

	}
	*/
	else
	{
		status = napi_get_undefined(env, &v);

		assert(status == napi_ok);
	}

	return v;
}

function_return node_loader_impl_return(napi_env env, napi_value v)
{
	value ret = NULL;

	napi_valuetype valuetype;

	napi_status status = napi_typeof(env, v, &valuetype);

	assert(status == napi_ok);

	if (valuetype == napi_undefined)
	{
		/* TODO */
	}
	else if (valuetype == napi_null)
	{
		/* TODO */
	}
	else if (valuetype == napi_boolean)
	{
		bool b;

		status = napi_get_value_bool(env, v, &b);

		assert(status == napi_ok);

		ret = value_create_bool((b == true) ? static_cast<boolean>(1) : static_cast<boolean>(0));
	}
	else if (valuetype == napi_number)
	{
		double d;

		status = napi_get_value_double(env, v, &d);

		assert(status == napi_ok);

		ret = value_create_double(d);
	}
	else if (valuetype == napi_string)
	{
		size_t length;

		status = napi_get_value_string_utf8(env, v, NULL, 0, &length);

		assert(status == napi_ok);

		ret = value_create_string(NULL, length);

		if (ret != NULL)
		{
			char * str = value_to_string(ret);

			status = napi_get_value_string_utf8(env, v, str, length + 1, &length);

			assert(status == napi_ok);
		}
	}
	else if (valuetype == napi_symbol)
	{
		/* TODO */
	}
	else if (valuetype == napi_object)
	{
		bool result = false;

		if (napi_is_array(env, v, &result) == napi_ok && result == true)
		{
			uint32_t iterator, length = 0;

			value * array_value;

			status = napi_get_array_length(env, v, &length);

			assert(status == napi_ok);

			ret = value_create_array(NULL, static_cast<size_t>(length));

			array_value = value_to_array(ret);

			for (iterator = 0; iterator < length; ++iterator)
			{
				napi_value element;

				status = napi_get_element(env, v, iterator, &element);

				assert(status == napi_ok);

				/* TODO: Review recursion overflow */
				array_value[iterator] = node_loader_impl_return(env, element);
			}
		}
		else if (napi_is_buffer(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
		}
		else if (napi_is_error(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
		}
		else if (napi_is_typedarray(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
		}
		else if (napi_is_dataview(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
		}
		else
		{
			/* TODO: Strict check if it is an object (map) */
			uint32_t iterator, length = 0;

			napi_value keys;

			value * map_value;

			status = napi_get_property_names(env, v, &keys);

			assert(status == napi_ok);

			status = napi_get_array_length(env, keys, &length);

			assert(status == napi_ok);

			ret = value_create_map(NULL, static_cast<size_t>(length));

			map_value = value_to_map(ret);

			for (iterator = 0; iterator < length; ++iterator)
			{
				napi_value key;

				size_t key_length;

				value * tupla;

				/* Create tupla */
				map_value[iterator] = value_create_array(NULL, 2);

				tupla = value_to_array(map_value[iterator]);

				/* Get key from object */
				status = napi_get_element(env, keys, iterator, &key);

				assert(status == napi_ok);

				/* Set key string in the tupla */
				status = napi_get_value_string_utf8(env, key, NULL, 0, &key_length);

				assert(status == napi_ok);

				tupla[0] = value_create_string(NULL, key_length);

				if (tupla[0] != NULL)
				{
					napi_value element;

					char * str = value_to_string(tupla[0]);

					status = napi_get_value_string_utf8(env, key, str, key_length + 1, &key_length);

					assert(status == napi_ok);

					status = napi_get_property(env, v, key, &element);

					assert(status == napi_ok);

					/* TODO: Review recursion overflow */
					tupla[1] = node_loader_impl_return(env, element);
				}
			}

		}
	}
	else if (valuetype == napi_function)
	{
		/* TODO */
	}
	else if (valuetype == napi_external)
	{
		/* TODO */
	}

	return static_cast<function_return>(ret);
}

void node_loader_impl_async_func_call(uv_async_t * async)
{
	loader_impl_async_func_call async_data;

	napi_env env;

	napi_handle_scope handle_scope;

	signature s;

	size_t args_size;

	void ** args;

	loader_impl_node_function node_func;

	size_t args_count;

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl_mutex);

	async_data = static_cast<loader_impl_async_func_call>(async->data);

	/* Get environment reference */
	env = async_data->node_impl->env;

	/* Get function data */
	s = function_signature(async_data->func);

	args_size = signature_count(s);

	args = static_cast<void **>(async_data->args);

	node_func = async_data->node_func;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	assert(status == napi_ok);

	/* Build parameters */
	for (args_count = 0; args_count < args_size; ++args_count)
	{
		/* Define parameter */
		node_func->argv[args_count] = node_loader_impl_value(env, args[args_count]);
	}

	/* Get function reference */
	napi_value function_ptr;

	status = napi_get_reference_value(env, node_func->func_ref, &function_ptr);

	assert(status == napi_ok);

	/* Call to function */
	napi_value global, func_return;

	status = napi_get_reference_value(env, async_data->node_impl->global_ref, &global);

	assert(status == napi_ok);

	status = napi_call_function(env, global, function_ptr, args_size, node_func->argv, &func_return);

	assert(status == napi_ok);

	/* Convert function return to value */
	async_data->ret = node_loader_impl_return(env, func_return);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	assert(status == napi_ok);

	/* Signal function call condition */
	uv_cond_signal(&node_impl_cond);

	uv_mutex_unlock(&node_impl_mutex);
}

void node_loader_impl_async_func_destroy(uv_async_t * async)
{
	loader_impl_async_func_destroy async_data;

	napi_env env;

	uint32_t ref_count = 0;

	napi_handle_scope handle_scope;

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl_mutex);

	async_data = static_cast<loader_impl_async_func_destroy>(async->data);

	/* Get environment reference */
	env = async_data->node_impl->env;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	assert(status == napi_ok);

	/* Clear function persistent reference */
	status = napi_reference_unref(env, async_data->node_func->func_ref, &ref_count);

	assert(status == napi_ok && ref_count == 0);

	status = napi_delete_reference(env, async_data->node_func->func_ref);

	assert(status == napi_ok);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	assert(status == napi_ok);

	/* Signal function destroy condition */
	uv_cond_signal(&node_impl_cond);

	uv_mutex_unlock(&node_impl_mutex);
}

void node_loader_impl_async_load_from_file(uv_async_t * async)
{
	loader_impl_async_load_from_file async_data;

	napi_env env;
	napi_value function_table_object;

	const char load_from_file_str[] = "load_from_file";
	napi_value load_from_file_str_value;

	bool result = false;

	napi_handle_scope handle_scope;

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl_mutex);

	async_data = static_cast<loader_impl_async_load_from_file>(async->data);

	/* Get environment reference */
	env = async_data->node_impl->env;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	assert(status == napi_ok);

	/* Get function table object from reference */
	status = napi_get_reference_value(env, async_data->node_impl->function_table_object_ref, &function_table_object);

	assert(status == napi_ok);

	/* Create function string */
	status = napi_create_string_utf8(env, load_from_file_str, sizeof(load_from_file_str) - 1, &load_from_file_str_value);

	assert(status == napi_ok);

	/* Check if exists in the table */
	status = napi_has_own_property(env, function_table_object, load_from_file_str_value, &result);

	assert(status == napi_ok);

	if (result == true)
	{
		napi_value function_trampoline_load_from_file;
		napi_valuetype valuetype;
		napi_value argv[1];
		uint32_t ref_count = 0;

		status = napi_get_named_property(env, function_table_object, load_from_file_str, &function_trampoline_load_from_file);

		assert(status == napi_ok);

		status = napi_typeof(env, function_trampoline_load_from_file, &valuetype);

		assert(status == napi_ok);

		if (valuetype != napi_function)
		{
			napi_throw_type_error(env, nullptr, "Invalid function load_from_file in function table object");
		}

		/* Define parameters */
		status = napi_create_array_with_length(env, async_data->size, &argv[0]);

		assert(status == napi_ok);

		for (size_t index = 0; index < async_data->size; ++index)
		{
			napi_value path_str;

			size_t length = strnlen(async_data->paths[index], LOADER_NAMING_PATH_SIZE);

			status = napi_create_string_utf8(env, async_data->paths[index], length, &path_str);

			assert(status == napi_ok);

			status = napi_set_element(env, argv[0], (uint32_t)index, path_str);

			assert(status == napi_ok);
		}

		/* Call to load from file function */
		napi_value global, return_value;

		status = napi_get_reference_value(env, async_data->node_impl->global_ref, &global);

		assert(status == napi_ok);

		status = napi_call_function(env, global, function_trampoline_load_from_file, 1, argv, &return_value);

		assert(status == napi_ok);

		/* Make handle persistent */
		status = napi_create_reference(env, return_value, 0, &async_data->handle_ref);

		assert(status == napi_ok);

		status = napi_reference_ref(env, async_data->handle_ref, &ref_count);

		assert(status == napi_ok && ref_count == 1);
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	assert(status == napi_ok);

	/* Signal load from file condition */
	uv_cond_signal(&node_impl_cond);

	uv_mutex_unlock(&node_impl_mutex);
}

void node_loader_impl_async_clear(uv_async_t * async)
{
	loader_impl_async_clear async_data;

	napi_env env;
	napi_value function_table_object;

	const char clear_str[] = "clear";
	napi_value clear_str_value;

	bool result = false;

	napi_handle_scope handle_scope;

	uint32_t ref_count = 0;

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl_mutex);

	async_data = static_cast<loader_impl_async_clear>(async->data);

	/* Get environment reference */
	env = async_data->node_impl->env;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	assert(status == napi_ok);
	/* Get function table object from reference */
	status = napi_get_reference_value(env, async_data->node_impl->function_table_object_ref, &function_table_object);

	assert(status == napi_ok);

	/* Create function string */
	status = napi_create_string_utf8(env, clear_str, sizeof(clear_str) - 1, &clear_str_value);

	assert(status == napi_ok);

	/* Check if exists in the table */
	status = napi_has_own_property(env, function_table_object, clear_str_value, &result);

	assert(status == napi_ok);

	if (result == true)
	{
		napi_value function_trampoline_clear;
		napi_valuetype valuetype;
		napi_value argv[1];

		status = napi_get_named_property(env, function_table_object, clear_str, &function_trampoline_clear);

		assert(status == napi_ok);

		status = napi_typeof(env, function_trampoline_clear, &valuetype);

		assert(status == napi_ok);

		if (valuetype != napi_function)
		{
			napi_throw_type_error(env, nullptr, "Invalid function clear in function table object");
		}

		/* Define parameters */
		status = napi_get_reference_value(env, async_data->handle_ref, &argv[0]);

		assert(status == napi_ok);

		/* Call to load from file function */
		napi_value global, clear_return;

		status = napi_get_reference_value(env, async_data->node_impl->global_ref, &global);

		assert(status == napi_ok);

		status = napi_call_function(env, global, function_trampoline_clear, 1, argv, &clear_return);

		assert(status == napi_ok);
	}

	/* Clear handle persistent reference */
	status = napi_reference_unref(env, async_data->handle_ref, &ref_count);

	assert(status == napi_ok && ref_count == 0);

	status = napi_delete_reference(env, async_data->handle_ref);

	assert(status == napi_ok);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	assert(status == napi_ok);

	/* Signal clear condition */
	uv_cond_signal(&node_impl_cond);

	uv_mutex_unlock(&node_impl_mutex);
}

void node_loader_impl_async_discover(uv_async_t * async)
{
	loader_impl_async_discover async_data;

	napi_env env;
	napi_value function_table_object;

	const char discover_str[] = "discover";
	napi_value discover_str_value;

	bool result = false;

	napi_handle_scope handle_scope;

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl_mutex);

	async_data = static_cast<loader_impl_async_discover>(async->data);

	/* Get environment reference */
	env = async_data->node_impl->env;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	assert(status == napi_ok);

	/* Get function table object from reference */
	status = napi_get_reference_value(env, async_data->node_impl->function_table_object_ref, &function_table_object);

	assert(status == napi_ok);

	/* Create function string */
	status = napi_create_string_utf8(env, discover_str, sizeof(discover_str) - 1, &discover_str_value);

	assert(status == napi_ok);

	/* Check if exists in the table */
	status = napi_has_own_property(env, function_table_object, discover_str_value, &result);

	assert(status == napi_ok);

	if (result == true)
	{
		napi_value function_trampoline_discover;
		napi_valuetype valuetype;
		napi_value argv[1];

		status = napi_get_named_property(env, function_table_object, discover_str, &function_trampoline_discover);

		assert(status == napi_ok);

		status = napi_typeof(env, function_trampoline_discover, &valuetype);

		assert(status == napi_ok);

		if (valuetype != napi_function)
		{
			napi_throw_type_error(env, nullptr, "Invalid function discover in function table object");
		}

		/* Define parameters */
		status = napi_get_reference_value(env, async_data->handle_ref, &argv[0]);

		assert(status == napi_ok);

		/* Call to load from file function */
		napi_value global, discover_map;

		status = napi_get_reference_value(env, async_data->node_impl->global_ref, &global);

		assert(status == napi_ok);

		status = napi_call_function(env, global, function_trampoline_discover, 1, argv, &discover_map);

		assert(status == napi_ok);

		/* Convert return value (discover object) to context */
		napi_value function_names;
		uint32_t function_names_length;

		status = napi_get_property_names(env, discover_map, &function_names);

		assert(status == napi_ok);

		status = napi_get_array_length(env, function_names, &function_names_length);

		assert(status == napi_ok);

		for (uint32_t index = 0; index < function_names_length; ++index)
		{
			napi_value function_name;
			size_t function_name_length;
			char * function_name_str = NULL;

			status = napi_get_element(env, function_names, index, &function_name);

			assert(status == napi_ok);

			status = napi_get_value_string_utf8(env, function_name, NULL, 0, &function_name_length);

			assert(status == napi_ok);

			if (function_name_length > 0)
			{
				function_name_str = static_cast<char *>(malloc(sizeof(char) * (function_name_length + 1)));
			}

			if (function_name_str != NULL)
			{
				napi_value function_descriptor;
				napi_value function_ptr;
				napi_value function_sig;
				uint32_t function_sig_length;

				/* Get function name */
				status = napi_get_value_string_utf8(env, function_name, function_name_str, function_name_length + 1, &function_name_length);

				assert(status == napi_ok);

				/* Get function descriptor */
				status = napi_get_named_property(env, discover_map, function_name_str, &function_descriptor);

				assert(status == napi_ok);

				/* Get function pointer */
				status = napi_get_named_property(env, function_descriptor, "ptr", &function_ptr);

				assert(status == napi_ok);

				/* Check function pointer type */
				status = napi_typeof(env, function_ptr, &valuetype);

				assert(status == napi_ok);

				if (valuetype != napi_function)
				{
					napi_throw_type_error(env, nullptr, "Invalid NodeJS function");
				}

				/* Get function signature */
				status = napi_get_named_property(env, function_descriptor, "signature", &function_sig);

				assert(status == napi_ok);

				/* Check function pointer type */
				status = napi_typeof(env, function_sig, &valuetype);

				assert(status == napi_ok);

				if (valuetype != napi_object)
				{
					napi_throw_type_error(env, nullptr, "Invalid NodeJS signature");
				}

				/* Get signature length */
				status = napi_get_array_length(env, function_sig, &function_sig_length);

				assert(status == napi_ok);

				/* Create node function */
				loader_impl_node_function node_func = static_cast<loader_impl_node_function>(malloc(sizeof(struct loader_impl_node_function_type)));

				/* Create reference to function pointer */
				uint32_t ref_count = 0;

				status = napi_create_reference(env, function_ptr, 0, &node_func->func_ref);

				assert(status == napi_ok);

				status = napi_reference_ref(env, node_func->func_ref, &ref_count);

				assert(status == napi_ok && ref_count == 1);

				node_func->node_impl = async_data->node_impl;

				/* Create function */
				function f = function_create(function_name_str, (size_t)function_sig_length, node_func, &function_node_singleton);

				if (f != NULL)
				{
					signature s = function_signature(f);
					scope sp = context_scope(async_data->ctx);

					for (index = 0; index < function_sig_length; ++index)
					{
						napi_value parameter_name;
						size_t parameter_name_length;
						char * parameter_name_str = NULL;

						/* Get signature parameter name */
						status = napi_get_element(env, function_sig, index, &parameter_name);

						assert(status == napi_ok);

						/* Get parameter name string length */
						status = napi_get_value_string_utf8(env, parameter_name, NULL, 0, &parameter_name_length);

						assert(status == napi_ok);

						if (parameter_name_length > 0)
						{
							parameter_name_str = static_cast<char *>(malloc(sizeof(char) * (parameter_name_length + 1)));
						}

						/* Get parameter name string */
						status = napi_get_value_string_utf8(env, parameter_name, parameter_name_str, parameter_name_length + 1, &parameter_name_length);

						assert(status == napi_ok);

						signature_set(s, index, parameter_name_str, NULL);
					}

					scope_define(sp, function_name_str, f);
				}
				else
				{
					free(node_func);
				}
			}
		}
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	assert(status == napi_ok);

	/* Signal discover condition */
	uv_cond_signal(&node_impl_cond);

	uv_mutex_unlock(&node_impl_mutex);
}

void * node_loader_impl_register(void * node_impl_ptr, void * env_ptr, void * function_table_object_ptr)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(node_impl_ptr);

	napi_env env = static_cast<napi_env>(env_ptr);
	napi_value function_table_object = static_cast<napi_value>(function_table_object_ptr);

	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		const char test_str[] = "test";
		napi_value test_str_value;
	#endif

	bool result = false;

	uint32_t ref_count = 0;

	napi_status status;

	napi_value global;

	node_impl->env = env;

	/* Make global object persistent */
	status = napi_get_global(env, &global);

	assert(status == napi_ok);

	status = napi_create_reference(env, global, 0, &node_impl->global_ref);

	assert(status == napi_ok);

	status = napi_reference_ref(env, node_impl->global_ref, &ref_count);

	assert(status == napi_ok && ref_count == 1);

	/* Make function table object persistent */
	status = napi_create_reference(env, function_table_object, 0, &node_impl->function_table_object_ref);

	assert(status == napi_ok);

	status = napi_reference_ref(env, node_impl->function_table_object_ref, &ref_count);

	assert(status == napi_ok && ref_count == 1);

	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	{
		/* Retrieve test function from object table */
		status = napi_create_string_utf8(env, test_str, sizeof(test_str) - 1, &test_str_value);

		assert(status == napi_ok);

		status = napi_has_own_property(env, function_table_object, test_str_value, &result);

		assert(status == napi_ok);

		if (result == true)
		{
			napi_value function_trampoline_test;
			napi_valuetype valuetype;

			status = napi_get_named_property(env, function_table_object, test_str, &function_trampoline_test);

			assert(status == napi_ok);

			status = napi_typeof(env, function_trampoline_test, &valuetype);

			assert(status == napi_ok);

			if (valuetype != napi_function)
			{
				napi_throw_type_error(env, nullptr, "Invalid function test in function table object");
			}

			/* Call to test function */
			napi_value return_value;

			status = napi_call_function(env, global, function_trampoline_test, 0, nullptr, &return_value);

			assert(status == napi_ok);
		}
	}
	#endif

	/* TODO: Return */
	return NULL;
}

void node_loader_impl_thread(void * data)
{
	loader_impl_node node_impl = *(static_cast<loader_impl_node *>(data));

	/* TODO: Reimplement from here to ... */

	/* TODO: Make this trick more portable... */
	char exe_path_str[PATH_MAX];
	size_t exe_path_str_size, exe_path_str_offset = 0;

	#if defined(WIN32) || defined(_WIN32)
		unsigned int length = GetModuleFileName(NULL, exe_path_str, PATH_MAX);
	#else
		ssize_t length = readlink("/proc/self/exe", exe_path_str, PATH_MAX);
	#endif

	size_t iterator;

	if (length == -1 || length == PATH_MAX)
	{
		/* log_write("metacall", LOG_LEVEL_ERROR, "node loader register invalid working directory path (%s)", exe_path_str); */
		return;
	}

	for (iterator = 0; iterator <= (size_t)length; ++iterator)
	{
		#if defined(WIN32) || defined(_WIN32)
			if (exe_path_str[iterator] == '\\')
		#else
			if (exe_path_str[iterator] == '/')
		#endif
		{
			exe_path_str_offset = iterator + 1;
		}
	}

	exe_path_str_size = (size_t)length - exe_path_str_offset + 1;

	/* Get the boostrap path */
	const char bootstrap_file_str[] = "bootstrap.js";

	char bootstrap_path_str[PATH_MAX];
	size_t bootstrap_path_str_size;

	const char * load_library_path_env = getenv("LOADER_LIBRARY_PATH");
	size_t load_library_path_length;

	if (load_library_path_env == NULL)
	{
		return;
	}

	load_library_path_length = strlen(load_library_path_env);

	strncpy(bootstrap_path_str, load_library_path_env, load_library_path_length);

	if (bootstrap_path_str[load_library_path_length - 1] != '/' && bootstrap_path_str[load_library_path_length - 1] != '\\')
	{
		#if defined(WIN32) || defined(_WIN32)
			bootstrap_path_str[load_library_path_length] = '\\';
		#else
			bootstrap_path_str[load_library_path_length] = '/';
		#endif

		++load_library_path_length;
	}

	strncpy(&bootstrap_path_str[load_library_path_length], bootstrap_file_str, sizeof(bootstrap_file_str) - 1);

	bootstrap_path_str_size = load_library_path_length + sizeof(bootstrap_file_str);

	bootstrap_path_str[bootstrap_path_str_size - 1] = '\0';

	/* Get node impl pointer */
	char * node_impl_ptr_str;
	size_t node_impl_ptr_str_size;

	ssize_t node_impl_ptr_length = snprintf(NULL, 0, "%p", (void *)node_impl);

	if (node_impl_ptr_length <= 0)
	{
		return;
	}

	node_impl_ptr_str_size = (size_t)node_impl_ptr_length + 1;

	node_impl_ptr_str = static_cast<char *>(malloc(sizeof(char) * node_impl_ptr_str_size));

	if (node_impl_ptr_str == NULL)
	{
		return;
	}

	snprintf(node_impl_ptr_str, node_impl_ptr_str_size, "%p", (void *)node_impl);

	/* Get register pointer */
	char * register_ptr_str;
	size_t register_ptr_str_size;

	ssize_t register_ptr_length = snprintf(NULL, 0, "%p", (void *)&node_loader_impl_register);

	if (register_ptr_length <= 0)
	{
		return;
	}

	register_ptr_str_size = (size_t)register_ptr_length + 1;

	register_ptr_str = static_cast<char *>(malloc(sizeof(char) * register_ptr_str_size));

	if (register_ptr_str == NULL)
	{
		free(node_impl_ptr_str);
		return;
	}

	snprintf(register_ptr_str, register_ptr_str_size, "%p", (void *)&node_loader_impl_register);

	/* Define argv_str contigously allocated with: executable name, bootstrap file, node impl pointer and register pointer */
	size_t argv_str_size = exe_path_str_size + bootstrap_path_str_size + node_impl_ptr_str_size + register_ptr_str_size;
	char * argv_str = static_cast<char *>(malloc(sizeof(char) * argv_str_size));

	if (argv_str == NULL)
	{
		free(node_impl_ptr_str);
		free(register_ptr_str);
		return;
	}

	memcpy(&argv_str[0], &exe_path_str[exe_path_str_offset], exe_path_str_size);
	memcpy(&argv_str[exe_path_str_size], bootstrap_path_str, bootstrap_path_str_size);
	memcpy(&argv_str[exe_path_str_size + bootstrap_path_str_size], node_impl_ptr_str, node_impl_ptr_str_size);
	memcpy(&argv_str[exe_path_str_size + bootstrap_path_str_size + node_impl_ptr_str_size], register_ptr_str, register_ptr_str_size);

	free(node_impl_ptr_str);
	free(register_ptr_str);

	/* Define argv */
	char * argv[] =
	{
		&argv_str[0],
		&argv_str[exe_path_str_size],
		&argv_str[exe_path_str_size + bootstrap_path_str_size],
		&argv_str[exe_path_str_size + bootstrap_path_str_size + node_impl_ptr_str_size],
		NULL
	};

	int argc = 4;

	/* TODO: ... reimplement until here */

	node_impl->thread_loop = uv_default_loop();

	/* Initialize initialize signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_initialize, &node_loader_impl_async_initialize);

	/* Initialize load from file signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_load_from_file, &node_loader_impl_async_load_from_file);

	/* Initialize clear signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_clear, &node_loader_impl_async_clear);

	/* Initialize discover signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_discover, &node_loader_impl_async_discover);

	/* Initialize function call signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_func_call, &node_loader_impl_async_func_call);

	/* Initialize function destroy signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_func_destroy, &node_loader_impl_async_func_destroy);

	/* Initialize destroy signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_destroy, &node_loader_impl_async_destroy);

	/* Signal start condition */
	uv_mutex_lock(&node_impl_mutex);

	uv_cond_signal(&node_impl_cond);

	uv_mutex_unlock(&node_impl_mutex);

	/* Start NodeJS runtime */
	int result = node::Start(argc, reinterpret_cast<char **>(argv));

	node_impl->result = result;
}

loader_impl_data node_loader_impl_initialize(loader_impl impl, configuration config, loader_host host)
{
	loader_impl_node node_impl;

	(void)impl;
	(void)config;

	if (log_copy(host->log) != 0)
	{
		return NULL;
	}

	node_impl = new loader_impl_node_type();

	if (node_impl == nullptr)
	{
		return NULL;
	}

	/* Duplicate stdin, stdout, stderr */
	stdin_copy = dup(STDIN_FILENO);
	stdout_copy = dup(STDOUT_FILENO);
	stderr_copy = dup(STDERR_FILENO);

	/* Initialize syncronization */
	assert(uv_cond_init(&node_impl_cond) == 0);

	assert(uv_mutex_init(&node_impl_mutex) == 0);

	/* Initialize execution result */
	node_impl->result = 1;

	/* Create NodeJS thread */
	assert(uv_thread_create(&node_impl->thread_id, node_loader_impl_thread, &node_impl) == 0);

	/* Wait until start has been launch */
	uv_mutex_lock(&node_impl_mutex);

	uv_cond_wait(&node_impl_cond, &node_impl_mutex);

	uv_mutex_unlock(&node_impl_mutex);

	/* TODO: Initialize method unused by now */

	/* Initialize node loader entry point */
	/*
	node_impl->async_initialize.data = static_cast<void *>(&node_impl);
	*/

	/* Execute initialize async callback */
	/*
	uv_async_send(&node_impl->async_initialize);
	*/

	/* Wait until script has been loaded */
	/*
	uv_mutex_lock(&node_impl_mutex);

	uv_cond_wait(&node_impl_cond, &node_impl_mutex);

	uv_mutex_unlock(&node_impl_mutex);
	*/

	return node_impl;
}

int node_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	/* TODO */

	(void)impl;
	(void)path;

	return 0;
}

loader_handle node_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	if (node_impl == NULL)
	{
		return NULL;
	}

	struct loader_impl_async_load_from_file_type async_data =
	{
		node_impl,
		paths,
		size,
		NULL
	};

	node_impl->async_load_from_file.data = static_cast<void *>(&async_data);

	/* Execute load from file async callback */
	uv_async_send(&node_impl->async_load_from_file);

	/* Wait until module is loaded */
	uv_mutex_lock(&node_impl_mutex);

	uv_cond_wait(&node_impl_cond, &node_impl_mutex);

	uv_mutex_unlock(&node_impl_mutex);

	return static_cast<loader_handle>(async_data.handle_ref);
}

loader_handle node_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size)
{
	/* TODO */

	(void)impl;
	(void)name;
	(void)buffer;
	(void)size;

	return NULL;
}

loader_handle node_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	/* TODO */

	(void)impl;
	(void)path;

	return NULL;
}

int node_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	napi_ref handle_ref = static_cast<napi_ref>(handle);

	if (node_impl == NULL || handle_ref == NULL)
	{
		return 1;
	}

	struct loader_impl_async_clear_type async_data =
	{
		node_impl,
		handle_ref
	};

	node_impl->async_clear.data = static_cast<void *>(&async_data);

	/* Execute clear async callback */
	uv_async_send(&node_impl->async_clear);

	/* Wait until module is cleared */
	uv_mutex_lock(&node_impl_mutex);

	uv_cond_wait(&node_impl_cond, &node_impl_mutex);

	uv_mutex_unlock(&node_impl_mutex);

	return 0;
}

int node_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	if (node_impl == NULL || handle == NULL || ctx == NULL)
	{
		return 1;
	}

	napi_ref handle_ref = static_cast<napi_ref>(handle);

	struct loader_impl_async_discover_type async_data =
	{
		node_impl,
		handle_ref,
		ctx
	};

	node_impl->async_discover.data = static_cast<void *>(&async_data);

	/* Execute discover async callback */
	uv_async_send(&node_impl->async_discover);

	/* Wait until module is discovered */
	uv_mutex_lock(&node_impl_mutex);

	uv_cond_wait(&node_impl_cond, &node_impl_mutex);

	uv_mutex_unlock(&node_impl_mutex);

	return 0;
}

void node_loader_impl_async_destroy(uv_async_t * async)
{
	loader_impl_node node_impl;

	uint32_t ref_count = 0;

	napi_status status;

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl_mutex);

	node_impl = *(static_cast<loader_impl_node *>(async->data));

	/* Call destroy function */
	{
		const char destroy_str[] = "destroy";
		napi_value destroy_str_value;

		napi_env env = node_impl->env;
		napi_value function_table_object;

		bool result = false;

		napi_handle_scope handle_scope;

		/* Create scope */
		status = napi_open_handle_scope(node_impl->env, &handle_scope);

		assert(status == napi_ok);

		/* Get function table object from reference */
		status = napi_get_reference_value(env, node_impl->function_table_object_ref, &function_table_object);

		assert(status == napi_ok);

		/* Retrieve destroy function from object table */
		status = napi_create_string_utf8(env, destroy_str, sizeof(destroy_str) - 1, &destroy_str_value);

		assert(status == napi_ok);

		status = napi_has_own_property(env, function_table_object, destroy_str_value, &result);

		assert(status == napi_ok);

		if (result == true)
		{
			napi_value function_trampoline_destroy;
			napi_valuetype valuetype;

			status = napi_get_named_property(env, function_table_object, destroy_str, &function_trampoline_destroy);

			assert(status == napi_ok);

			status = napi_typeof(env, function_trampoline_destroy, &valuetype);

			assert(status == napi_ok);

			if (valuetype != napi_function)
			{
				napi_throw_type_error(env, nullptr, "Invalid function destroy in function table object");
			}

			/* Call to destroy function */
			napi_value global, return_value;

			status = napi_get_global(env, &global);

			assert(status == napi_ok);

			status = napi_call_function(env, global, function_trampoline_destroy, 0, nullptr, &return_value);

			assert(status == napi_ok);
		}

		/* Close scope */
		status = napi_close_handle_scope(env, handle_scope);

		assert(status == napi_ok);
	}

	/* Clear persistent references */
	status = napi_reference_unref(node_impl->env, node_impl->global_ref, &ref_count);

	assert(status == napi_ok && ref_count == 0);

	status = napi_delete_reference(node_impl->env, node_impl->global_ref);

	assert(status == napi_ok);

	status = napi_reference_unref(node_impl->env, node_impl->function_table_object_ref, &ref_count);

	assert(status == napi_ok && ref_count == 0);

	status = napi_delete_reference(node_impl->env, node_impl->function_table_object_ref);

	assert(status == napi_ok);

	/* Destroy async objects */
	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_destroy), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_func_destroy), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_func_call), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_discover), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_clear), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_load_from_file), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_initialize), NULL);

	/*  Stop event loop */
	uv_stop(node_impl->thread_loop);

	/* Clear event loop */
	uv_walk(node_impl->thread_loop, node_loader_impl_walk, NULL);

	while (uv_run(node_impl->thread_loop, UV_RUN_DEFAULT) != 0);

	/* Destroy node loop */
	if (uv_loop_alive(node_impl->thread_loop) != 0)
	{
		/* log_write("metacall", LOG_LEVEL_ERROR, "NodeJS event loop should not be alive"); */
	}

	/* TODO: Check how to delete properly all handles */
	if (uv_loop_close(node_impl->thread_loop) == UV_EBUSY)
	{
		/* log_write("metacall", LOG_LEVEL_ERROR, "NodeJS event loop should not be busy"); */
	}

	/* Signal destroy condition */
	uv_cond_signal(&node_impl_cond);

	uv_mutex_unlock(&node_impl_mutex);
}

void node_loader_impl_walk(uv_handle_t * handle, void * arg)
{
	(void)arg;

	/* TODO: This method also deletes the handle flush_tasks_ inside the NodePlatform class. */
	/* So, now I don't know how to identify this pointer in order to avoid closing it... */
	/* By this way, I prefer to make valgrind angry instead of closing it with an abort or an exception */

	(void)handle;

	/*
	if (!uv_is_closing(handle))
	{
		uv_close(handle, NULL);
	}
	*/
}

int node_loader_impl_destroy(loader_impl impl)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	if (node_impl == NULL)
	{
		return 1;
	}

	/* Send async destroy */
	node_impl->async_destroy.data = static_cast<void *>(&node_impl);

	uv_async_send(&node_impl->async_destroy);

	/* Wait until node is destroyed */
	uv_mutex_lock(&node_impl_mutex);

	uv_cond_wait(&node_impl_cond, &node_impl_mutex);

	uv_mutex_unlock(&node_impl_mutex);

	/* Clear destroy syncronization and async objects */
	uv_mutex_destroy(&node_impl_mutex);

	uv_cond_destroy(&node_impl_cond);

	/* Wait for node thread to finish */
	uv_thread_join(&node_impl->thread_id);

	/* Print NodeJS execution result */
	/* log_write("metacall", LOG_LEVEL_INFO, "NodeJS execution return status %d", node_impl->result); */

	delete node_impl;

	/* Restore stdin, stdout, stderr */
	dup2(stdin_copy, STDIN_FILENO);
	dup2(stdout_copy, STDOUT_FILENO);
	dup2(stderr_copy, STDERR_FILENO);

	return 0;
}
