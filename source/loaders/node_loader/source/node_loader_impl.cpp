/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
#include <reflect/reflect_future.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

/* TODO: Make logs thread safe */
#include <log/log.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <new>
#include <string>
#include <fstream>
#include <streambuf>

#include <node.h>
#include <node_api.h>

#include <libplatform/libplatform.h>
#include <v8.h> /* version: 6.2.414.50 */

#ifdef ENABLE_DEBUGGER_SUPPORT
#	include <v8-debug.h>
#endif /* ENALBLE_DEBUGGER_SUPPORT */

#include <uv.h>

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

/* TODO: (2.0)

	Detour method is not valid because of NodeJS cannot be reinitialized, platform pointer already initialized in CHECK macro
*/

/* TODO: (3.0)

	Use uv_loop_fork if available to fork and avoid reinitializing
*/

#define NODE_GET_EVENT_LOOP \
	(NAPI_VERSION >= 2) && \
	((NODE_MAJOR_VERSION == 8 && NODE_MINOR_VERSION >= 10) || \
	(NODE_MAJOR_VERSION == 9 && NODE_MINOR_VERSION >= 3) || \
	(NODE_MAJOR_VERSION >= 10))

#define NODE_THREADSAFE_FUNCTION \
	(NAPI_VERSION >= 4) && \
	(NODE_MAJOR_VERSION >= 8 && NODE_MINOR_VERSION >= 10)

#define NODE_THREADSAFE_FUNCTION_OPTIONAL \
	(NAPI_VERSION >= 4) && \
	(NODE_MAJOR_VERSION > 12 || (NODE_MAJOR_VERSION == 12 && NODE_MINOR_VERSION >= 6))

static const char loader_impl_node_resolve_trampoline[] = "__metacall_loader_impl_node_resolve_trampoline__";
static const char loader_impl_node_reject_trampoline[] = "__metacall_loader_impl_node_reject_trampoline__";

typedef struct loader_impl_node_type
{
	napi_env env;
	napi_ref global_ref;
	napi_ref function_table_object_ref;

	uv_thread_t thread_id;
	uv_loop_t * thread_loop;

	uv_async_t async_initialize;
	uv_async_t async_load_from_file;
	uv_async_t async_load_from_memory;
	uv_async_t async_clear;
	uv_async_t async_discover;
	uv_async_t async_func_call;
	uv_async_t async_func_destroy;
	uv_async_t async_future_await;
	uv_async_t async_destroy;

	napi_ref resolve_trampoline_ref;
	napi_ref reject_trampoline_ref;

	/* TODO: This data must be binded to the promise, not here */
	future_resolve_callback resolve_callback;
	future_reject_callback reject_callback;

	uv_mutex_t mutex;
	uv_cond_t cond;

	int stdin_copy;
	int stdout_copy;
	int stderr_copy;

	int result;

} * loader_impl_node;

typedef struct loader_impl_node_function_type
{
	loader_impl_node node_impl;
	napi_ref func_ref;
	napi_value * argv;

} * loader_impl_node_function;

typedef struct loader_impl_node_future_type
{
	loader_impl_node node_impl;
	napi_ref promise_ref;

} * loader_impl_node_future;

typedef struct loader_impl_async_initialize_type
{
	loader_impl_node node_impl;
	int result;

} * loader_impl_async_initialize;

typedef struct loader_impl_async_load_from_file_type
{
	loader_impl_node node_impl;
	const loader_naming_path * paths;
	size_t size;
	napi_ref handle_ref;

} * loader_impl_async_load_from_file;

typedef struct loader_impl_async_load_from_memory_type
{
	loader_impl_node node_impl;
	const char * name;
	const char * buffer;
	size_t size;
	napi_ref handle_ref;

} * loader_impl_async_load_from_memory;

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

typedef struct loader_impl_async_future_await_type
{
	loader_impl_node node_impl;
	future f;
	loader_impl_node_future node_future;
	future_impl impl;
	future_resolve_callback resolve_callback;
	future_reject_callback reject_callback;
	void * data;
	future_return ret;

} * loader_impl_async_future_await;

/* Exception */

static inline void node_loader_impl_exception(napi_env env, napi_status status);

/* Type conversion */
static value node_loader_impl_napi_to_value(loader_impl_node node_impl, napi_env env, napi_value v);

static napi_value node_loader_impl_value_to_napi(loader_impl_node node_impl, napi_env env, value arg);

/* Function */
static int function_node_interface_create(function func, function_impl impl);

static function_return function_node_interface_invoke(function func, function_impl impl, function_args args);

static void function_node_interface_destroy(function func, function_impl impl);

static function_interface function_node_singleton(void);

/* Future */
static int future_node_interface_create(future f, future_impl impl);

static future_return future_node_interface_await(future f, future_impl impl, future_resolve_callback resolve_callback, future_reject_callback reject_callback, void * data);

static void future_node_interface_destroy(future f, future_impl impl);

static future_interface future_node_singleton(void);

napi_value future_node_on_resolve(napi_env env, napi_callback_info info);

napi_value future_node_on_reject(napi_env env, napi_callback_info info);

/* Async */
static void node_loader_impl_async_initialize(uv_async_t * async);

static void node_loader_impl_async_func_call(uv_async_t * async);

static void node_loader_impl_async_func_destroy(uv_async_t * async);

static void node_loader_impl_async_future_await(uv_async_t * async);

static void node_loader_impl_async_load_from_file(uv_async_t * async);

static void node_loader_impl_async_load_from_memory(uv_async_t * async);

static void node_loader_impl_async_clear(uv_async_t * async);

static void node_loader_impl_async_discover(uv_async_t * async);

static void node_loader_impl_async_destroy(uv_async_t * async);

/* Loader */
static void * node_loader_impl_register(void * node_impl_ptr, void * env_ptr, void * function_table_object_ptr);

static void node_loader_impl_thread(void * data);

static void node_loader_impl_walk(uv_handle_t * handle, void * data);

/* -- Methods -- */

inline void node_loader_impl_exception(napi_env env, napi_status status)
{
	if (status != napi_ok)
	{
		if (status != napi_pending_exception)
		{
			const napi_extended_error_info * error_info = NULL;

			bool pending;

			napi_get_last_error_info(env, &error_info);

			napi_is_exception_pending(env, &pending);

			const char * message = (error_info->error_message == NULL) ? "Error message not available" : error_info->error_message;

			/* TODO: Notify MetaCall error handling system when it is implemented */
			/* ... */

			if (pending)
			{
				napi_throw_error(env, NULL, message);
			}
		}
		else
		{
			napi_value error, message;
			bool result;
			napi_valuetype valuetype;
			size_t length;
			char * str;

			status = napi_get_and_clear_last_exception(env, &error);

			node_loader_impl_exception(env, status);

			status = napi_is_error(env, error, &result);

			node_loader_impl_exception(env, status);

			if (result == false)
			{
				/* TODO: Notify MetaCall error handling system when it is implemented */
				return;
			}

			status = napi_get_named_property(env, error, "message", &message);

			node_loader_impl_exception(env, status);

			status = napi_typeof(env, message, &valuetype);

			node_loader_impl_exception(env, status);

			if (valuetype != napi_string)
			{
				/* TODO: Notify MetaCall error handling system when it is implemented */
				return;
			}

			status = napi_get_value_string_utf8(env, message, NULL, 0, &length);

			node_loader_impl_exception(env, status);

			str = static_cast<char *>(malloc(sizeof(char) * (length + 1)));

			if (str == NULL)
			{
				/* TODO: Notify MetaCall error handling system when it is implemented */
				return;
			}

			status = napi_get_value_string_utf8(env, message, str, length + 1, &length);

			node_loader_impl_exception(env, status);

			/* TODO: Notify MetaCall error handling system when it is implemented */
			/* error_raise(str); */

			free(str);
		}
	}
}

value node_loader_impl_napi_to_value(loader_impl_node node_impl, napi_env env, napi_value v)
{
	value ret = NULL;

	napi_valuetype valuetype;

	napi_status status = napi_typeof(env, v, &valuetype);

	node_loader_impl_exception(env, status);

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

		node_loader_impl_exception(env, status);

		ret = value_create_bool((b == true) ? static_cast<boolean>(1) : static_cast<boolean>(0));
	}
	else if (valuetype == napi_number)
	{
		double d;

		status = napi_get_value_double(env, v, &d);

		node_loader_impl_exception(env, status);

		ret = value_create_double(d);
	}
	else if (valuetype == napi_string)
	{
		size_t length;

		status = napi_get_value_string_utf8(env, v, NULL, 0, &length);

		node_loader_impl_exception(env, status);

		ret = value_create_string(NULL, length);

		if (ret != NULL)
		{
			char * str = value_to_string(ret);

			status = napi_get_value_string_utf8(env, v, str, length + 1, &length);

			node_loader_impl_exception(env, status);
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

			node_loader_impl_exception(env, status);

			ret = value_create_array(NULL, static_cast<size_t>(length));

			array_value = value_to_array(ret);

			for (iterator = 0; iterator < length; ++iterator)
			{
				napi_value element;

				status = napi_get_element(env, v, iterator, &element);

				node_loader_impl_exception(env, status);

				/* TODO: Review recursion overflow */
				array_value[iterator] = node_loader_impl_napi_to_value(node_impl, env, element);
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
		else if (napi_is_promise(env, v, &result) == napi_ok && result == true)
		{
			loader_impl_node_future node_future = static_cast<loader_impl_node_future>(malloc(sizeof(struct loader_impl_node_future_type)));

			future f;

			if (node_future == NULL)
			{
				return static_cast<function_return>(NULL);
			}

			f = future_create(node_future, &future_node_singleton);

			if (f == NULL)
			{
				free(node_future);

				return static_cast<function_return>(NULL);
			}

			ret = value_create_future(f);

			if (ret == NULL)
			{
				future_destroy(f);
			}

			/* Create reference to promise */
			node_future->node_impl = node_impl;

			status = napi_create_reference(env, v, 1, &node_future->promise_ref);

			node_loader_impl_exception(env, status);
		}
		else
		{
			/* TODO: Strict check if it is an object (map) */
			uint32_t iterator, length = 0;

			napi_value keys;

			value * map_value;

			status = napi_get_property_names(env, v, &keys);

			node_loader_impl_exception(env, status);

			status = napi_get_array_length(env, keys, &length);

			node_loader_impl_exception(env, status);

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

				node_loader_impl_exception(env, status);

				/* Set key string in the tupla */
				status = napi_get_value_string_utf8(env, key, NULL, 0, &key_length);

				node_loader_impl_exception(env, status);

				tupla[0] = value_create_string(NULL, key_length);

				if (tupla[0] != NULL)
				{
					napi_value element;

					char * str = value_to_string(tupla[0]);

					status = napi_get_value_string_utf8(env, key, str, key_length + 1, &key_length);

					node_loader_impl_exception(env, status);

					status = napi_get_property(env, v, key, &element);

					node_loader_impl_exception(env, status);

					/* TODO: Review recursion overflow */
					tupla[1] = node_loader_impl_napi_to_value(node_impl, env, element);
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

	return ret;
}

napi_value node_loader_impl_value_to_napi(loader_impl_node node_impl, napi_env env, value arg)
{
	value arg_value = static_cast<value>(arg);

	type_id id = value_type_id(arg_value);

	napi_status status;

	napi_value v = nullptr;

	if (id == TYPE_BOOL)
	{
		boolean bool_value = value_to_bool(arg_value);

		status = napi_get_boolean(env, (bool_value == 0) ? false : true, &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_CHAR)
	{
		char char_value = value_to_char(arg_value);

		status = napi_create_int32(env, static_cast<int32_t>(char_value), &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_SHORT)
	{
		short short_value = value_to_short(arg_value);

		status = napi_create_int32(env, static_cast<int32_t>(short_value), &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_INT)
	{
		int int_value = value_to_int(arg_value);

		/* TODO: Check integer overflow */
		status = napi_create_int32(env, static_cast<int32_t>(int_value), &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_LONG)
	{
		long long_value = value_to_long(arg_value);

		/* TODO: Check integer overflow */
		status = napi_create_int64(env, static_cast<int64_t>(long_value), &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_FLOAT)
	{
		float float_value = value_to_float(arg_value);

		status = napi_create_double(env, static_cast<double>(float_value), &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_DOUBLE)
	{
		double double_value = value_to_double(arg_value);

		status = napi_create_double(env, double_value, &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_STRING)
	{
		const char * str_value = value_to_string(arg_value);

		size_t length = value_type_size(arg_value) - 1;

		status = napi_create_string_utf8(env, str_value, length, &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_BUFFER)
	{
		void * buff_value = value_to_buffer(arg_value);

		size_t size = value_type_size(arg_value);

		status = napi_create_buffer(env, size, &buff_value, &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_ARRAY)
	{
		value * array_value = value_to_array(arg_value);

		size_t array_size = value_type_count(arg_value);

		uint32_t iterator;

		status = napi_create_array_with_length(env, array_size, &v);

		node_loader_impl_exception(env, status);

		for (iterator = 0; iterator < array_size; ++iterator)
		{
			/* TODO: Review recursion overflow */
			napi_value element_v =node_loader_impl_value_to_napi(node_impl, env, static_cast<value>(array_value[iterator]));

			status = napi_set_element(env, v, iterator, element_v);

			node_loader_impl_exception(env, status);
		}
	}
	else if (id == TYPE_MAP)
	{
		value * map_value = value_to_map(arg_value);

		size_t iterator, map_size = value_type_count(arg_value);

		status = napi_create_object(env, &v);

		node_loader_impl_exception(env, status);

		for (iterator = 0; iterator < map_size; ++iterator)
		{
			value * pair_value = value_to_array(map_value[iterator]);

			const char * key = value_to_string(pair_value[0]);

			/* TODO: Review recursion overflow */
			napi_value element_v =node_loader_impl_value_to_napi(node_impl, env, static_cast<value>(pair_value[1]));

			status = napi_set_named_property(env, v, key, element_v);

			node_loader_impl_exception(env, status);
		}
	}
	/* TODO */
	/*
	else if (id == TYPE_PTR)
	{

	}
	*/
	else if (id == TYPE_FUTURE)
	{
		/* TODO: Implement promise properly for await */
	}
	else
	{
		status = napi_get_undefined(env, &v);

		node_loader_impl_exception(env, status);
	}

	return v;
}

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

		uv_mutex_lock(&node_impl->mutex);

		node_impl->async_func_call.data = static_cast<void *>(&async_data);

		/* Execute function call async callback */
		uv_async_send(&node_impl->async_func_call);

		/* Wait until function is called */
		uv_cond_wait(&node_impl->cond, &node_impl->mutex);

		uv_mutex_unlock(&node_impl->mutex);

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

		uv_mutex_lock(&node_impl->mutex);

		node_impl->async_func_destroy.data = static_cast<void *>(&async_data);

		/* Execute function destroy async callback */
		uv_async_send(&node_impl->async_func_destroy);

		/* Wait until function is destroyed */
		uv_cond_wait(&node_impl->cond, &node_impl->mutex);

		uv_mutex_unlock(&node_impl->mutex);

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

int future_node_interface_create(future f, future_impl impl)
{
	(void)f;
	(void)impl;

	return 0;
}

future_return future_node_interface_await(future f, future_impl impl, future_resolve_callback resolve_callback, future_reject_callback reject_callback, void * data)
{
	loader_impl_node_future node_future = (loader_impl_node_future)impl;

	if (node_future != NULL)
	{
		loader_impl_node node_impl = node_future->node_impl;

		struct loader_impl_async_future_await_type async_data =
		{
			node_impl,
			f,
			node_future,
			impl,
			resolve_callback,
			reject_callback,
			data,
			NULL
		};

		uv_mutex_lock(&node_impl->mutex);

		node_impl->async_future_await.data = static_cast<void *>(&async_data);

		/* Execute future await async callback */
		uv_async_send(&node_impl->async_future_await);

		/* Wait until function is called */
		uv_cond_wait(&node_impl->cond, &node_impl->mutex);

		uv_mutex_unlock(&node_impl->mutex);

		return async_data.ret;
	}

	return NULL;
}

void future_node_interface_destroy(future f, future_impl impl)
{
	/* TODO */
	(void)f;
	(void)impl;
}

future_interface future_node_singleton()
{
	static struct future_interface_type node_future_interface =
	{
		&future_node_interface_create,
		&future_node_interface_await,
		&future_node_interface_destroy
	};

	return &node_future_interface;
}

napi_value future_node_on_resolve(napi_env env, napi_callback_info info)
{
	loader_impl_node node_impl;

	size_t argc;

	napi_value argv[1], this_arg, result;

	void * data;

	napi_status status;

	value arg, ret;

	napi_handle_scope handle_scope;

	/* Create scope */
	status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Retrieve the arguments and bind data */
	status = napi_get_cb_info(env, info, &argc, &argv[0], &this_arg, &data);

	node_loader_impl_exception(env, status);

	if (argc != 1)
	{
		/* TODO: Error handling */
	}

	node_impl = static_cast<loader_impl_node>(data);

	if (node_impl->resolve_callback == NULL)
	{
		return nullptr;
	}

	/* Convert the argument to a value */
	arg = node_loader_impl_napi_to_value(node_impl, env, argv[0]);

	if (arg == NULL)
	{
		arg = value_create_null();
	}

	/* Call the resolve callback */
	ret = node_impl->resolve_callback(arg, NULL /* TODO: data*/);

	/* Destroy parameter argument */
	value_type_destroy(arg);

	/* Return the result */
	result = node_loader_impl_value_to_napi(node_impl, env, ret);

	/* Close scope */
	status = napi_close_handle_scope(node_impl->env, handle_scope);

	node_loader_impl_exception(node_impl->env, status);

	/* Destroy return value */
	value_type_destroy(ret);

	return result;
}

napi_value future_node_on_reject(napi_env env, napi_callback_info info)
{
	loader_impl_node node_impl;

	size_t argc;

	napi_value argv[1], this_arg, result;

	void * data;

	napi_status status;

	value arg, ret;

	napi_handle_scope handle_scope;

	/* Create scope */
	status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Retrieve the arguments and bind data */
	status = napi_get_cb_info(env, info, &argc, &argv[0], &this_arg, &data);

	node_loader_impl_exception(env, status);

	if (argc != 1)
	{
		/* TODO: Error handling */
	}

	node_impl = static_cast<loader_impl_node>(data);

	if (node_impl->reject_callback == NULL)
	{
		return nullptr;
	}

	/* Convert the argument to a value */
	arg = node_loader_impl_napi_to_value(node_impl, env, argv[0]);

	if (arg == NULL)
	{
		arg = value_create_null();
	}

	/* Call the resolve callback */
	ret = node_impl->reject_callback(arg, NULL /* TODO: data*/);

	/* Destroy parameter argument */
	value_type_destroy(arg);

	/* Return the result */
	result = node_loader_impl_value_to_napi(node_impl, env, ret);

	/* Close scope */
	status = napi_close_handle_scope(node_impl->env, handle_scope);

	node_loader_impl_exception(node_impl->env, status);

	/* Destroy return value */
	value_type_destroy(ret);

	return result;
}

void node_loader_impl_async_initialize(uv_async_t * async)
{
	napi_status status;

	napi_value resolve_func, reject_func;

	napi_handle_scope handle_scope;

	loader_impl_async_initialize async_initialize = static_cast<loader_impl_async_initialize>(async->data);

	loader_impl_node node_impl = async_initialize->node_impl;

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl->mutex);

	/* Create scope */
	status = napi_open_handle_scope(node_impl->env, &handle_scope);

	node_loader_impl_exception(node_impl->env, status);

	/* Check if event loop is correctly initialized */
	#if NODE_GET_EVENT_LOOP
	{
		struct uv_loop_s * loop;

		status = napi_get_uv_event_loop(node_impl->env, &loop);

		node_loader_impl_exception(node_impl->env, status);

		if (loop != node_impl->thread_loop)
		{
			/* TODO: error_raise("Invalid event loop"); */

			/* Set error */
			async_initialize->result = 1;

			/* Signal start condition */
			uv_cond_signal(&node_impl->cond);

			uv_mutex_unlock(&node_impl->mutex);

			return;
		}
	}
	#endif /* NODE_GET_EVENT_LOOP */

	/* Initialize future trampolines (TODO: Check version and use napi_create_threadsafe_function if available) */
	status = napi_create_function(node_impl->env, loader_impl_node_resolve_trampoline, sizeof(loader_impl_node_resolve_trampoline) - 1, &future_node_on_resolve, node_impl, &resolve_func);

	node_loader_impl_exception(node_impl->env, status);

	status = napi_create_reference(node_impl->env, resolve_func, 1, &node_impl->resolve_trampoline_ref);

	node_loader_impl_exception(node_impl->env, status);

	status = napi_create_function(node_impl->env, loader_impl_node_reject_trampoline, sizeof(loader_impl_node_reject_trampoline) - 1, &future_node_on_reject, node_impl, &reject_func);

	node_loader_impl_exception(node_impl->env, status);

	status = napi_create_reference(node_impl->env, reject_func, 1, &node_impl->reject_trampoline_ref);

	node_loader_impl_exception(node_impl->env, status);

	/* Close scope */
	status = napi_close_handle_scope(node_impl->env, handle_scope);

	node_loader_impl_exception(node_impl->env, status);

	/* Signal start condition */
	uv_cond_signal(&node_impl->cond);

	uv_mutex_unlock(&node_impl->mutex);
}

void node_loader_impl_async_func_call(uv_async_t * async)
{
	loader_impl_async_func_call async_data;

	napi_env env;

	napi_handle_scope handle_scope;

	signature s;

	size_t args_size;

	value * args;

	loader_impl_node_function node_func;

	size_t args_count;

	async_data = static_cast<loader_impl_async_func_call>(async->data);

	/* Lock node implementation mutex */
	uv_mutex_lock(&async_data->node_impl->mutex);

	/* Get environment reference */
	env = async_data->node_impl->env;

	/* Get function data */
	s = function_signature(async_data->func);

	args_size = signature_count(s);

	args = static_cast<value *>(async_data->args);

	node_func = async_data->node_func;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Build parameters */
	for (args_count = 0; args_count < args_size; ++args_count)
	{
		/* Define parameter */
		node_func->argv[args_count] = node_loader_impl_value_to_napi(async_data->node_impl, env, args[args_count]);
	}

	/* Get function reference */
	napi_value function_ptr;

	status = napi_get_reference_value(env, node_func->func_ref, &function_ptr);

	node_loader_impl_exception(env, status);

	/* Call to function */
	napi_value global, func_return;

	status = napi_get_reference_value(env, async_data->node_impl->global_ref, &global);

	node_loader_impl_exception(env, status);

	status = napi_call_function(env, global, function_ptr, args_size, node_func->argv, &func_return);

	node_loader_impl_exception(env, status);

	/* Convert function return to value */
	async_data->ret = node_loader_impl_napi_to_value(async_data->node_impl, env, func_return);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);

	/* Signal function call condition */
	uv_cond_signal(&async_data->node_impl->cond);

	uv_mutex_unlock(&async_data->node_impl->mutex);
}

void node_loader_impl_async_func_destroy(uv_async_t * async)
{
	loader_impl_async_func_destroy async_data;

	napi_env env;

	uint32_t ref_count = 0;

	napi_handle_scope handle_scope;

	async_data = static_cast<loader_impl_async_func_destroy>(async->data);

	/* Lock node implementation mutex */
	uv_mutex_lock(&async_data->node_impl->mutex);

	/* Get environment reference */
	env = async_data->node_impl->env;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Clear function persistent reference */
	status = napi_reference_unref(env, async_data->node_func->func_ref, &ref_count);

	node_loader_impl_exception(env, status);

	if (ref_count != 0)
	{
		/* TODO: Error handling */
	}

	status = napi_delete_reference(env, async_data->node_func->func_ref);

	node_loader_impl_exception(env, status);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);

	/* Signal function destroy condition */
	uv_cond_signal(&async_data->node_impl->cond);

	uv_mutex_unlock(&async_data->node_impl->mutex);
}

void node_loader_impl_async_future_await(uv_async_t * async)
{
	loader_impl_async_future_await async_data;

	napi_env env;

	napi_value then;

	napi_value argv[2];

	napi_handle_scope handle_scope;

	async_data = static_cast<loader_impl_async_future_await>(async->data);

	/* Lock node implementation mutex */
	uv_mutex_lock(&async_data->node_impl->mutex);

	/* Get environment reference */
	env = async_data->node_impl->env;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Get promise reference */
	napi_value promise;

	status = napi_get_reference_value(env, async_data->node_future->promise_ref, &promise);

	node_loader_impl_exception(env, status);

	/* Get then function */
	status = napi_get_named_property(env, promise, "then", &then);

	node_loader_impl_exception(env, status);

	/* Get trampoline functions from JS to C and assign them to argv */
	status = napi_get_reference_value(env, async_data->node_impl->resolve_trampoline_ref, &argv[0]);

	node_loader_impl_exception(env, status);

	status = napi_get_reference_value(env, async_data->node_impl->reject_trampoline_ref, &argv[1]);

	node_loader_impl_exception(env, status);

	/* Set current callbacks to node impl in order to make them accessible to the trampolines */
	async_data->node_impl->resolve_callback = async_data->resolve_callback;

	async_data->node_impl->reject_callback = async_data->reject_callback;

	/* Call to function */
	napi_value promise_return;

	status = napi_call_function(env, promise, then, 2, argv, &promise_return);

	node_loader_impl_exception(env, status);

	/* Proccess the promise_return */
	async_data->ret = node_loader_impl_napi_to_value(async_data->node_impl, env, promise_return);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);

	/* Signal function destroy condition */
	uv_cond_signal(&async_data->node_impl->cond);

	uv_mutex_unlock(&async_data->node_impl->mutex);
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

	async_data = static_cast<loader_impl_async_load_from_file>(async->data);

	/* Lock node implementation mutex */
	uv_mutex_lock(&async_data->node_impl->mutex);

	/* Get environment reference */
	env = async_data->node_impl->env;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Get function table object from reference */
	status = napi_get_reference_value(env, async_data->node_impl->function_table_object_ref, &function_table_object);

	node_loader_impl_exception(env, status);

	/* Create function string */
	status = napi_create_string_utf8(env, load_from_file_str, sizeof(load_from_file_str) - 1, &load_from_file_str_value);

	node_loader_impl_exception(env, status);

	/* Check if exists in the table */
	status = napi_has_own_property(env, function_table_object, load_from_file_str_value, &result);

	node_loader_impl_exception(env, status);

	if (result == true)
	{
		napi_value function_trampoline_load_from_file;
		napi_valuetype valuetype;
		napi_value argv[1];

		status = napi_get_named_property(env, function_table_object, load_from_file_str, &function_trampoline_load_from_file);

		node_loader_impl_exception(env, status);

		status = napi_typeof(env, function_trampoline_load_from_file, &valuetype);

		node_loader_impl_exception(env, status);

		if (valuetype != napi_function)
		{
			napi_throw_type_error(env, nullptr, "Invalid function load_from_file in function table object");
		}

		/* Define parameters */
		status = napi_create_array_with_length(env, async_data->size, &argv[0]);

		node_loader_impl_exception(env, status);

		for (size_t index = 0; index < async_data->size; ++index)
		{
			napi_value path_str;

			size_t length = strnlen(async_data->paths[index], LOADER_NAMING_PATH_SIZE);

			status = napi_create_string_utf8(env, async_data->paths[index], length, &path_str);

			node_loader_impl_exception(env, status);

			status = napi_set_element(env, argv[0], (uint32_t)index, path_str);

			node_loader_impl_exception(env, status);
		}

		/* Call to load from file function */
		napi_value global, return_value;

		status = napi_get_reference_value(env, async_data->node_impl->global_ref, &global);

		node_loader_impl_exception(env, status);

		status = napi_call_function(env, global, function_trampoline_load_from_file, 1, argv, &return_value);

		node_loader_impl_exception(env, status);

		/* Check return value */
		napi_valuetype return_valuetype;

		status = napi_typeof(env, return_value, &return_valuetype);

		node_loader_impl_exception(env, status);

		if (return_valuetype != napi_null)
		{
			/* Make handle persistent */
			status = napi_create_reference(env, return_value, 1, &async_data->handle_ref);

			node_loader_impl_exception(env, status);
		}
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);

	/* Signal load from file condition */
	uv_cond_signal(&async_data->node_impl->cond);

	uv_mutex_unlock(&async_data->node_impl->mutex);
}

void node_loader_impl_async_load_from_memory(uv_async_t * async)
{
	loader_impl_async_load_from_memory async_data;

	napi_env env;
	napi_value function_table_object;

	const char load_from_memory_str[] = "load_from_memory";
	napi_value load_from_memory_str_value;

	bool result = false;

	napi_handle_scope handle_scope;

	async_data = static_cast<loader_impl_async_load_from_memory>(async->data);

	/* Lock node implementation mutex */
	uv_mutex_lock(&async_data->node_impl->mutex);

	/* Get environment reference */
	env = async_data->node_impl->env;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Get function table object from reference */
	status = napi_get_reference_value(env, async_data->node_impl->function_table_object_ref, &function_table_object);

	node_loader_impl_exception(env, status);

	/* Create function string */
	status = napi_create_string_utf8(env, load_from_memory_str, sizeof(load_from_memory_str) - 1, &load_from_memory_str_value);

	node_loader_impl_exception(env, status);

	/* Check if exists in the table */
	status = napi_has_own_property(env, function_table_object, load_from_memory_str_value, &result);

	node_loader_impl_exception(env, status);

	if (result == true)
	{
		napi_value function_trampoline_load_from_memory;
		napi_valuetype valuetype;
		napi_value argv[3];

		status = napi_get_named_property(env, function_table_object, load_from_memory_str, &function_trampoline_load_from_memory);

		node_loader_impl_exception(env, status);

		status = napi_typeof(env, function_trampoline_load_from_memory, &valuetype);

		node_loader_impl_exception(env, status);

		if (valuetype != napi_function)
		{
			napi_throw_type_error(env, nullptr, "Invalid function load_from_memory in function table object");
		}

		/* Define parameters */
		status = napi_create_string_utf8(env, async_data->name, strlen(async_data->name), &argv[0]);

		node_loader_impl_exception(env, status);

		status = napi_create_string_utf8(env, async_data->buffer, async_data->size - 1, &argv[1]);

		node_loader_impl_exception(env, status);

		status = napi_create_object(env, &argv[2]);

		node_loader_impl_exception(env, status);

		/* Call to load from memory function */
		napi_value global, return_value;

		status = napi_get_reference_value(env, async_data->node_impl->global_ref, &global);

		node_loader_impl_exception(env, status);

		status = napi_call_function(env, global, function_trampoline_load_from_memory, 3, argv, &return_value);

		node_loader_impl_exception(env, status);

		/* Check return value */
		napi_valuetype return_valuetype;

		status = napi_typeof(env, return_value, &return_valuetype);

		node_loader_impl_exception(env, status);

		if (return_valuetype != napi_null)
		{
			/* Make handle persistent */
			status = napi_create_reference(env, return_value, 1, &async_data->handle_ref);

			node_loader_impl_exception(env, status);
		}
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);

	/* Signal load from memory condition */
	uv_cond_signal(&async_data->node_impl->cond);

	uv_mutex_unlock(&async_data->node_impl->mutex);
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

	async_data = static_cast<loader_impl_async_clear>(async->data);

	/* Lock node implementation mutex */
	uv_mutex_lock(&async_data->node_impl->mutex);

	/* Get environment reference */
	env = async_data->node_impl->env;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);
	/* Get function table object from reference */
	status = napi_get_reference_value(env, async_data->node_impl->function_table_object_ref, &function_table_object);

	node_loader_impl_exception(env, status);

	/* Create function string */
	status = napi_create_string_utf8(env, clear_str, sizeof(clear_str) - 1, &clear_str_value);

	node_loader_impl_exception(env, status);

	/* Check if exists in the table */
	status = napi_has_own_property(env, function_table_object, clear_str_value, &result);

	node_loader_impl_exception(env, status);

	if (result == true)
	{
		napi_value function_trampoline_clear;
		napi_valuetype valuetype;
		napi_value argv[1];

		status = napi_get_named_property(env, function_table_object, clear_str, &function_trampoline_clear);

		node_loader_impl_exception(env, status);

		status = napi_typeof(env, function_trampoline_clear, &valuetype);

		node_loader_impl_exception(env, status);

		if (valuetype != napi_function)
		{
			napi_throw_type_error(env, nullptr, "Invalid function clear in function table object");
		}

		/* Define parameters */
		status = napi_get_reference_value(env, async_data->handle_ref, &argv[0]);

		node_loader_impl_exception(env, status);

		/* Call to load from file function */
		napi_value global, clear_return;

		status = napi_get_reference_value(env, async_data->node_impl->global_ref, &global);

		node_loader_impl_exception(env, status);

		status = napi_call_function(env, global, function_trampoline_clear, 1, argv, &clear_return);

		node_loader_impl_exception(env, status);
	}

	/* Clear handle persistent reference */
	status = napi_reference_unref(env, async_data->handle_ref, &ref_count);

	node_loader_impl_exception(env, status);

	if (ref_count != 0)
	{
		/* TODO: Error handling */
	}

	status = napi_delete_reference(env, async_data->handle_ref);

	node_loader_impl_exception(env, status);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);

	/* Signal clear condition */
	uv_cond_signal(&async_data->node_impl->cond);

	uv_mutex_unlock(&async_data->node_impl->mutex);
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

	async_data = static_cast<loader_impl_async_discover>(async->data);

	/* Lock node implementation mutex */
	uv_mutex_lock(&async_data->node_impl->mutex);

	/* Get environment reference */
	env = async_data->node_impl->env;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Get function table object from reference */
	status = napi_get_reference_value(env, async_data->node_impl->function_table_object_ref, &function_table_object);

	node_loader_impl_exception(env, status);

	/* Create function string */
	status = napi_create_string_utf8(env, discover_str, sizeof(discover_str) - 1, &discover_str_value);

	node_loader_impl_exception(env, status);

	/* Check if exists in the table */
	status = napi_has_own_property(env, function_table_object, discover_str_value, &result);

	node_loader_impl_exception(env, status);

	if (result == true)
	{
		napi_value function_trampoline_discover;
		napi_valuetype valuetype;
		napi_value argv[1];

		status = napi_get_named_property(env, function_table_object, discover_str, &function_trampoline_discover);

		node_loader_impl_exception(env, status);

		status = napi_typeof(env, function_trampoline_discover, &valuetype);

		node_loader_impl_exception(env, status);

		if (valuetype != napi_function)
		{
			napi_throw_type_error(env, nullptr, "Invalid function discover in function table object");
		}

		/* Define parameters */
		status = napi_get_reference_value(env, async_data->handle_ref, &argv[0]);

		node_loader_impl_exception(env, status);

		/* Call to load from file function */
		napi_value global, discover_map;

		status = napi_get_reference_value(env, async_data->node_impl->global_ref, &global);

		node_loader_impl_exception(env, status);

		status = napi_call_function(env, global, function_trampoline_discover, 1, argv, &discover_map);

		node_loader_impl_exception(env, status);

		/* Convert return value (discover object) to context */
		napi_value function_names;
		uint32_t function_names_length;

		status = napi_get_property_names(env, discover_map, &function_names);

		node_loader_impl_exception(env, status);

		status = napi_get_array_length(env, function_names, &function_names_length);

		node_loader_impl_exception(env, status);

		for (uint32_t index = 0; index < function_names_length; ++index)
		{
			napi_value function_name;
			size_t function_name_length;
			char * function_name_str = NULL;

			status = napi_get_element(env, function_names, index, &function_name);

			node_loader_impl_exception(env, status);

			status = napi_get_value_string_utf8(env, function_name, NULL, 0, &function_name_length);

			node_loader_impl_exception(env, status);

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

				node_loader_impl_exception(env, status);

				/* Get function descriptor */
				status = napi_get_named_property(env, discover_map, function_name_str, &function_descriptor);

				node_loader_impl_exception(env, status);

				/* Get function pointer */
				status = napi_get_named_property(env, function_descriptor, "ptr", &function_ptr);

				node_loader_impl_exception(env, status);

				/* Check function pointer type */
				status = napi_typeof(env, function_ptr, &valuetype);

				node_loader_impl_exception(env, status);

				if (valuetype != napi_function)
				{
					napi_throw_type_error(env, nullptr, "Invalid NodeJS function");
				}

				/* Get function signature */
				status = napi_get_named_property(env, function_descriptor, "signature", &function_sig);

				node_loader_impl_exception(env, status);

				/* Check function pointer type */
				status = napi_typeof(env, function_sig, &valuetype);

				node_loader_impl_exception(env, status);

				if (valuetype != napi_object)
				{
					napi_throw_type_error(env, nullptr, "Invalid NodeJS signature");
				}

				/* Get signature length */
				status = napi_get_array_length(env, function_sig, &function_sig_length);

				node_loader_impl_exception(env, status);

				/* Create node function */
				loader_impl_node_function node_func = static_cast<loader_impl_node_function>(malloc(sizeof(struct loader_impl_node_function_type)));

				/* Create reference to function pointer */
				status = napi_create_reference(env, function_ptr, 1, &node_func->func_ref);

				node_loader_impl_exception(env, status);

				node_func->node_impl = async_data->node_impl;

				/* Create function */
				function f = function_create(function_name_str, (size_t)function_sig_length, node_func, &function_node_singleton);

				if (f != NULL)
				{
					signature s = function_signature(f);
					scope sp = context_scope(async_data->ctx);

					for (uint32_t arg_index = 0; arg_index < function_sig_length; ++arg_index)
					{
						napi_value parameter_name;
						size_t parameter_name_length;
						char * parameter_name_str = NULL;

						/* Get signature parameter name */
						status = napi_get_element(env, function_sig, arg_index, &parameter_name);

						node_loader_impl_exception(env, status);

						/* Get parameter name string length */
						status = napi_get_value_string_utf8(env, parameter_name, NULL, 0, &parameter_name_length);

						node_loader_impl_exception(env, status);

						if (parameter_name_length > 0)
						{
							parameter_name_str = static_cast<char *>(malloc(sizeof(char) * (parameter_name_length + 1)));
						}

						/* Get parameter name string */
						status = napi_get_value_string_utf8(env, parameter_name, parameter_name_str, parameter_name_length + 1, &parameter_name_length);

						node_loader_impl_exception(env, status);

						signature_set(s, (size_t)arg_index, parameter_name_str, NULL);
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

	node_loader_impl_exception(env, status);

	/* Signal discover condition */
	uv_cond_signal(&async_data->node_impl->cond);

	uv_mutex_unlock(&async_data->node_impl->mutex);
}

void * node_loader_impl_register(void * node_impl_ptr, void * env_ptr, void * function_table_object_ptr)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(node_impl_ptr);

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl->mutex);

	napi_env env = static_cast<napi_env>(env_ptr);
	napi_value function_table_object = static_cast<napi_value>(function_table_object_ptr);

	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		const char test_str[] = "test";
		napi_value test_str_value;
	#endif

	bool result = false;

	napi_status status;

	napi_value global;

	node_impl->env = env;

	/* Make global object persistent */
	status = napi_get_global(env, &global);

	node_loader_impl_exception(env, status);

	status = napi_create_reference(env, global, 1, &node_impl->global_ref);

	node_loader_impl_exception(env, status);

	/* Make function table object persistent */
	status = napi_create_reference(env, function_table_object, 1, &node_impl->function_table_object_ref);

	node_loader_impl_exception(env, status);

	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	{
		/* Retrieve test function from object table */
		status = napi_create_string_utf8(env, test_str, sizeof(test_str) - 1, &test_str_value);

		node_loader_impl_exception(env, status);

		status = napi_has_own_property(env, function_table_object, test_str_value, &result);

		node_loader_impl_exception(env, status);

		if (result == true)
		{
			napi_value function_trampoline_test;
			napi_valuetype valuetype;

			status = napi_get_named_property(env, function_table_object, test_str, &function_trampoline_test);

			node_loader_impl_exception(env, status);

			status = napi_typeof(env, function_trampoline_test, &valuetype);

			node_loader_impl_exception(env, status);

			if (valuetype != napi_function)
			{
				napi_throw_type_error(env, nullptr, "Invalid function test in function table object");
			}

			/* Call to test function */
			napi_value return_value;

			status = napi_call_function(env, global, function_trampoline_test, 0, nullptr, &return_value);

			node_loader_impl_exception(env, status);
		}
	}
	#endif

	/* Signal start condition */
	uv_cond_signal(&node_impl->cond);

	uv_mutex_unlock(&node_impl->mutex);

	/* TODO: Return */
	return NULL;
}

void node_loader_impl_thread(void * data)
{
	loader_impl_node node_impl = *(static_cast<loader_impl_node *>(data));

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl->mutex);

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
		/* TODO: Make logs thread safe */
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

	node_impl->thread_loop = uv_default_loop(); /* TODO: napi_get_uv_event_loop() */

	/* Initialize initialize signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_initialize, &node_loader_impl_async_initialize);

	/* Initialize load from file signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_load_from_file, &node_loader_impl_async_load_from_file);

	/* Initialize load from memory signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_load_from_memory, &node_loader_impl_async_load_from_memory);

	/* Initialize clear signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_clear, &node_loader_impl_async_clear);

	/* Initialize discover signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_discover, &node_loader_impl_async_discover);

	/* Initialize function call signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_func_call, &node_loader_impl_async_func_call);

	/* Initialize function destroy signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_func_destroy, &node_loader_impl_async_func_destroy);

	/* Initialize future await signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_future_await, &node_loader_impl_async_future_await);

	/* Initialize destroy signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_destroy, &node_loader_impl_async_destroy);

	/* Unlock node implementation mutex */
	uv_mutex_unlock(&node_impl->mutex);

	/* Start NodeJS runtime */
	int result = node::Start(argc, reinterpret_cast<char **>(argv));

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl->mutex);

	node_impl->result = result;

	/* Unlock node implementation mutex */
	uv_mutex_unlock(&node_impl->mutex);
}

loader_impl_data node_loader_impl_initialize(loader_impl impl, configuration config, loader_host host)
{
	loader_impl_node node_impl;

	(void)impl;
	(void)config;

	log_copy(host->log);

	node_impl = new loader_impl_node_type();

	if (node_impl == nullptr)
	{
		return NULL;
	}

	/* Duplicate stdin, stdout, stderr */
	node_impl->stdin_copy = dup(STDIN_FILENO);
	node_impl->stdout_copy = dup(STDOUT_FILENO);
	node_impl->stderr_copy = dup(STDERR_FILENO);

	/* Initialize trampolines */
	node_impl->resolve_callback = NULL;
	node_impl->reject_callback = NULL;

	/* Initialize syncronization */
	if (uv_cond_init(&node_impl->cond) != 0)
	{
		delete node_impl;

		return NULL;
	}

	if (uv_mutex_init(&node_impl->mutex) != 0)
	{
		delete node_impl;

		return NULL;
	}

	/* Initialize execution result */
	node_impl->result = 1;

	/* Create NodeJS thread */
	if (uv_thread_create(&node_impl->thread_id, node_loader_impl_thread, &node_impl) != 0)
	{
		delete node_impl;

		return NULL;
	}

	/* Wait until start has been launch */
	uv_mutex_lock(&node_impl->mutex);

	uv_cond_wait(&node_impl->cond, &node_impl->mutex);

	uv_mutex_unlock(&node_impl->mutex);

	/* Set initialize async data */
	struct loader_impl_async_initialize_type async_initialize =
	{
		node_impl,
		0
	};

	/* Initialize node loader entry point */
	uv_mutex_lock(&node_impl->mutex);

	node_impl->async_initialize.data = static_cast<loader_impl_async_initialize>(&async_initialize);

	/* Execute initialize async callback */
	uv_async_send(&node_impl->async_initialize);

	/* Wait until script has been loaded */
	uv_cond_wait(&node_impl->cond, &node_impl->mutex);

	uv_mutex_unlock(&node_impl->mutex);

	if (async_initialize.result != 0)
	{
		/* TODO: Handle error properly */
	}

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

	uv_mutex_lock(&node_impl->mutex);

	node_impl->async_load_from_file.data = static_cast<void *>(&async_data);

	/* Execute load from file async callback */
	uv_async_send(&node_impl->async_load_from_file);

	/* Wait until module is loaded */
	uv_cond_wait(&node_impl->cond, &node_impl->mutex);

	uv_mutex_unlock(&node_impl->mutex);

	return static_cast<loader_handle>(async_data.handle_ref);
}

loader_handle node_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	if (node_impl == NULL)
	{
		return NULL;
	}

	struct loader_impl_async_load_from_memory_type async_data =
	{
		node_impl,
		name,
		buffer,
		size,
		NULL
	};

	uv_mutex_lock(&node_impl->mutex);

	node_impl->async_load_from_memory.data = static_cast<void *>(&async_data);

	/* Execute load from memory async callback */
	uv_async_send(&node_impl->async_load_from_memory);

	/* Wait until module is loaded */
	uv_cond_wait(&node_impl->cond, &node_impl->mutex);

	uv_mutex_unlock(&node_impl->mutex);

	return static_cast<loader_handle>(async_data.handle_ref);
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

	uv_mutex_lock(&node_impl->mutex);

	node_impl->async_clear.data = static_cast<void *>(&async_data);

	/* Execute clear async callback */
	uv_async_send(&node_impl->async_clear);

	/* Wait until module is cleared */
	uv_cond_wait(&node_impl->cond, &node_impl->mutex);

	uv_mutex_unlock(&node_impl->mutex);

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

	uv_mutex_lock(&node_impl->mutex);

	node_impl->async_discover.data = static_cast<void *>(&async_data);

	/* Execute discover async callback */
	uv_async_send(&node_impl->async_discover);

	/* Wait until module is discovered */
	uv_cond_wait(&node_impl->cond, &node_impl->mutex);

	uv_mutex_unlock(&node_impl->mutex);

	return 0;
}

void node_loader_impl_async_destroy(uv_async_t * async)
{
	loader_impl_node node_impl;

	uint32_t ref_count = 0;

	napi_status status;

	node_impl = *(static_cast<loader_impl_node *>(async->data));

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl->mutex);

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

		node_loader_impl_exception(env, status);

		/* Get function table object from reference */
		status = napi_get_reference_value(env, node_impl->function_table_object_ref, &function_table_object);

		node_loader_impl_exception(env, status);

		/* Retrieve destroy function from object table */
		status = napi_create_string_utf8(env, destroy_str, sizeof(destroy_str) - 1, &destroy_str_value);

		node_loader_impl_exception(env, status);

		status = napi_has_own_property(env, function_table_object, destroy_str_value, &result);

		node_loader_impl_exception(env, status);

		if (result == true)
		{
			napi_value function_trampoline_destroy;
			napi_valuetype valuetype;

			status = napi_get_named_property(env, function_table_object, destroy_str, &function_trampoline_destroy);

			node_loader_impl_exception(env, status);

			status = napi_typeof(env, function_trampoline_destroy, &valuetype);

			node_loader_impl_exception(env, status);

			if (valuetype != napi_function)
			{
				napi_throw_type_error(env, nullptr, "Invalid function destroy in function table object");
			}

			/* Call to destroy function */
			napi_value global, return_value;

			status = napi_get_global(env, &global);

			node_loader_impl_exception(env, status);

			status = napi_call_function(env, global, function_trampoline_destroy, 0, nullptr, &return_value);

			node_loader_impl_exception(env, status);
		}

		/* Close scope */
		status = napi_close_handle_scope(env, handle_scope);

		node_loader_impl_exception(env, status);
	}

	/* Clear persistent references */
	status = napi_reference_unref(node_impl->env, node_impl->resolve_trampoline_ref, &ref_count);

	node_loader_impl_exception(node_impl->env, status);

	if (ref_count != 0)
	{
		/* TODO: Error handling */
	}

	status = napi_delete_reference(node_impl->env, node_impl->resolve_trampoline_ref);

	node_loader_impl_exception(node_impl->env, status);

	status = napi_reference_unref(node_impl->env, node_impl->reject_trampoline_ref, &ref_count);

	node_loader_impl_exception(node_impl->env, status);

	if (ref_count != 0)
	{
		/* TODO: Error handling */
	}

	status = napi_delete_reference(node_impl->env, node_impl->reject_trampoline_ref);

	node_loader_impl_exception(node_impl->env, status);

	status = napi_reference_unref(node_impl->env, node_impl->global_ref, &ref_count);

	node_loader_impl_exception(node_impl->env, status);

	if (ref_count != 0)
	{
		/* TODO: Error handling */
	}

	status = napi_delete_reference(node_impl->env, node_impl->global_ref);

	node_loader_impl_exception(node_impl->env, status);

	status = napi_reference_unref(node_impl->env, node_impl->function_table_object_ref, &ref_count);

	node_loader_impl_exception(node_impl->env, status);

	if (ref_count != 0)
	{
		/* TODO: Error handling */
	}

	status = napi_delete_reference(node_impl->env, node_impl->function_table_object_ref);

	node_loader_impl_exception(node_impl->env, status);

	/* Destroy async objects */
	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_destroy), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_func_destroy), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_func_call), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_future_await), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_discover), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_clear), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_load_from_file), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_load_from_memory), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_initialize), NULL);

	/*  Stop event loop */
	uv_stop(node_impl->thread_loop);

	/* Clear event loop */
	uv_walk(node_impl->thread_loop, node_loader_impl_walk, NULL);

	while (uv_run(node_impl->thread_loop, UV_RUN_DEFAULT) != 0);

	/* Destroy node loop */
	if (uv_loop_alive(node_impl->thread_loop) != 0)
	{
		/* TODO: Make logs thread safe */
		/* log_write("metacall", LOG_LEVEL_ERROR, "NodeJS event loop should not be alive"); */
	}

	/* TODO: Check how to delete properly all handles */
	if (uv_loop_close(node_impl->thread_loop) == UV_EBUSY)
	{
		/* TODO: Make logs thread safe */
		/* log_write("metacall", LOG_LEVEL_ERROR, "NodeJS event loop should not be busy"); */
	}

	/* Signal destroy condition */
	uv_cond_signal(&node_impl->cond);

	uv_mutex_unlock(&node_impl->mutex);
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

	uv_mutex_lock(&node_impl->mutex);

	node_impl->async_destroy.data = static_cast<void *>(&node_impl);

	/* Execute destroy async callback */
	uv_async_send(&node_impl->async_destroy);

	/* Wait until node is destroyed */
	uv_cond_wait(&node_impl->cond, &node_impl->mutex);

	uv_mutex_unlock(&node_impl->mutex);

	/* Clear condition syncronization object */
	uv_cond_destroy(&node_impl->cond);

	/* Wait for node thread to finish */
	uv_thread_join(&node_impl->thread_id);

	/* Clear mutex syncronization object */
	uv_mutex_destroy(&node_impl->mutex);

	/* Print NodeJS execution result */
	log_write("metacall", LOG_LEVEL_INFO, "NodeJS execution return status %d", node_impl->result);

	/* Restore stdin, stdout, stderr */
	dup2(node_impl->stdin_copy, STDIN_FILENO);
	dup2(node_impl->stdout_copy, STDOUT_FILENO);
	dup2(node_impl->stderr_copy, STDERR_FILENO);

	delete node_impl;

	return 0;
}
