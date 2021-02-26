/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#if defined(__POSIX__)
#	include <signal.h>
#endif

#ifdef __linux__
#	include <elf.h>
#	ifdef __LP64__
#		define Elf_auxv_t Elf64_auxv_t
#	else
#		define Elf_auxv_t Elf32_auxv_t
#	endif /* __LP64__ */
	extern char** environ;
#endif /* __linux__ */

#include <node_loader/node_loader_impl.h>
#include <node_loader/node_loader_port.h>
#include <node_loader/node_loader_trampoline.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_future.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

/* TODO: Make logs thread safe */
#include <log/log.h>

#include <metacall/metacall.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <new>
#include <string>
#include <fstream>
#include <streambuf>
#include <atomic>
#include <thread>

/* Disable warnings from V8 and NodeJS */
#if defined(_MSC_VER) || defined(__clang__)
#	pragma warning(push)
#	pragma warning(disable: 4100)
#	pragma warning(disable: 4275)
#	pragma warning(disable: 4251)
#elif defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <node.h>
#include <node_api.h>

#include <libplatform/libplatform.h>
#include <v8.h> /* version: 6.2.414.50 */

#ifdef ENABLE_DEBUGGER_SUPPORT
#	include <v8-debug.h>
#endif /* ENALBLE_DEBUGGER_SUPPORT */

#include <uv.h>

/* Disable warnings from V8 and NodeJS */
#if defined(_MSC_VER) || defined(__clang__)
#	pragma warning(pop)
#elif defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif

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

namespace node
{
	extern bool linux_at_secure;
}

#define NODE_GET_EVENT_LOOP \
	(NAPI_VERSION >= 2) && \
	((NODE_MAJOR_VERSION == 8 && NODE_MINOR_VERSION >= 10) || \
	(NODE_MAJOR_VERSION == 9 && NODE_MINOR_VERSION >= 3) || \
	(NODE_MAJOR_VERSION >= 10))


#if !defined(NODE_MAJOR_VERSION) || NODE_MAJOR_VERSION < 10
#	error "NodeJS version not supported"
#endif

struct loader_impl_async_initialize_safe_type;
typedef struct loader_impl_async_initialize_safe_type * loader_impl_async_initialize_safe;

struct loader_impl_async_load_from_file_safe_type;
typedef struct loader_impl_async_load_from_file_safe_type * loader_impl_async_load_from_file_safe;

struct loader_impl_async_load_from_memory_safe_type;
typedef struct loader_impl_async_load_from_memory_safe_type * loader_impl_async_load_from_memory_safe;

struct loader_impl_async_clear_safe_type;
typedef struct loader_impl_async_clear_safe_type * loader_impl_async_clear_safe;

struct loader_impl_async_discover_function_safe_type;
typedef struct loader_impl_async_discover_function_safe_type * loader_impl_async_discover_function_safe;

struct loader_impl_async_discover_safe_type;
typedef struct loader_impl_async_discover_safe_type * loader_impl_async_discover_safe;

struct loader_impl_async_func_call_safe_type;
typedef struct loader_impl_async_func_call_safe_type * loader_impl_async_func_call_safe;

struct loader_impl_async_func_await_safe_type;
typedef struct loader_impl_async_func_await_safe_type * loader_impl_async_func_await_safe;

struct loader_impl_async_func_destroy_safe_type;
typedef struct loader_impl_async_func_destroy_safe_type * loader_impl_async_func_destroy_safe;

struct loader_impl_async_future_delete_safe_type;
typedef struct loader_impl_async_future_delete_safe_type * loader_impl_async_future_delete_safe;

struct loader_impl_async_destroy_safe_type;
typedef struct loader_impl_async_destroy_safe_type * loader_impl_async_destroy_safe;

struct loader_impl_node_type
{
	/* TODO: The current implementation may not support multi-isolate environments. We should test it. */
	napi_env env; /* Used for storing environment for reentrant calls */
	napi_ref global_ref; /* Store global reference */
	napi_ref function_table_object_ref; /* Store function table reference registered by the trampoline */

	napi_value initialize_safe_ptr;
	loader_impl_async_initialize_safe initialize_safe;
	napi_threadsafe_function threadsafe_initialize;

	napi_value load_from_file_safe_ptr;
	loader_impl_async_load_from_file_safe load_from_file_safe;
	napi_threadsafe_function threadsafe_load_from_file;

	napi_value load_from_memory_safe_ptr;
	loader_impl_async_load_from_memory_safe load_from_memory_safe;
	napi_threadsafe_function threadsafe_load_from_memory;

	napi_value clear_safe_ptr;
	loader_impl_async_clear_safe clear_safe;
	napi_threadsafe_function threadsafe_clear;

	napi_value discover_safe_ptr;
	loader_impl_async_discover_safe discover_safe;
	napi_threadsafe_function threadsafe_discover;

	napi_value func_call_safe_ptr;
	loader_impl_async_func_call_safe func_call_safe;
	napi_threadsafe_function threadsafe_func_call;

	napi_value func_await_safe_ptr;
	loader_impl_async_func_await_safe func_await_safe;
	napi_threadsafe_function threadsafe_func_await;

	napi_value func_destroy_safe_ptr;
	loader_impl_async_func_destroy_safe func_destroy_safe;
	napi_threadsafe_function threadsafe_func_destroy;

	napi_value future_delete_safe_ptr;
	loader_impl_async_future_delete_safe future_delete_safe;
	napi_threadsafe_function threadsafe_future_delete;

	napi_value destroy_safe_ptr;
	loader_impl_async_destroy_safe destroy_safe;
	napi_threadsafe_function threadsafe_destroy;

	uv_thread_t thread;
	uv_loop_t * thread_loop;

	uv_mutex_t mutex;
	uv_cond_t cond;
	std::atomic_bool locked;

	int stdin_copy;
	int stdout_copy;
	int stderr_copy;

	#ifdef __ANDROID__
		int pfd[2];
		uv_thread_t thread_log_id;
	#endif

	int result;
	const char * error_message;

	/* TODO: This implementation won't work for multi-isolate environments. We should test it. */
	std::thread::id js_thread_id;
};

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

struct loader_impl_async_initialize_safe_type
{
	loader_impl_node node_impl;
	int result;
};

struct loader_impl_async_load_from_file_safe_type
{
	loader_impl_node node_impl;
	const loader_naming_path * paths;
	size_t size;
	napi_ref handle_ref;
};

struct loader_impl_async_load_from_memory_safe_type
{
	loader_impl_node node_impl;
	const char * name;
	const char * buffer;
	size_t size;
	napi_ref handle_ref;
};

struct loader_impl_async_clear_safe_type
{
	loader_impl_node node_impl;
	napi_ref handle_ref;
};

struct loader_impl_async_discover_function_safe_type
{
	loader_impl impl;
	loader_impl_node node_impl;
	napi_value func;
};

struct loader_impl_async_discover_safe_type
{
	loader_impl impl;
	loader_impl_node node_impl;
	napi_ref handle_ref;
	context ctx;
};

struct loader_impl_async_func_call_safe_type
{
	loader_impl_node node_impl;
	function func;
	loader_impl_node_function node_func;
	void ** args;
	size_t size;
	napi_value recv;
	function_return ret;
};

struct loader_impl_async_func_await_safe_type
{
	loader_impl_node node_impl;
	function func;
	loader_impl_node_function node_func;
	void ** args;
	size_t size;
	function_resolve_callback resolve_callback;
	function_reject_callback reject_callback;
	void * context;
	napi_value recv;
	function_return ret;
};

typedef napi_value (*function_resolve_trampoline)(loader_impl_node, napi_env, function_resolve_callback, napi_value, napi_value, void *);
typedef napi_value (*function_reject_trampoline)(loader_impl_node, napi_env, function_reject_callback, napi_value, napi_value, void *);

typedef struct loader_impl_async_func_await_trampoline_type
{
	loader_impl_node node_loader;
	function_resolve_trampoline resolve_trampoline;
	function_reject_trampoline reject_trampoline;
	function_resolve_callback resolve_callback;
	function_resolve_callback reject_callback;
	void * context;

} * loader_impl_async_func_await_trampoline;

struct loader_impl_async_func_destroy_safe_type
{
	loader_impl_node node_impl;
	loader_impl_node_function node_func;
};

struct loader_impl_async_future_delete_safe_type
{
	loader_impl_node node_impl;
	future f;
	loader_impl_node_future node_future;
};

struct loader_impl_async_destroy_safe_type
{
	loader_impl_node node_impl;
};

typedef struct loader_impl_thread_type
{
	loader_impl_node node_impl;
	configuration config;

} * loader_impl_thread;

typedef struct loader_impl_napi_to_value_callback_closure_type
{
	value func;
	loader_impl_node node_impl;

} * loader_impl_napi_to_value_callback_closure;

/* Type conversion */
static void node_loader_impl_napi_to_value_callback_finalizer(value v, void * data);

static napi_value node_loader_impl_napi_to_value_callback(napi_env env, napi_callback_info info);

/* Function */
static int function_node_interface_create(function func, function_impl impl);

static function_return function_node_interface_invoke(function func, function_impl impl, function_args args, size_t size);

static function_return function_node_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void * context);

static void function_node_interface_destroy(function func, function_impl impl);

static function_interface function_node_singleton(void);

/* Future */
static int future_node_interface_create(future f, future_impl impl);

static void future_node_interface_destroy(future f, future_impl impl);

static future_interface future_node_singleton(void);

/* JavaScript Thread Safe */
static void node_loader_impl_initialize_safe(napi_env env, loader_impl_async_initialize_safe initialize_safe);

static napi_value node_loader_impl_async_initialize_safe(napi_env env, napi_callback_info info);

static void node_loader_impl_func_call_safe(napi_env env, loader_impl_async_func_call_safe func_call_safe);

static napi_value node_loader_impl_async_func_call_safe(napi_env env, napi_callback_info info);

static void node_loader_impl_func_await_safe(napi_env env, loader_impl_async_func_await_safe func_await_safe);

static napi_value node_loader_impl_async_func_await_safe(napi_env env, napi_callback_info info);

static void node_loader_impl_func_destroy_safe(napi_env env, loader_impl_async_func_destroy_safe func_destroy_safe);

static napi_value node_loader_impl_async_func_destroy_safe(napi_env env, napi_callback_info info);

static void node_loader_impl_future_delete_safe(napi_env env, loader_impl_async_future_delete_safe future_delete_safe);

static napi_value node_loader_impl_async_future_delete_safe(napi_env env, napi_callback_info info);

static void node_loader_impl_load_from_file_safe(napi_env env, loader_impl_async_load_from_file_safe load_from_file_safe);

static napi_value node_loader_impl_async_load_from_file_safe(napi_env env, napi_callback_info info);

static void node_loader_impl_load_from_memory_safe(napi_env env, loader_impl_async_load_from_memory_safe load_from_memory_safe);

static napi_value node_loader_impl_async_load_from_memory_safe(napi_env env, napi_callback_info info);

static void node_loader_impl_clear_safe(napi_env env, loader_impl_async_clear_safe clear_safe);

static napi_value node_loader_impl_async_clear_safe(napi_env env, napi_callback_info info);

static value node_loader_impl_discover_function_safe(napi_env env, loader_impl_async_discover_function_safe discover_function_safe);

static void node_loader_impl_discover_safe(napi_env env, loader_impl_async_discover_safe discover_safe);

static napi_value node_loader_impl_async_discover_safe(napi_env env, napi_callback_info info);

static void node_loader_impl_destroy_safe(napi_env env, loader_impl_async_destroy_safe destroy_safe);

static napi_value node_loader_impl_async_destroy_safe(napi_env env, napi_callback_info info);

/* Loader */
static void * node_loader_impl_register(void * node_impl_ptr, void * env_ptr, void * function_table_object_ptr);

static void node_loader_impl_thread(void * data);

#ifdef __ANDROID__
	static void node_loader_impl_thread_log(void * data);
#endif

static void node_loader_impl_walk(uv_handle_t * handle, void * data);

/* -- Methods -- */

void node_loader_impl_exception(napi_env env, napi_status status)
{
	if (status != napi_ok)
	{
		if (status != napi_pending_exception)
		{
			const napi_extended_error_info * error_info = NULL;

			bool pending;

			napi_get_last_error_info(env, &error_info);

			napi_is_exception_pending(env, &pending);

			const char * message = (error_info != NULL && error_info->error_message != NULL) ? error_info->error_message : "Error message not available";

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
			printf("NodeJS Loader Error: %s\n", str);
			fflush(stdout);

			/* Meanwhile, throw it again */
			status = napi_throw_error(env, nullptr, str);

			node_loader_impl_exception(env, status);

			free(str);
		}
	}
}

void node_loader_impl_finalizer(napi_env env, napi_value v, void * data)
{
	napi_status status;

	if (value_type_id(data) == TYPE_NULL)
	{
		value_type_destroy(data);
		return;
	}

	auto finalizer = [](napi_env, void * finalize_data, void *)
	{
		value_type_destroy(finalize_data);
	};

	// Create a finalizer for the value
	#if (NAPI_VERSION < 5)
	{
		napi_value symbol, external;

		status = napi_create_symbol(env, nullptr, &symbol);

		node_loader_impl_exception(env, status);

		status = napi_create_external(env, data, finalizer, nullptr, &external);

		node_loader_impl_exception(env, status);

		napi_property_descriptor desc =
		{
			nullptr,
			symbol,
			nullptr,
			nullptr,
			nullptr,
			external,
			napi_default,
			nullptr
		};

		status = napi_define_properties(env, v, 1, &desc);

		node_loader_impl_exception(env, status);
	}
	#else // NAPI_VERSION >= 5
	{
		status = napi_add_finalizer(env, v, data, finalizer, nullptr, nullptr);

		node_loader_impl_exception(env, status);
	}
	#endif
}

value node_loader_impl_napi_to_value(loader_impl_node node_impl, napi_env env, napi_value recv, napi_value v)
{
	value ret = NULL;

	napi_valuetype valuetype;

	napi_status status = napi_typeof(env, v, &valuetype);

	node_loader_impl_exception(env, status);

	if (valuetype == napi_undefined || valuetype == napi_null)
	{
		/* TODO: Review this, type null will be lost due to mapping of two N-API types into one metacall type */
		ret = value_create_null();
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
		napi_throw_error(env, NULL, "NodeJS Loader symbol is not implemented");
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
				array_value[iterator] = node_loader_impl_napi_to_value(node_impl, env, recv, element);
			}
		}
		else if (napi_is_buffer(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
			napi_throw_error(env, NULL, "NodeJS Loader buffer is not implemented");
		}
		else if (napi_is_error(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
			napi_throw_error(env, NULL, "NodeJS Loader error is not implemented");
		}
		else if (napi_is_typedarray(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
			napi_throw_error(env, NULL, "NodeJS Loader typed array is not implemented");
		}
		else if (napi_is_dataview(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
			napi_throw_error(env, NULL, "NodeJS Loader data view is not implemented");
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
					tupla[1] = node_loader_impl_napi_to_value(node_impl, env, recv, element);
				}
			}

		}
	}
	else if (valuetype == napi_function)
	{
		struct loader_impl_async_discover_function_safe_type discover_function_safe =
		{
			/* TODO: */
			NULL,
			/* impl, */
			node_impl,
			v
		};

		/* Discover and create the function */
		return node_loader_impl_discover_function_safe(env, &discover_function_safe);
	}
	else if (valuetype == napi_external)
	{
		/* Returns the previously allocated copy */
		void * c = NULL;

		status = napi_get_value_external(env, v, &c);

		node_loader_impl_exception(env, status);

		return c;
	}

	return ret;
}

void node_loader_impl_napi_to_value_callback_finalizer(value v, void * data)
{
	loader_impl_napi_to_value_callback_closure closure = static_cast<loader_impl_napi_to_value_callback_closure>(data);

	(void)v;

	delete closure;
}

napi_value node_loader_impl_napi_to_value_callback(napi_env env, napi_callback_info info)
{
	size_t iterator, argc = 0;

	napi_get_cb_info(env, info, &argc, NULL, NULL, NULL);

	napi_value * argv = new napi_value[argc];
	void ** args = new void *[argc];
	napi_value recv;
	loader_impl_napi_to_value_callback_closure closure = NULL;

	napi_get_cb_info(env, info, &argc, argv, &recv, (void **)(&closure));

	/* Set environment */
	closure->node_impl->env = env;

	for (iterator = 0; iterator < argc; ++iterator)
	{
		args[iterator] = node_loader_impl_napi_to_value(closure->node_impl, env, recv, argv[iterator]);

		node_loader_impl_finalizer(env, argv[iterator], args[iterator]);
	}

	void * ret = metacallfv_s(value_to_function(closure->func), args, argc);

	napi_value result = node_loader_impl_value_to_napi(closure->node_impl, env, ret);

	/* Set result finalizer */
	node_loader_impl_finalizer(env, result, ret);

	/* Set closure finalizer */
	value_finalizer(closure->func, &node_loader_impl_napi_to_value_callback_finalizer, closure);

	/* Reset environment */
	// closure->node_impl->env = NULL;

	delete[] argv;
	delete[] args;

	return result;
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
			napi_value element_v = node_loader_impl_value_to_napi(node_impl, env, static_cast<value>(array_value[iterator]));

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
			napi_value element_v = node_loader_impl_value_to_napi(node_impl, env, static_cast<value>(pair_value[1]));

			status = napi_set_named_property(env, v, key, element_v);

			node_loader_impl_exception(env, status);
		}
	}
	else if (id == TYPE_PTR)
	{
		/* Copy value and set the ownership, the old value will be deleted after the call */
		void * c = value_copy(arg_value);

		value_move(arg_value, c);

		status = napi_create_external(env, c, nullptr, nullptr, &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_FUTURE)
	{
		/* TODO: Implement promise properly for await */
		napi_throw_error(env, NULL, "NodeJS Loader future is not implemented");
	}
	else if (id == TYPE_FUNCTION)
	{
		loader_impl_napi_to_value_callback_closure closure = new loader_impl_napi_to_value_callback_closure_type();

		closure->func = value_type_copy(arg_value);
		closure->node_impl = node_impl;

		status = napi_create_function(env, NULL, 0, node_loader_impl_napi_to_value_callback, closure, &v);

		node_loader_impl_exception(env, status);

		node_loader_impl_finalizer(env, v, closure->func);
	}
	else if (id == TYPE_CLASS)
	{
		/* TODO */
		napi_throw_error(env, NULL, "NodeJS Loader class is not implemented");

		/*
		klass cls = value_to_class(arg_value);

		napi_define_class(env, cls->name, NAPI_AUTO_LENGTH, )
		*/
	}
	else if (id == TYPE_OBJECT)
	{
		/* TODO */
		napi_throw_error(env, NULL, "NodeJS Loader object is not implemented");
	}
	else if (id == TYPE_NULL)
	{
		status = napi_get_undefined(env, &v);

		node_loader_impl_exception(env, status);
	}
	else
	{
		napi_throw_error(env, NULL, "NodeJS Loader could not convert the value to N-API");
	}

	return v;
}

void node_loader_impl_env(loader_impl_node node_impl, napi_env env)
{
	node_impl->env = env;
}

int function_node_interface_create(function func, function_impl impl)
{
	loader_impl_node_function node_func = (loader_impl_node_function)impl;

	signature s = function_signature(func);

	const size_t args_size = signature_count(s);

	node_func->argv = static_cast<napi_value *>(malloc(sizeof(napi_value) * args_size));

	return (node_func->argv == NULL);
}

function_return function_node_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	loader_impl_node_function node_func = (loader_impl_node_function)impl;

	if (node_func != NULL)
	{
		loader_impl_node node_impl = node_func->node_impl;
		function_return ret = NULL;
		napi_status status;

		/* Set up call safe arguments */
		node_impl->func_call_safe->node_impl = node_impl;
		node_impl->func_call_safe->func = func;
		node_impl->func_call_safe->node_func = node_func;
		node_impl->func_call_safe->args = static_cast<void **>(args);
		node_impl->func_call_safe->size = size;
		node_impl->func_call_safe->recv = NULL;
		node_impl->func_call_safe->ret = NULL;

		/* Check if we are in the JavaScript thread */
		if (node_impl->js_thread_id == std::this_thread::get_id())
		{
			/* We are already in the V8 thread, we can call safely */
			node_loader_impl_func_call_safe(node_impl->env, node_impl->func_call_safe);

			/* Set up return of the function call */
			ret = node_impl->func_call_safe->ret;
		}
		/* Lock the mutex and set the parameters */
		else if (node_impl->locked.load() == false && uv_mutex_trylock(&node_impl->mutex) == 0)
		{
			node_impl->locked.store(true);

			/* Acquire the thread safe function in order to do the call */
			status = napi_acquire_threadsafe_function(node_impl->threadsafe_func_call);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to aquire thread safe function invoke function in NodeJS loader");
			}

			/* Execute the thread safe call in a nonblocking manner */
			status = napi_call_threadsafe_function(node_impl->threadsafe_func_call, nullptr, napi_tsfn_nonblocking);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to call to thread safe function invoke function in NodeJS loader");
			}

			/* Release call safe function */
			status = napi_release_threadsafe_function(node_impl->threadsafe_func_call, napi_tsfn_release);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to release thread safe function invoke function in NodeJS loader");
			}

			/* Wait for the execution of the safe call */
			uv_cond_wait(&node_impl->cond, &node_impl->mutex);

			/* Set up return of the function call */
			ret = node_impl->func_call_safe->ret;

			node_impl->locked.store(false);

			/* Unlock the mutex */
			uv_mutex_unlock(&node_impl->mutex);
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Potential deadlock detected in function_node_interface_invoke, the call has not been executed in order to avoid the deadlock");
		}

		return ret;
	}

	return NULL;
}

function_return function_node_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void * context)
{
	loader_impl_node_function node_func = (loader_impl_node_function)impl;

	if (node_func != NULL)
	{
		loader_impl_node node_impl = node_func->node_impl;
		function_return ret = NULL;
		napi_status status;

		/* Set up await safe arguments */
		node_impl->func_await_safe->node_impl = node_impl;
		node_impl->func_await_safe->func = func;
		node_impl->func_await_safe->node_func = node_func;
		node_impl->func_await_safe->args = static_cast<void **>(args);
		node_impl->func_await_safe->size = size;
		node_impl->func_await_safe->resolve_callback = resolve_callback;
		node_impl->func_await_safe->reject_callback = reject_callback;
		node_impl->func_await_safe->context = context;
		node_impl->func_await_safe->recv = NULL;
		node_impl->func_await_safe->ret = NULL;

		/* Check if we are in the JavaScript thread */
		if (node_impl->js_thread_id == std::this_thread::get_id())
		{
			/* We are already in the V8 thread, we can call safely */
			node_loader_impl_func_await_safe(node_impl->env, node_impl->func_await_safe);

			/* Set up return of the function call */
			ret = node_impl->func_await_safe->ret;
		}
		/* Lock the mutex and set the parameters */
		else if (node_impl->locked.load() == false && uv_mutex_trylock(&node_impl->mutex) == 0)
		{
			node_impl->locked.store(true);

			/* Acquire the thread safe function in order to do the call */
			status = napi_acquire_threadsafe_function(node_impl->threadsafe_func_await);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to aquire thread safe function await function in NodeJS loader");
			}

			/* Execute the thread safe call in a nonblocking manner */
			status = napi_call_threadsafe_function(node_impl->threadsafe_func_await, nullptr, napi_tsfn_nonblocking);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to call to thread safe function await function in NodeJS loader");
			}

			/* Release call safe function */
			status = napi_release_threadsafe_function(node_impl->threadsafe_func_await, napi_tsfn_release);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to release thread safe function await function in NodeJS loader");
			}

			/* Wait for the execution of the safe call */
			uv_cond_wait(&node_impl->cond, &node_impl->mutex);

			/* Set up return of the function call */
			ret = node_impl->func_await_safe->ret;

			node_impl->locked.store(false);

			/* Unlock call safe mutex */
			uv_mutex_unlock(&node_impl->mutex);
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Potential deadlock detected in function_node_interface_await, the call has not been executed in order to avoid the deadlock");
		}

		return ret;
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
		napi_status status;

		/* Set up function destroy safe arguments */
		node_impl->func_destroy_safe->node_impl = node_impl;
		node_impl->func_destroy_safe->node_func = node_func;

		/* Check if we are in the JavaScript thread */
		if (node_impl->js_thread_id == std::this_thread::get_id())
		{
			/* We are already in the V8 thread, we can call safely */
			node_loader_impl_func_destroy_safe(node_impl->env, node_impl->func_destroy_safe);
		}
		/* Lock the mutex and set the parameters */
		else if (node_impl->locked.load() == false && uv_mutex_trylock(&node_impl->mutex) == 0)
		{
			node_impl->locked.store(true);

			/* Acquire the thread safe function in order to do the call */
			status = napi_acquire_threadsafe_function(node_impl->threadsafe_func_destroy);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to aquire thread safe function destroy function in NodeJS loader");
			}

			/* Execute the thread safe call in a nonblocking manner */
			status = napi_call_threadsafe_function(node_impl->threadsafe_func_destroy, nullptr, napi_tsfn_nonblocking);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to call to thread safe function destroy function in NodeJS loader");
			}

			/* Release call safe function */
			status = napi_release_threadsafe_function(node_impl->threadsafe_func_destroy, napi_tsfn_release);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to release thread safe function destroy function in NodeJS loader");
			}

			/* Wait for the execution of the safe call */
			uv_cond_wait(&node_impl->cond, &node_impl->mutex);

			node_impl->locked.store(false);

			/* Unlock call safe mutex */
			uv_mutex_unlock(&node_impl->mutex);
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Potential deadlock detected in function_node_interface_destroy, the call has not been executed in order to avoid the deadlock");
		}

		/* Free node function arguments */
		free(node_func->argv);

		/* Free node function */
		free(node_func);
	}
}

function_interface function_node_singleton()
{
	static struct function_interface_type node_function_interface =
	{
		&function_node_interface_create,
		&function_node_interface_invoke,
		&function_node_interface_await,
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

void future_node_interface_destroy(future f, future_impl impl)
{
	loader_impl_node_future node_future = (loader_impl_node_future)impl;

	if (node_future != NULL)
	{
		loader_impl_node node_impl = node_future->node_impl;
		napi_status status;

		/* Set up future delete safe arguments */
		node_impl->future_delete_safe->node_impl = node_impl;
		node_impl->future_delete_safe->node_future = node_future;
		node_impl->future_delete_safe->f = f;

		/* Check if we are in the JavaScript thread */
		if (node_impl->js_thread_id == std::this_thread::get_id())
		{
			/* We are already in the V8 thread, we can call safely */
			node_loader_impl_future_delete_safe(node_impl->env, node_impl->future_delete_safe);
		}
		/* Lock the mutex and set the parameters */
		else if (node_impl->locked.load() == false && uv_mutex_trylock(&node_impl->mutex) == 0)
		{
			node_impl->locked.store(true);

			/* Acquire the thread safe function in order to do the call */
			status = napi_acquire_threadsafe_function(node_impl->threadsafe_future_delete);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to aquire thread safe future destroy function in NodeJS loader");
			}

			/* Execute the thread safe call in a nonblocking manner */
			status = napi_call_threadsafe_function(node_impl->threadsafe_future_delete, nullptr, napi_tsfn_nonblocking);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to call to thread safe future destroy function in NodeJS loader");
			}

			/* Release call safe function */
			status = napi_release_threadsafe_function(node_impl->threadsafe_future_delete, napi_tsfn_release);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to release thread safe future destroy function in NodeJS loader");
			}

			/* Wait for the execution of the safe call */
			uv_cond_wait(&node_impl->cond, &node_impl->mutex);

			node_impl->locked.store(false);

			/* Unlock call safe mutex */
			uv_mutex_unlock(&node_impl->mutex);
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Potential deadlock detected in future_node_interface_destroy, the call has not been executed in order to avoid the deadlock");
		}

		/* Free node future */
		free(node_future);
	}
}

future_interface future_node_singleton()
{
	static struct future_interface_type node_future_interface =
	{
		&future_node_interface_create,
		&future_node_interface_destroy
	};

	return &node_future_interface;
}

void node_loader_impl_initialize_safe(napi_env env, loader_impl_async_initialize_safe initialize_safe)
{
	static const char initialize_str[] = "initialize";
	napi_value function_table_object;
	napi_value initialize_str_value;
	bool result = false;
	napi_handle_scope handle_scope;
	loader_impl_node node_impl = initialize_safe->node_impl;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Get function table object from reference */
	status = napi_get_reference_value(env, node_impl->function_table_object_ref, &function_table_object);

	node_loader_impl_exception(env, status);

	/* Create function string */
	status = napi_create_string_utf8(env, initialize_str, sizeof(initialize_str) - 1, &initialize_str_value);

	node_loader_impl_exception(env, status);

	/* Check if exists in the table */
	status = napi_has_own_property(env, function_table_object, initialize_str_value, &result);

	node_loader_impl_exception(env, status);

	if (result == true)
	{
		napi_value function_trampoline_initialize;
		napi_valuetype valuetype;

		status = napi_get_named_property(env, function_table_object, initialize_str, &function_trampoline_initialize);

		node_loader_impl_exception(env, status);

		status = napi_typeof(env, function_trampoline_initialize, &valuetype);

		node_loader_impl_exception(env, status);

		if (valuetype != napi_function)
		{
			napi_throw_type_error(env, nullptr, "Invalid function initialize in function table object");
		}

		/* Call to load from file function */
		napi_value global, return_value;

		status = napi_get_reference_value(env, node_impl->global_ref, &global);

		node_loader_impl_exception(env, status);

		status = napi_call_function(env, global, function_trampoline_initialize, 0, nullptr, &return_value);

		node_loader_impl_exception(env, status);
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

napi_value node_loader_impl_async_initialize_safe(napi_env env, napi_callback_info info)
{
	loader_impl_async_initialize_safe initialize_safe = NULL;

	napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, nullptr, (void**)&initialize_safe);

	node_loader_impl_exception(env, status);

	/* Lock node implementation mutex */
	uv_mutex_lock(&initialize_safe->node_impl->mutex);

	/* Store environment for reentrant calls */
	initialize_safe->node_impl->env = env;

	/* Call to the implementation function */
	node_loader_impl_initialize_safe(env, initialize_safe);

	/* Clear environment */
	// initialize_safe->node_impl->env = NULL;

	/* Signal start condition */
	uv_cond_signal(&initialize_safe->node_impl->cond);

	uv_mutex_unlock(&initialize_safe->node_impl->mutex);

	return nullptr;
}

void node_loader_impl_func_call_safe(napi_env env, loader_impl_async_func_call_safe func_call_safe)
{
	napi_handle_scope handle_scope;
	size_t args_size;
	value * args;
	napi_value * argv;
	loader_impl_node_function node_func;
	size_t args_count;
	signature s = function_signature(func_call_safe->func);
	const size_t signature_args_size = signature_count(s);

	/* Get function data */
	args_size = func_call_safe->size;
	node_func = func_call_safe->node_func;
	args = func_call_safe->args;

	/* Allocate dynamically more space for values in case of variable arguments */
	argv = args_size > signature_args_size ? static_cast<napi_value *>(malloc(sizeof(napi_value) * args_size)) : node_func->argv;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Build parameters */
	for (args_count = 0; args_count < args_size; ++args_count)
	{
		/* Define parameter */
		argv[args_count] = node_loader_impl_value_to_napi(func_call_safe->node_impl, env, args[args_count]);
	}

	/* Get function reference */
	napi_value function_ptr;

	status = napi_get_reference_value(env, node_func->func_ref, &function_ptr);

	node_loader_impl_exception(env, status);

	/* Get global */
	napi_value global;

	status = napi_get_reference_value(env, func_call_safe->node_impl->global_ref, &global);

	node_loader_impl_exception(env, status);

	/* Call to function */
	napi_value func_return;

	status = napi_call_function(env, global, function_ptr, args_size, argv, &func_return);

	node_loader_impl_exception(env, status);

	/* Convert function return to value */
	func_call_safe->ret = node_loader_impl_napi_to_value(func_call_safe->node_impl, env, func_call_safe->recv, func_return);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);

	if (args_size > signature_args_size)
	{
		free(argv);
	}
}

napi_value node_loader_impl_async_func_call_safe(napi_env env, napi_callback_info info)
{
	loader_impl_async_func_call_safe func_call_safe = NULL;
	napi_status status;
	napi_value recv;

	status = napi_get_cb_info(env, info, nullptr, nullptr, &recv, (void**)&func_call_safe);

	node_loader_impl_exception(env, status);

	/* Lock the call safe mutex and get the parameters */
	uv_mutex_lock(&func_call_safe->node_impl->mutex);

	/* Store function recv for reentrant calls */
	func_call_safe->recv = recv;

	/* Store environment for reentrant calls */
	func_call_safe->node_impl->env = env;

	/* Call to the implementation function */
	node_loader_impl_func_call_safe(env, func_call_safe);

	/* Clear environment */
	// func_call_safe->node_impl->env = NULL;

	/* Signal function call condition */
	uv_cond_signal(&func_call_safe->node_impl->cond);

	uv_mutex_unlock(&func_call_safe->node_impl->mutex);

	return nullptr;
}

void node_loader_impl_async_func_await_finalize(napi_env, void * finalize_data, void *)
{
	loader_impl_async_func_await_trampoline trampoline = static_cast<loader_impl_async_func_await_trampoline>(finalize_data);

	free(trampoline);
}

napi_value node_loader_impl_async_func_resolve(loader_impl_node node_impl, napi_env env, function_resolve_callback resolve, napi_value recv, napi_value v, void * context)
{
	napi_value result;
	value arg, ret;

	if (node_impl == NULL || resolve == NULL)
	{
		return nullptr;
	}

	/* Convert the argument to a value */
	arg = node_loader_impl_napi_to_value(node_impl, env, recv, v);

	if (arg == NULL)
	{
		arg = value_create_null();
	}

	/* Call the resolve callback */
	ret = resolve(arg, context);

	/* Destroy parameter argument */
	value_type_destroy(arg);

	/* Return the result */
	if (ret != NULL)
	{
		result = node_loader_impl_value_to_napi(node_impl, env, ret);
	}
	else
	{
		napi_status status = napi_get_undefined(env, &result);

		node_loader_impl_exception(env, status);
	}

	/* Destroy return value */
	value_type_destroy(ret);

	return result;
}

napi_value node_loader_impl_async_func_reject(loader_impl_node node_impl, napi_env env, function_reject_callback reject, napi_value recv, napi_value v, void * context)
{
	napi_value result;
	value arg, ret;

	if (node_impl == NULL || reject == NULL)
	{
		return nullptr;
	}

	/* Convert the argument to a value */
	arg = node_loader_impl_napi_to_value(node_impl, env, recv, v);

	if (arg == NULL)
	{
		arg = value_create_null();
	}

	/* Call the reject callback */
	ret = reject(arg, context);

	/* Destroy parameter argument */
	value_type_destroy(arg);

	/* Return the result */
	if (ret != NULL)
	{
		result = node_loader_impl_value_to_napi(node_impl, env, ret);
	}
	else
	{
		napi_status status = napi_get_undefined(env, &result);

		node_loader_impl_exception(env, status);
	}

	/* Destroy return value */
	value_type_destroy(ret);

	return result;
}

void node_loader_impl_func_await_safe(napi_env env, loader_impl_async_func_await_safe func_await_safe)
{
	static const char await_str[] = "await";
	napi_value await_str_value;
	napi_value function_table_object;
	napi_value function_await;
	bool result = false;
	napi_value argv[3];
	napi_handle_scope handle_scope;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Get function table object from reference */
	status = napi_get_reference_value(env, func_await_safe->node_impl->function_table_object_ref, &function_table_object);

	node_loader_impl_exception(env, status);

	/* Retrieve resolve function from object table */
	status = napi_create_string_utf8(env, await_str, sizeof(await_str) - 1, &await_str_value);

	node_loader_impl_exception(env, status);

	status = napi_has_own_property(env, function_table_object, await_str_value, &result);

	node_loader_impl_exception(env, status);

	if (result == true)
	{
		napi_valuetype valuetype;

		status = napi_get_named_property(env, function_table_object, await_str, &function_await);

		node_loader_impl_exception(env, status);

		status = napi_typeof(env, function_await, &valuetype);

		node_loader_impl_exception(env, status);

		if (valuetype != napi_function)
		{
			napi_throw_type_error(env, nullptr, "Invalid function test in function table object");
		}
		else
		{
			/* Allocate trampoline object */
			loader_impl_async_func_await_trampoline trampoline = static_cast<loader_impl_async_func_await_trampoline>(malloc(sizeof(struct loader_impl_async_func_await_trampoline_type)));

			if (trampoline != NULL)
			{
				napi_ref trampoline_ref;
				size_t args_size;
				value * args;
				loader_impl_node_function node_func;
				size_t args_count;
				napi_value * func_argv;
				signature s = function_signature(func_await_safe->func);
				const size_t signature_args_size = signature_count(s);

				/* Get function reference */
				status = napi_get_reference_value(env, func_await_safe->node_func->func_ref, &argv[0]);

				node_loader_impl_exception(env, status);

				/* Create array for arguments */
				status = napi_create_array(env, &argv[1]);

				node_loader_impl_exception(env, status);

				/* Get push property from array */
				napi_value push_func;

				status = napi_get_named_property(env, argv[1], "push", &push_func);

				node_loader_impl_exception(env, status);

				/* Get function data */
				args_size = func_await_safe->size;
				args = static_cast<value *>(func_await_safe->args);
				node_func = func_await_safe->node_func;

				/* Allocate dynamically more space for values in case of variable arguments */
				func_argv = args_size > signature_args_size ? static_cast<napi_value *>(malloc(sizeof(napi_value) * args_size)) : node_func->argv;

				/* Build parameters */
				for (args_count = 0; args_count < args_size; ++args_count)
				{
					/* Define parameter */
					func_argv[args_count] = node_loader_impl_value_to_napi(func_await_safe->node_impl, env, args[args_count]);

					/* Push parameter to the array */
					status = napi_call_function(env, argv[1], push_func, 1, &func_argv[args_count], NULL);

					node_loader_impl_exception(env, status);
				}

				/* Set trampoline object values */
				trampoline->node_loader = func_await_safe->node_impl;
				trampoline->resolve_trampoline = &node_loader_impl_async_func_resolve;
				trampoline->reject_trampoline = &node_loader_impl_async_func_reject;
				trampoline->resolve_callback = func_await_safe->resolve_callback;
				trampoline->reject_callback = func_await_safe->reject_callback;
				trampoline->context = func_await_safe->context;

				/* Set the C trampoline object as JS wrapped object */
				status = napi_create_object(env, &argv[2]);

				node_loader_impl_exception(env, status);

				status = napi_wrap(env, argv[2], static_cast<void *>(trampoline), &node_loader_impl_async_func_await_finalize, NULL, &trampoline_ref);

				node_loader_impl_exception(env, status);

				/* Call to function */
				napi_value global, await_return;

				status = napi_get_reference_value(env, func_await_safe->node_impl->global_ref, &global);

				node_loader_impl_exception(env, status);

				status = napi_call_function(env, global, function_await, 3, argv, &await_return);

				node_loader_impl_exception(env, status);

				/* Delete references references to wrapped objects */
				status = napi_delete_reference(env, trampoline_ref);

				node_loader_impl_exception(env, status);

				/* Proccess the await return */
				func_await_safe->ret = node_loader_impl_napi_to_value(func_await_safe->node_impl, env, func_await_safe->recv, await_return);

				if (args_size > signature_args_size)
				{
					free(func_argv);
				}
			}
		}
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

napi_value node_loader_impl_async_func_await_safe(napi_env env, napi_callback_info info)
{
	napi_value recv;
	loader_impl_async_func_await_safe func_await_safe = NULL;

	napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &recv, (void**)&func_await_safe);

	node_loader_impl_exception(env, status);

	/* Lock node implementation mutex */
	uv_mutex_lock(&func_await_safe->node_impl->mutex);

	/* Store function recv for reentrant calls */
	func_await_safe->recv = recv;

	/* Store environment for reentrant calls */
	func_await_safe->node_impl->env = env;

	/* Call to the implementation function */
	node_loader_impl_func_await_safe(env, func_await_safe);

	/* Clear environment */
	// func_await_safe->node_impl->env = NULL;

	/* Signal function await condition */
	uv_cond_signal(&func_await_safe->node_impl->cond);

	uv_mutex_unlock(&func_await_safe->node_impl->mutex);

	return nullptr;
}

void node_loader_impl_func_destroy_safe(napi_env env, loader_impl_async_func_destroy_safe func_destroy_safe)
{
	uint32_t ref_count = 0;
	napi_handle_scope handle_scope;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Clear function persistent reference */
	status = napi_reference_unref(env, func_destroy_safe->node_func->func_ref, &ref_count);

	node_loader_impl_exception(env, status);

	if (ref_count != 0)
	{
		/* TODO: Error handling */
	}

	status = napi_delete_reference(env, func_destroy_safe->node_func->func_ref);

	node_loader_impl_exception(env, status);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

napi_value node_loader_impl_async_func_destroy_safe(napi_env env, napi_callback_info info)
{
	loader_impl_async_func_destroy_safe func_destroy_safe = NULL;

	napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, nullptr, (void**)&func_destroy_safe);

	node_loader_impl_exception(env, status);

	/* Lock node implementation mutex */
	uv_mutex_lock(&func_destroy_safe->node_impl->mutex);

	/* Store environment for reentrant calls */
	func_destroy_safe->node_impl->env = env;

	/* Call to the implementation function */
	node_loader_impl_func_destroy_safe(env, func_destroy_safe);

	/* Clear environment */
	// func_destroy_safe->node_impl->env = NULL;

	/* Signal function destroy condition */
	uv_cond_signal(&func_destroy_safe->node_impl->cond);

	uv_mutex_unlock(&func_destroy_safe->node_impl->mutex);

	return nullptr;
}

void node_loader_impl_future_delete_safe(napi_env env, loader_impl_async_future_delete_safe future_delete_safe)
{
	uint32_t ref_count = 0;
	napi_handle_scope handle_scope;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Clear promise reference */
	status = napi_reference_unref(env, future_delete_safe->node_future->promise_ref, &ref_count);

	node_loader_impl_exception(env, status);

	if (ref_count != 0)
	{
		/* TODO: Error handling */
	}

	status = napi_delete_reference(env, future_delete_safe->node_future->promise_ref);

	node_loader_impl_exception(env, status);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

napi_value node_loader_impl_async_future_delete_safe(napi_env env, napi_callback_info info)
{
	loader_impl_async_future_delete_safe future_delete_safe = NULL;

	napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, nullptr, (void**)&future_delete_safe);

	node_loader_impl_exception(env, status);

	/* Lock node implementation mutex */
	uv_mutex_lock(&future_delete_safe->node_impl->mutex);

	/* Store environment for reentrant calls */
	future_delete_safe->node_impl->env = env;

	/* Call to the implementation function */
	node_loader_impl_future_delete_safe(env, future_delete_safe);

	/* Clear environment */
	// future_delete_safe->node_impl->env = NULL;

	/* Signal future delete condition */
	uv_cond_signal(&future_delete_safe->node_impl->cond);

	uv_mutex_unlock(&future_delete_safe->node_impl->mutex);

	return nullptr;
}

void node_loader_impl_load_from_file_safe(napi_env env, loader_impl_async_load_from_file_safe load_from_file_safe)
{
	static const char load_from_file_str[] = "load_from_file";
	napi_value function_table_object;
	napi_value load_from_file_str_value;
	bool result = false;
	napi_handle_scope handle_scope;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Get function table object from reference */
	status = napi_get_reference_value(env, load_from_file_safe->node_impl->function_table_object_ref, &function_table_object);

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
		status = napi_create_array_with_length(env, load_from_file_safe->size, &argv[0]);

		node_loader_impl_exception(env, status);

		for (size_t index = 0; index < load_from_file_safe->size; ++index)
		{
			napi_value path_str;

			size_t length = strnlen(load_from_file_safe->paths[index], LOADER_NAMING_PATH_SIZE);

			status = napi_create_string_utf8(env, load_from_file_safe->paths[index], length, &path_str);

			node_loader_impl_exception(env, status);

			status = napi_set_element(env, argv[0], (uint32_t)index, path_str);

			node_loader_impl_exception(env, status);
		}

		/* Call to load from file function */
		napi_value global, return_value;

		status = napi_get_reference_value(env, load_from_file_safe->node_impl->global_ref, &global);

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
			status = napi_create_reference(env, return_value, 1, &load_from_file_safe->handle_ref);

			node_loader_impl_exception(env, status);
		}
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

napi_value node_loader_impl_async_load_from_file_safe(napi_env env, napi_callback_info info)
{
	loader_impl_async_load_from_file_safe load_from_file_safe = NULL;

	napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, nullptr, (void**)&load_from_file_safe);

	node_loader_impl_exception(env, status);

	/* Lock node implementation mutex */
	uv_mutex_lock(&load_from_file_safe->node_impl->mutex);

	/* Store environment for reentrant calls */
	load_from_file_safe->node_impl->env = env;

	/* Call to the implementation function */
	node_loader_impl_load_from_file_safe(env, load_from_file_safe);

	/* Clear environment */
	// load_from_file_safe->node_impl->env = NULL;

	/* Signal load from file condition */
	uv_cond_signal(&load_from_file_safe->node_impl->cond);

	uv_mutex_unlock(&load_from_file_safe->node_impl->mutex);

	return nullptr;
}

void node_loader_impl_load_from_memory_safe(napi_env env, loader_impl_async_load_from_memory_safe load_from_memory_safe)
{
	static const char load_from_memory_str[] = "load_from_memory";
	napi_value function_table_object;
	napi_value load_from_memory_str_value;
	bool result = false;
	napi_handle_scope handle_scope;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Get function table object from reference */
	status = napi_get_reference_value(env, load_from_memory_safe->node_impl->function_table_object_ref, &function_table_object);

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
		status = napi_create_string_utf8(env, load_from_memory_safe->name, strlen(load_from_memory_safe->name), &argv[0]);

		node_loader_impl_exception(env, status);

		status = napi_create_string_utf8(env, load_from_memory_safe->buffer, load_from_memory_safe->size - 1, &argv[1]);

		node_loader_impl_exception(env, status);

		status = napi_create_object(env, &argv[2]);

		node_loader_impl_exception(env, status);

		/* Call to load from memory function */
		napi_value global, return_value;

		status = napi_get_reference_value(env, load_from_memory_safe->node_impl->global_ref, &global);

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
			status = napi_create_reference(env, return_value, 1, &load_from_memory_safe->handle_ref);

			node_loader_impl_exception(env, status);
		}
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

napi_value node_loader_impl_async_load_from_memory_safe(napi_env env, napi_callback_info info)
{
	loader_impl_async_load_from_memory_safe load_from_memory_safe = NULL;

	napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, nullptr, (void**)&load_from_memory_safe);

	node_loader_impl_exception(env, status);

	/* Lock node implementation mutex */
	uv_mutex_lock(&load_from_memory_safe->node_impl->mutex);

	/* Store environment for reentrant calls */
	load_from_memory_safe->node_impl->env = env;

	/* Call to the implementation function */
	node_loader_impl_load_from_memory_safe(env, load_from_memory_safe);

	/* Clear environment */
	// load_from_memory_safe->node_impl->env = NULL;

	/* Signal load from memory condition */
	uv_cond_signal(&load_from_memory_safe->node_impl->cond);

	uv_mutex_unlock(&load_from_memory_safe->node_impl->mutex);

	return nullptr;
}

void node_loader_impl_clear_safe(napi_env env, loader_impl_async_clear_safe clear_safe)
{
	static const char clear_str[] = "clear";
	napi_value function_table_object;
	napi_value clear_str_value;
	bool result = false;
	napi_handle_scope handle_scope;
	uint32_t ref_count = 0;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);
	/* Get function table object from reference */
	status = napi_get_reference_value(env, clear_safe->node_impl->function_table_object_ref, &function_table_object);

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
		status = napi_get_reference_value(env, clear_safe->handle_ref, &argv[0]);

		node_loader_impl_exception(env, status);

		/* Call to load from file function */
		napi_value global, clear_return;

		status = napi_get_reference_value(env, clear_safe->node_impl->global_ref, &global);

		node_loader_impl_exception(env, status);

		status = napi_call_function(env, global, function_trampoline_clear, 1, argv, &clear_return);

		node_loader_impl_exception(env, status);
	}

	/* Clear handle persistent reference */
	status = napi_reference_unref(env, clear_safe->handle_ref, &ref_count);

	node_loader_impl_exception(env, status);

	if (ref_count != 0)
	{
		/* TODO: Error handling */
	}

	status = napi_delete_reference(env, clear_safe->handle_ref);

	node_loader_impl_exception(env, status);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

napi_value node_loader_impl_async_clear_safe(napi_env env, napi_callback_info info)
{
	loader_impl_async_clear_safe clear_safe = NULL;

	napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, nullptr, (void**)&clear_safe);

	node_loader_impl_exception(env, status);

	/* Lock node implementation mutex */
	uv_mutex_lock(&clear_safe->node_impl->mutex);

	/* Store environment for reentrant calls */
	clear_safe->node_impl->env = env;

	/* Call to the implementation function */
	node_loader_impl_clear_safe(env, clear_safe);

	/* Clear environment */
	// clear_safe->node_impl->env = NULL;

	/* Signal clear condition */
	uv_cond_signal(&clear_safe->node_impl->cond);

	uv_mutex_unlock(&clear_safe->node_impl->mutex);

	return nullptr;
}

value node_loader_impl_discover_function_safe(napi_env env, loader_impl_async_discover_function_safe discover_function_safe)
{
	static const char discover_function_str[] = "discover_function";
	napi_value discover_function_str_value;
	napi_value function_table_object;
	bool result = false;
	napi_handle_scope handle_scope;
	value function_value = NULL;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Get function table object from reference */
	status = napi_get_reference_value(env, discover_function_safe->node_impl->function_table_object_ref, &function_table_object);

	node_loader_impl_exception(env, status);

	/* Create function string */
	status = napi_create_string_utf8(env, discover_function_str, sizeof(discover_function_str) - 1, &discover_function_str_value);

	node_loader_impl_exception(env, status);

	/* Check if exists in the table */
	status = napi_has_own_property(env, function_table_object, discover_function_str_value, &result);

	node_loader_impl_exception(env, status);

	if (result == true)
	{
		napi_value function_trampoline_discover;
		napi_valuetype valuetype;
		napi_value argv[1];

		status = napi_get_named_property(env, function_table_object, discover_function_str, &function_trampoline_discover);

		node_loader_impl_exception(env, status);

		status = napi_typeof(env, function_trampoline_discover, &valuetype);

		node_loader_impl_exception(env, status);

		if (valuetype != napi_function)
		{
			napi_throw_type_error(env, nullptr, "Invalid function discover in function table object");
		}

		/* Define parameters */
		argv[0] = discover_function_safe->func;

		/* Call to load from file function */
		napi_value global, function_descriptor;

		status = napi_get_reference_value(env, discover_function_safe->node_impl->global_ref, &global);

		node_loader_impl_exception(env, status);

		status = napi_call_function(env, global, function_trampoline_discover, 1, argv, &function_descriptor);

		node_loader_impl_exception(env, status);

		/* Convert return value (discover object) to context */
		napi_value func_name;
		char * func_name_str = NULL;
		bool has_name = false;

		status = napi_has_named_property(env, function_descriptor, "name", &has_name);

		node_loader_impl_exception(env, status);

		/* Retrieve the function name if any */
		if (has_name == true)
		{
			size_t func_name_length = 0;

			status = napi_get_named_property(env, function_descriptor, "name", &func_name);

			node_loader_impl_exception(env, status);

			status = napi_get_value_string_utf8(env, func_name, NULL, 0, &func_name_length);

			node_loader_impl_exception(env, status);

			if (func_name_length > 0)
			{
				func_name_str = static_cast<char *>(malloc(sizeof(char) * (func_name_length + 1)));
			}

			/* Get function name */
			status = napi_get_value_string_utf8(env, func_name, func_name_str, func_name_length + 1, &func_name_length);

			node_loader_impl_exception(env, status);
		}

		/* Retrieve the function properties */
		napi_value function_sig;
		napi_value function_types;
		napi_value function_ret;
		napi_value function_is_async;
		uint32_t function_sig_length;

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

		/* Get function async */
		status = napi_get_named_property(env, function_descriptor, "async", &function_is_async);

		node_loader_impl_exception(env, status);

		/* Check function async type */
		status = napi_typeof(env, function_is_async, &valuetype);

		node_loader_impl_exception(env, status);

		if (valuetype != napi_boolean)
		{
			napi_throw_type_error(env, nullptr, "Invalid NodeJS async flag");
		}

		/* Optionally retrieve types if any in order to support typed supersets of JavaScript like TypeScript */
		static const char types_str[] = "types";
		bool has_types = false;

		status = napi_has_named_property(env, function_descriptor, types_str, &has_types);

		node_loader_impl_exception(env, status);

		if (has_types == true)
		{
			status = napi_get_named_property(env, function_descriptor, types_str, &function_types);

			node_loader_impl_exception(env, status);

			/* Check types array type */
			status = napi_typeof(env, function_types, &valuetype);

			node_loader_impl_exception(env, status);

			if (valuetype != napi_object)
			{
				napi_throw_type_error(env, nullptr, "Invalid NodeJS function types");
			}
		}

		/* Optionally retrieve return value type if any in order to support typed supersets of JavaScript like TypeScript */
		static const char ret_str[] = "ret";
		bool has_ret = false;

		status = napi_has_named_property(env, function_descriptor, ret_str, &has_ret);

		node_loader_impl_exception(env, status);

		if (has_ret == true)
		{
			status = napi_get_named_property(env, function_descriptor, ret_str, &function_ret);

			node_loader_impl_exception(env, status);

			/* Check return value type */
			status = napi_typeof(env, function_ret, &valuetype);

			node_loader_impl_exception(env, status);

			if (valuetype != napi_string)
			{
				napi_throw_type_error(env, nullptr, "Invalid NodeJS return type");
			}
		}

		/* Create node function */
		loader_impl_node_function node_func = static_cast<loader_impl_node_function>(malloc(sizeof(struct loader_impl_node_function_type)));

		/* Create reference to function pointer */
		status = napi_create_reference(env, discover_function_safe->func, 1, &node_func->func_ref);

		node_loader_impl_exception(env, status);

		node_func->node_impl = discover_function_safe->node_impl;

		/* Create function */
		function f = function_create(func_name_str, (size_t)function_sig_length, node_func, &function_node_singleton);

		if (f != NULL)
		{
			signature s = function_signature(f);
			bool is_async = false;

			/* Set function async */
			status = napi_get_value_bool(env, function_is_async, &is_async);

			node_loader_impl_exception(env, status);

			function_async(f, is_async == true ? FUNCTION_ASYNC : FUNCTION_SYNC);

			/* Set return value if any */
			if (has_ret)
			{
				size_t return_type_length;
				char * return_type_str = NULL;

				/* Get return value string length */
				status = napi_get_value_string_utf8(env, function_ret, NULL, 0, &return_type_length);

				node_loader_impl_exception(env, status);

				if (return_type_length > 0)
				{
					return_type_str = static_cast<char *>(malloc(sizeof(char) * (return_type_length + 1)));
				}

				if (return_type_str != NULL)
				{
					/* Get parameter name string */
					status = napi_get_value_string_utf8(env, function_ret, return_type_str, return_type_length + 1, &return_type_length);

					node_loader_impl_exception(env, status);

					/* TODO: Implement return type with impl (may need an important refactor) */
					signature_set_return(s, /*loader_impl_type(discover_function_safe->impl, return_type_str)*/ NULL);

					free(return_type_str);
				}
			}

			/* Set signature */
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

				/* Check if type info is available */
				if (has_types)
				{
					napi_value parameter_type;
					size_t parameter_type_length;
					char * parameter_type_str = NULL;

					/* Get signature parameter type */
					status = napi_get_element(env, function_types, arg_index, &parameter_type);

					node_loader_impl_exception(env, status);

					/* Get parameter type string length */
					status = napi_get_value_string_utf8(env, parameter_type, NULL, 0, &parameter_type_length);

					node_loader_impl_exception(env, status);

					if (parameter_type_length > 0)
					{
						parameter_type_str = static_cast<char *>(malloc(sizeof(char) * (parameter_type_length + 1)));
					}

					/* Get parameter type string */
					status = napi_get_value_string_utf8(env, parameter_type, parameter_type_str, parameter_type_length + 1, &parameter_type_length);

					node_loader_impl_exception(env, status);

					/* TODO: Implement parameter type with impl (may need an important refactor) */
					signature_set(s, (size_t)arg_index, parameter_name_str, /*loader_impl_type(discover_function_safe->impl, parameter_type_str)*/ NULL);

					if (parameter_type_str != NULL)
					{
						free(parameter_type_str);
					}
				}
				else
				{
					signature_set(s, (size_t)arg_index, parameter_name_str, NULL);
				}

				if (parameter_name_str != NULL)
				{
					free(parameter_name_str);
				}
			}

			/* Create value with the function */
			function_value = value_create_function(f);
		}
		else
		{
			free(node_func);
		}

		if (func_name_str != NULL)
		{
			free(func_name_str);
		}
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);

	return function_value;
}

void node_loader_impl_discover_safe(napi_env env, loader_impl_async_discover_safe discover_safe)
{
	static const char discover_str[] = "discover";
	napi_value function_table_object;
	napi_value discover_str_value;
	bool result = false;
	napi_handle_scope handle_scope;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Get function table object from reference */
	status = napi_get_reference_value(env, discover_safe->node_impl->function_table_object_ref, &function_table_object);

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
		status = napi_get_reference_value(env, discover_safe->handle_ref, &argv[0]);

		node_loader_impl_exception(env, status);

		/* Call to load from file function */
		napi_value global, discover_map;

		status = napi_get_reference_value(env, discover_safe->node_impl->global_ref, &global);

		node_loader_impl_exception(env, status);

		status = napi_call_function(env, global, function_trampoline_discover, 1, argv, &discover_map);

		node_loader_impl_exception(env, status);

		/* Convert return value (discover object) to context */
		napi_value func_names;
		uint32_t func_names_length;

		status = napi_get_property_names(env, discover_map, &func_names);

		node_loader_impl_exception(env, status);

		status = napi_get_array_length(env, func_names, &func_names_length);

		node_loader_impl_exception(env, status);

		for (uint32_t index = 0; index < func_names_length; ++index)
		{
			napi_value func_name;
			size_t func_name_length;
			char * func_name_str = NULL;

			status = napi_get_element(env, func_names, index, &func_name);

			node_loader_impl_exception(env, status);

			status = napi_get_value_string_utf8(env, func_name, NULL, 0, &func_name_length);

			node_loader_impl_exception(env, status);

			if (func_name_length > 0)
			{
				func_name_str = static_cast<char *>(malloc(sizeof(char) * (func_name_length + 1)));
			}

			if (func_name_str != NULL)
			{
				napi_value function_descriptor;
				napi_value function_ptr;
				napi_value function_sig;
				napi_value function_types;
				napi_value function_ret;
				napi_value function_is_async;
				uint32_t function_sig_length;

				/* Get function name */
				status = napi_get_value_string_utf8(env, func_name, func_name_str, func_name_length + 1, &func_name_length);

				node_loader_impl_exception(env, status);

				/* Get function descriptor */
				status = napi_get_named_property(env, discover_map, func_name_str, &function_descriptor);

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

				/* Get function async */
				status = napi_get_named_property(env, function_descriptor, "async", &function_is_async);

				node_loader_impl_exception(env, status);

				/* Check function async type */
				status = napi_typeof(env, function_is_async, &valuetype);

				node_loader_impl_exception(env, status);

				if (valuetype != napi_boolean)
				{
					napi_throw_type_error(env, nullptr, "Invalid NodeJS async flag");
				}

				/* Optionally retrieve types if any in order to support typed supersets of JavaScript like TypeScript */
				static const char types_str[] = "types";
				bool has_types = false;

				status = napi_has_named_property(env, function_descriptor, types_str, &has_types);

				node_loader_impl_exception(env, status);

				if (has_types == true)
				{
					status = napi_get_named_property(env, function_descriptor, types_str, &function_types);

					node_loader_impl_exception(env, status);

					/* Check types array type */
					status = napi_typeof(env, function_types, &valuetype);

					node_loader_impl_exception(env, status);

					if (valuetype != napi_object)
					{
						napi_throw_type_error(env, nullptr, "Invalid NodeJS function types");
					}
				}

				/* Optionally retrieve return value type if any in order to support typed supersets of JavaScript like TypeScript */
				static const char ret_str[] = "ret";
				bool has_ret = false;

				status = napi_has_named_property(env, function_descriptor, ret_str, &has_ret);

				node_loader_impl_exception(env, status);

				if (has_ret == true)
				{
					status = napi_get_named_property(env, function_descriptor, ret_str, &function_ret);

					node_loader_impl_exception(env, status);

					/* Check return value type */
					status = napi_typeof(env, function_ret, &valuetype);

					node_loader_impl_exception(env, status);

					if (valuetype != napi_string)
					{
						napi_throw_type_error(env, nullptr, "Invalid NodeJS return type");
					}
				}

				/* Create node function */
				loader_impl_node_function node_func = static_cast<loader_impl_node_function>(malloc(sizeof(struct loader_impl_node_function_type)));

				/* Create reference to function pointer */
				status = napi_create_reference(env, function_ptr, 1, &node_func->func_ref);

				node_loader_impl_exception(env, status);

				node_func->node_impl = discover_safe->node_impl;

				/* Create function */
				function f = function_create(func_name_str, (size_t)function_sig_length, node_func, &function_node_singleton);

				if (f != NULL)
				{
					signature s = function_signature(f);
					scope sp = context_scope(discover_safe->ctx);
					bool is_async = false;

					/* Set function async */
					status = napi_get_value_bool(env, function_is_async, &is_async);

					node_loader_impl_exception(env, status);

					function_async(f, is_async == true ? FUNCTION_ASYNC : FUNCTION_SYNC);

					/* Set return value if any */
					if (has_ret)
					{
						size_t return_type_length;
						char * return_type_str = NULL;

						/* Get return value string length */
						status = napi_get_value_string_utf8(env, function_ret, NULL, 0, &return_type_length);

						node_loader_impl_exception(env, status);

						if (return_type_length > 0)
						{
							return_type_str = static_cast<char *>(malloc(sizeof(char) * (return_type_length + 1)));
						}

						if (return_type_str != NULL)
						{
							/* Get parameter name string */
							status = napi_get_value_string_utf8(env, function_ret, return_type_str, return_type_length + 1, &return_type_length);

							node_loader_impl_exception(env, status);

							signature_set_return(s, loader_impl_type(discover_safe->impl, return_type_str));

							free(return_type_str);
						}
					}

					/* Set signature */
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

						/* Check if type info is available */
						if (has_types)
						{
							napi_value parameter_type;
							size_t parameter_type_length;
							char * parameter_type_str = NULL;

							/* Get signature parameter type */
							status = napi_get_element(env, function_types, arg_index, &parameter_type);

							node_loader_impl_exception(env, status);

							/* Get parameter type string length */
							status = napi_get_value_string_utf8(env, parameter_type, NULL, 0, &parameter_type_length);

							node_loader_impl_exception(env, status);

							if (parameter_type_length > 0)
							{
								parameter_type_str = static_cast<char *>(malloc(sizeof(char) * (parameter_type_length + 1)));
							}

							/* Get parameter type string */
							status = napi_get_value_string_utf8(env, parameter_type, parameter_type_str, parameter_type_length + 1, &parameter_type_length);

							node_loader_impl_exception(env, status);

							signature_set(s, (size_t)arg_index, parameter_name_str, loader_impl_type(discover_safe->impl, parameter_type_str));

							if (parameter_type_str != NULL)
							{
								free(parameter_type_str);
							}
						}
						else
						{
							signature_set(s, (size_t)arg_index, parameter_name_str, NULL);
						}

						if (parameter_name_str != NULL)
						{
							free(parameter_name_str);
						}
					}

					scope_define(sp, function_name(f), value_create_function(f));
				}
				else
				{
					free(node_func);
				}

				free(func_name_str);
			}
		}
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

napi_value node_loader_impl_async_discover_safe(napi_env env, napi_callback_info info)
{
	loader_impl_async_discover_safe discover_safe = NULL;

	napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, nullptr, (void**)&discover_safe);

	node_loader_impl_exception(env, status);

	/* Lock node implementation mutex */
	uv_mutex_lock(&discover_safe->node_impl->mutex);

	/* Store environment for reentrant calls */
	discover_safe->node_impl->env = env;

	/* Call to the implementation function */
	node_loader_impl_discover_safe(env, discover_safe);

	/* Clear environment */
	// discover_safe->node_impl->env = NULL;

	/* Signal discover condition */
	uv_cond_signal(&discover_safe->node_impl->cond);

	uv_mutex_unlock(&discover_safe->node_impl->mutex);

	return nullptr;
}

template <typename T>
void node_loader_impl_thread_safe_function_initialize(napi_env env,
	const char name[], size_t size, napi_value(*callback)(napi_env, napi_callback_info), T ** data,
	napi_value * ptr, napi_threadsafe_function * threadsafe_function)
{
	napi_status status;

	/* Create func call function */
	*data = new T();

	/* Initialize call safe function with context */
	status = napi_create_function(env, NULL, 0, callback, static_cast<void*>(*data), ptr);

	node_loader_impl_exception(env, status);

	/* Create call safe function */
	napi_value threadsafe_func_name;

	status = napi_create_string_utf8(env, name, size, &threadsafe_func_name);

	node_loader_impl_exception(env, status);

	// TODO: Does this number must be equivalent to the number of the threads of NodeJS?
	unsigned int processor_count = std::thread::hardware_concurrency();

	status = napi_create_threadsafe_function(env, *ptr,
		nullptr, threadsafe_func_name,
		0, processor_count,
		nullptr, nullptr,
		nullptr, nullptr,
		threadsafe_function);

	node_loader_impl_exception(env, status);
}

template <typename T>
void node_loader_impl_thread_safe_function_destroy(napi_env env,
	T ** data, napi_threadsafe_function * threadsafe_function)
{
	/* Release aborting the thread safe function */
	napi_status status = napi_release_threadsafe_function(*threadsafe_function, napi_tsfn_abort);

	node_loader_impl_exception(env, status);

	/* Clear safe arguments */
	delete *data;
}

void * node_loader_impl_register(void * node_impl_ptr, void * env_ptr, void * function_table_object_ptr)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(node_impl_ptr);
	napi_env env;
	napi_value function_table_object;
	napi_value global;
	napi_status status;

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl->mutex);

	/* Retrieve the js thread id */
	node_impl->js_thread_id = std::this_thread::get_id();

	env = static_cast<napi_env>(env_ptr);
	function_table_object = static_cast<napi_value>(function_table_object_ptr);

	/* Make global object persistent */
	status = napi_get_global(env, &global);

	node_loader_impl_exception(env, status);

	status = napi_create_reference(env, global, 1, &node_impl->global_ref);

	node_loader_impl_exception(env, status);

	/* Make function table object persistent */
	status = napi_create_reference(env, function_table_object, 1, &node_impl->function_table_object_ref);

	node_loader_impl_exception(env, status);

	/* Initialize thread safe functions */
	{
		/* Safe initialize */
		{
			static const char threadsafe_func_name_str[] = "node_loader_impl_async_initialize_safe";

			node_loader_impl_thread_safe_function_initialize<loader_impl_async_initialize_safe_type>(
				env,
				threadsafe_func_name_str, sizeof(threadsafe_func_name_str),
				&node_loader_impl_async_initialize_safe,
				(loader_impl_async_initialize_safe_type **)(&node_impl->initialize_safe),
				&node_impl->initialize_safe_ptr,
				&node_impl->threadsafe_initialize);
		}

		/* Safe load from file */
		{
			static const char threadsafe_func_name_str[] = "node_loader_impl_async_load_from_file_safe";

			node_loader_impl_thread_safe_function_initialize<loader_impl_async_load_from_file_safe_type>(
				env,
				threadsafe_func_name_str, sizeof(threadsafe_func_name_str),
				&node_loader_impl_async_load_from_file_safe,
				(loader_impl_async_load_from_file_safe_type **)(&node_impl->load_from_file_safe),
				&node_impl->load_from_file_safe_ptr,
				&node_impl->threadsafe_load_from_file);
		}

		/* Safe load from memory */
		{
			static const char threadsafe_func_name_str[] = "node_loader_impl_async_load_from_memory_safe";

			node_loader_impl_thread_safe_function_initialize<loader_impl_async_load_from_memory_safe_type>(
				env,
				threadsafe_func_name_str, sizeof(threadsafe_func_name_str),
				&node_loader_impl_async_load_from_memory_safe,
				(loader_impl_async_load_from_memory_safe_type **)(&node_impl->load_from_memory_safe),
				&node_impl->load_from_memory_safe_ptr,
				&node_impl->threadsafe_load_from_memory);
		}

		/* Safe clear */
		{
			static const char threadsafe_func_name_str[] = "node_loader_impl_async_clear_safe";

			node_loader_impl_thread_safe_function_initialize<loader_impl_async_clear_safe_type>(
				env,
				threadsafe_func_name_str, sizeof(threadsafe_func_name_str),
				&node_loader_impl_async_clear_safe,
				(loader_impl_async_clear_safe_type **)(&node_impl->clear_safe),
				&node_impl->clear_safe_ptr,
				&node_impl->threadsafe_clear);
		}

		/* Safe discover */
		{
			static const char threadsafe_func_name_str[] = "node_loader_impl_async_discover_safe";

			node_loader_impl_thread_safe_function_initialize<loader_impl_async_discover_safe_type>(
				env,
				threadsafe_func_name_str, sizeof(threadsafe_func_name_str),
				&node_loader_impl_async_discover_safe,
				(loader_impl_async_discover_safe_type **)(&node_impl->discover_safe),
				&node_impl->discover_safe_ptr,
				&node_impl->threadsafe_discover);
		}

		/* Safe function call */
		{
			static const char threadsafe_func_name_str[] = "node_loader_impl_async_func_call_safe";

			node_loader_impl_thread_safe_function_initialize<loader_impl_async_func_call_safe_type>(
				env,
				threadsafe_func_name_str, sizeof(threadsafe_func_name_str),
				&node_loader_impl_async_func_call_safe,
				(loader_impl_async_func_call_safe_type **)(&node_impl->func_call_safe),
				&node_impl->func_call_safe_ptr,
				&node_impl->threadsafe_func_call);
		}

		/* Safe function await */
		{
			static const char threadsafe_func_name_str[] = "node_loader_impl_async_func_await_safe";

			node_loader_impl_thread_safe_function_initialize<loader_impl_async_func_await_safe_type>(
				env,
				threadsafe_func_name_str, sizeof(threadsafe_func_name_str),
				&node_loader_impl_async_func_await_safe,
				(loader_impl_async_func_await_safe_type **)(&node_impl->func_await_safe),
				&node_impl->func_await_safe_ptr,
				&node_impl->threadsafe_func_await);
		}

		/* Safe function destroy */
		{
			static const char threadsafe_func_name_str[] = "node_loader_impl_async_func_destroy_safe";

			node_loader_impl_thread_safe_function_initialize<loader_impl_async_func_destroy_safe_type>(
				env,
				threadsafe_func_name_str, sizeof(threadsafe_func_name_str),
				&node_loader_impl_async_func_destroy_safe,
				(loader_impl_async_func_destroy_safe_type **)(&node_impl->func_destroy_safe),
				&node_impl->func_destroy_safe_ptr,
				&node_impl->threadsafe_func_destroy);
		}

		/* Safe future delete */
		{
			static const char threadsafe_func_name_str[] = "node_loader_impl_async_future_delete_safe";

			node_loader_impl_thread_safe_function_initialize<loader_impl_async_future_delete_safe_type>(
				env,
				threadsafe_func_name_str, sizeof(threadsafe_func_name_str),
				&node_loader_impl_async_future_delete_safe,
				(loader_impl_async_future_delete_safe_type **)(&node_impl->future_delete_safe),
				&node_impl->future_delete_safe_ptr,
				&node_impl->threadsafe_future_delete);
		}

		/* Safe destroy */
		{
			static const char threadsafe_func_name_str[] = "node_loader_impl_async_destroy_safe";

			node_loader_impl_thread_safe_function_initialize<loader_impl_async_destroy_safe_type>(
				env,
				threadsafe_func_name_str, sizeof(threadsafe_func_name_str),
				&node_loader_impl_async_destroy_safe,
				(loader_impl_async_destroy_safe_type **)(&node_impl->destroy_safe),
				&node_impl->destroy_safe_ptr,
				&node_impl->threadsafe_destroy);
		}
	}

	/* Run test function, this one can be called without thread safe mechanism */
	/* because it is run already in the correct V8 thread */
	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	{
		static const char test_str[] = "test";
		napi_value test_str_value;

		bool result = false;

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
	loader_impl_thread thread_data = static_cast<loader_impl_thread>(data);
	loader_impl_node node_impl = thread_data->node_impl;
	configuration config = thread_data->config;

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl->mutex);

	/* TODO: Reimplement from here to ... */

	/* TODO: Make this trick more portable... */
	#if defined(WIN32) || defined(_WIN32)
		const size_t path_max_length = MAX_PATH;
	#else
		const size_t path_max_length = PATH_MAX;
	#endif

	char exe_path_str[path_max_length] = { 0 };
	size_t exe_path_str_size = 0, exe_path_str_offset = 0;

	#if defined(WIN32) || defined(_WIN32)
		unsigned int length = GetModuleFileName(NULL, exe_path_str, path_max_length);
	#else
		ssize_t length = readlink("/proc/self/exe", exe_path_str, path_max_length);
	#endif

	size_t iterator;

	if (length == -1 || length == path_max_length)
	{
		/* Report error (TODO: Implement it with thread safe logs) */
		node_impl->error_message = "Node loader register invalid working directory path";

		/* TODO: Make logs thread safe */
		/* log_write("metacall", LOG_LEVEL_ERROR, "node loader register invalid working directory path (%s)", exe_path_str); */

		/* Signal start condition */
		uv_cond_signal(&node_impl->cond);

		/* Unlock node implementation mutex */
		uv_mutex_unlock(&node_impl->mutex);

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
	static const char bootstrap_file_str[] = "bootstrap.js";
	char bootstrap_path_str[path_max_length] = { 0 };
	size_t bootstrap_path_str_size = 0;
	const char * load_library_path_env = getenv("LOADER_LIBRARY_PATH");
	size_t load_library_path_length = 0;

	if (load_library_path_env == NULL)
	{
		/* Report error (TODO: Implement it with thread safe logs) */
		node_impl->error_message = "LOADER_LIBRARY_PATH not defined, bootstrap.js cannot be found";

		/* Signal start condition */
		uv_cond_signal(&node_impl->cond);

		/* Unlock node implementation mutex */
		uv_mutex_unlock(&node_impl->mutex);

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

	/* Detect if another bootstrap script has been defined in the configuration */
	value bootstrap_value = configuration_value(config, "bootstrap_script");

	if (bootstrap_value != NULL)
	{
		/* Load bootstrap script defined in the configuration */
		const char * bootstrap_script = value_to_string(bootstrap_value);
		size_t bootstrap_script_length = strlen(bootstrap_script);

		strncpy(&bootstrap_path_str[load_library_path_length], bootstrap_script, bootstrap_script_length);

		bootstrap_path_str_size = load_library_path_length + bootstrap_script_length + 1;

		bootstrap_path_str[bootstrap_path_str_size - 1] = '\0';
	}
	else
	{
		/* Load default script name */
		strncpy(&bootstrap_path_str[load_library_path_length], bootstrap_file_str, sizeof(bootstrap_file_str) - 1);

		bootstrap_path_str_size = load_library_path_length + sizeof(bootstrap_file_str);

		bootstrap_path_str[bootstrap_path_str_size - 1] = '\0';
	}

	/* Get node impl pointer */
	char * node_impl_ptr_str;
	size_t node_impl_ptr_str_size;

	ssize_t node_impl_ptr_length = snprintf(NULL, 0, "%p", (void *)node_impl);

	if (node_impl_ptr_length <= 0)
	{
		/* Report error (TODO: Implement it with thread safe logs) */
		node_impl->error_message = "Invalid node impl pointer length in NodeJS thread";

		/* Signal start condition */
		uv_cond_signal(&node_impl->cond);

		/* Unlock node implementation mutex */
		uv_mutex_unlock(&node_impl->mutex);

		return;
	}

	node_impl_ptr_str_size = (size_t)node_impl_ptr_length + 1;

	node_impl_ptr_str = static_cast<char *>(malloc(sizeof(char) * node_impl_ptr_str_size));

	if (node_impl_ptr_str == NULL)
	{
		/* Report error (TODO: Implement it with thread safe logs) */
		node_impl->error_message = "Invalid node impl pointer initialization in NodeJS thread";

		/* Signal start condition */
		uv_cond_signal(&node_impl->cond);

		/* Unlock node implementation mutex */
		uv_mutex_unlock(&node_impl->mutex);

		return;
	}

	snprintf(node_impl_ptr_str, node_impl_ptr_str_size, "%p", (void *)node_impl);

	/* Get register pointer */
	char * register_ptr_str;
	size_t register_ptr_str_size;

	ssize_t register_ptr_length = snprintf(NULL, 0, "%p", (void *)&node_loader_impl_register);

	if (register_ptr_length <= 0)
	{
		/* Report error (TODO: Implement it with thread safe logs) */
		node_impl->error_message = "Invalid register pointer length in NodeJS thread";

		/* Signal start condition */
		uv_cond_signal(&node_impl->cond);

		/* Unlock node implementation mutex */
		uv_mutex_unlock(&node_impl->mutex);

		return;
	}

	register_ptr_str_size = (size_t)register_ptr_length + 1;

	register_ptr_str = static_cast<char *>(malloc(sizeof(char) * register_ptr_str_size));

	if (register_ptr_str == NULL)
	{
		free(node_impl_ptr_str);

		/* Report error (TODO: Implement it with thread safe logs) */
		node_impl->error_message = "Invalid register pointer initialization in NodeJS thread";

		/* Signal start condition */
		uv_cond_signal(&node_impl->cond);

		/* Unlock node implementation mutex */
		uv_mutex_unlock(&node_impl->mutex);

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

		/* Report error (TODO: Implement it with thread safe logs) */
		node_impl->error_message = "Invalid argv initialization in NodeJS thread";

		/* Signal start condition */
		uv_cond_signal(&node_impl->cond);

		/* Unlock node implementation mutex */
		uv_mutex_unlock(&node_impl->mutex);

		return;
	}

	/* Initialize the argv string memory */
	memset(argv_str, 0, sizeof(char) * argv_str_size);

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

	#if defined(__POSIX__)
	{
		/* In node::PlatformInit(), we squash all signal handlers for non-shared lib
		build. In order to run test cases against shared lib build, we also need
		to do the same thing for shared lib build here, but only for SIGPIPE for
		now. If node::PlatformInit() is moved to here, then this section could be
		removed. */
		struct sigaction act;
		memset(&act, 0, sizeof(act));
		act.sa_handler = SIG_IGN;
		sigaction(SIGPIPE, &act, nullptr);
	}
	#endif

	/* TODO: Review this, it generates a memory leak in line with 'auxv->a_type' */
	/*
	#if defined(__linux__)
	{
		char** envp = environ;
		while (*envp++ != nullptr) {}
		Elf_auxv_t* auxv = reinterpret_cast<Elf_auxv_t*>(envp);
		for (; auxv->a_type != AT_NULL; auxv++)
		{
			if (auxv->a_type == AT_SECURE)
			{
				node::linux_at_secure = auxv->a_un.a_val;
				break;
			}
		}
	}
	#endif
	*/

	/* Unlock node implementation mutex */
	uv_mutex_unlock(&node_impl->mutex);

	/* Start NodeJS runtime */
	int result = node::Start(argc, reinterpret_cast<char **>(argv));

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl->mutex);

	node_impl->result = result;
	free(argv_str);

	/* Unlock node implementation mutex */
	uv_mutex_unlock(&node_impl->mutex);
}

#ifdef __ANDROID__
void node_loader_impl_thread_log(void * data)
{
	loader_impl_node node_impl = *(static_cast<loader_impl_node *>(data));
	char buffer[128];
	size_t size = 0;

	/* Implement manual buffering for NodeJS stdio */
	while ((size = read(node_impl->pfd[0], buffer, sizeof(buffer) - 1)) > 0)
	{
		if(size > 0 && buffer[size - 1] == '\n')
		{
			--size;
		}

		buffer[size] = '\0';

		__android_log_print(ANDROID_LOG_DEBUG, "MetaCall", buffer);
	}
}
#endif

loader_impl_data node_loader_impl_initialize(loader_impl impl, configuration config, loader_host host)
{
	loader_impl_node node_impl;

	(void)impl;

	loader_copy(host);

	/* Initialize Node Loader Trampoline */
	{
		static napi_module node_loader_trampoline_module =
		{
			NAPI_MODULE_VERSION,
			0x01, /* NM_F_BUILTIN */
			__FILE__,
			node_loader_trampoline_initialize,
			"node_loader_trampoline_module",
			NULL,
			{ 0 }
		};

		napi_module_register(&node_loader_trampoline_module);
	}

	/* Initialize Node Loader Port */
	{
		static napi_module node_loader_port_module =
		{
			NAPI_MODULE_VERSION,
			0x01, /* NM_F_BUILTIN */
			__FILE__,
			node_loader_port_initialize,
			"node_loader_port_module",
			NULL,
			{ 0 }
		};

		napi_module_register(&node_loader_port_module);
	}

	node_impl = new loader_impl_node_type();

	if (node_impl == nullptr)
	{
		return NULL;
	}

	/* Initialize environment for reentrant calls */
	node_impl->env = NULL;

	/* TODO: On error, delete dup, condition and mutex */

	/* Disable stdio buffering, it interacts poorly with printf()
	calls elsewhere in the program (e.g., any logging from V8.) */
	setvbuf(stdout, nullptr, _IONBF, 0);
	setvbuf(stderr, nullptr, _IONBF, 0);

	/* Duplicate stdin, stdout, stderr */
	node_impl->stdin_copy = dup(STDIN_FILENO);
	node_impl->stdout_copy = dup(STDOUT_FILENO);
	node_impl->stderr_copy = dup(STDERR_FILENO);

	#ifdef __ANDROID__
	{
		pipe(node_impl->pfd);
		dup2(node_impl->pfd[1], 1);
		dup2(node_impl->pfd[1], 2);
	}
	#endif

	/* Initialize syncronization */
	if (uv_cond_init(&node_impl->cond) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid NodeJS Thread condition creation");

		/* TODO: Clear resources */

		delete node_impl;

		return NULL;
	}

	if (uv_mutex_init(&node_impl->mutex) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid NodeJS Thread mutex creation");

		/* TODO: Clear resources */

		delete node_impl;

		return NULL;
	}

	/* Initialize lock info */
	node_impl->locked.store(false);

	/* Initialize execution result */
	node_impl->result = 1;
	node_impl->error_message = NULL;

	/* Create NodeJS logging thread */
	#ifdef __ANDROID__
	{
		if (uv_thread_create(&node_impl->thread_log_id, node_loader_impl_thread_log, &node_impl) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid NodeJS Logging Thread creation");

			/* TODO: Clear resources */

			delete node_impl;

			return NULL;
		}
	}
	#endif

	struct loader_impl_thread_type thread_data =
	{
		node_impl,
		config
	};

	/* Create NodeJS thread */
	if (uv_thread_create(&node_impl->thread, node_loader_impl_thread, &thread_data) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid NodeJS Thread creation");

		/* TODO: Clear resources */

		delete node_impl;

		return NULL;
	}

	/* Wait until start has been launch */
	uv_mutex_lock(&node_impl->mutex);

	uv_cond_wait(&node_impl->cond, &node_impl->mutex);

	if (node_impl->error_message != NULL)
	{
		uv_mutex_unlock(&node_impl->mutex);

		/* TODO: Remove this when implementing thread safe */
		log_write("metacall", LOG_LEVEL_ERROR, node_impl->error_message);

		return NULL;
	}

	uv_mutex_unlock(&node_impl->mutex);

	/* Call initialize function with thread safe */
	{
		napi_status status;
		int result = 1;

		/* Set up initialize safe arguments */
		node_impl->initialize_safe->node_impl = node_impl;
		node_impl->initialize_safe->result = 0;

		/* Check if we are in the JavaScript thread */
		if (node_impl->js_thread_id == std::this_thread::get_id())
		{
			/* We are already in the V8 thread, we can call safely */
			node_loader_impl_initialize_safe(node_impl->env, node_impl->initialize_safe);

			/* Set up return of the function call */
			result = node_impl->initialize_safe->result;
		}
		/* Lock the mutex and set the parameters */
		else if (node_impl->locked.load() == false && uv_mutex_trylock(&node_impl->mutex) == 0)
		{
			node_impl->locked.store(true);

			/* Acquire the thread safe function in order to do the call */
			status = napi_acquire_threadsafe_function(node_impl->threadsafe_initialize);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to aquire thread safe initialize function in NodeJS loader");
			}

			/* Execute the thread safe call in a nonblocking manner */
			status = napi_call_threadsafe_function(node_impl->threadsafe_initialize, nullptr, napi_tsfn_nonblocking);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to call to thread safe initialize function in NodeJS loader");
			}

			/* Release initialize safe function */
			status = napi_release_threadsafe_function(node_impl->threadsafe_initialize, napi_tsfn_release);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to release thread safe initialize function in NodeJS loader");
			}

			/* Wait for the execution of the safe call */
			uv_cond_wait(&node_impl->cond, &node_impl->mutex);

			/* Set up return of the function call */
			result = node_impl->initialize_safe->result;

			node_impl->locked.store(false);

			/* Unlock the mutex */
			uv_mutex_unlock(&node_impl->mutex);
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Potential deadlock detected in node_loader_impl_initialize, the call has not been executed in order to avoid the deadlock");
		}

		if (result != 0)
		{
			/* TODO: Implement better error message */
			log_write("metacall", LOG_LEVEL_ERROR, "Call to initialization function node_loader_impl_async_initialize_safe failed");

			/* TODO: Handle properly the error */
		}
	}

	/* Register initialization */
	loader_initialization_register(impl);

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
	napi_ref handle_ref = NULL;
	napi_status status;

	if (node_impl == NULL || size == 0)
	{
		return NULL;
	}

	/* Set up load from file safe arguments */
	node_impl->load_from_file_safe->node_impl = node_impl;
	node_impl->load_from_file_safe->paths = paths;
	node_impl->load_from_file_safe->size = size;
	node_impl->load_from_file_safe->handle_ref = NULL;

	/* Check if we are in the JavaScript thread */
	if (node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_load_from_file_safe(node_impl->env, node_impl->load_from_file_safe);

		/* Retreive the result handle */
		handle_ref = node_impl->load_from_file_safe->handle_ref;
	}
	/* Lock the mutex and set the parameters */
	else if (node_impl->locked.load() == false && uv_mutex_trylock(&node_impl->mutex) == 0)
	{
		node_impl->locked.store(true);

		/* Acquire the thread safe function in order to do the call */
		status = napi_acquire_threadsafe_function(node_impl->threadsafe_load_from_file);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to aquire thread safe load from file function in NodeJS loader");
		}

		/* Execute the thread safe call in a nonblocking manner */
		status = napi_call_threadsafe_function(node_impl->threadsafe_load_from_file, nullptr, napi_tsfn_nonblocking);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to call to thread safe load from file function in NodeJS loader");
		}

		/* Release call safe function */
		status = napi_release_threadsafe_function(node_impl->threadsafe_load_from_file, napi_tsfn_release);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to release thread safe load from file function in NodeJS loader");
		}

		/* Wait for the execution of the safe call */
		uv_cond_wait(&node_impl->cond, &node_impl->mutex);

		/* Retreive the result handle */
		handle_ref = node_impl->load_from_file_safe->handle_ref;

		node_impl->locked.store(false);

		/* Unlock call safe mutex */
		uv_mutex_unlock(&node_impl->mutex);
	}
	else
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Potential deadlock detected in node_loader_impl_load_from_file, the call has not been executed in order to avoid the deadlock");
	}

	return static_cast<loader_handle>(handle_ref);
}

loader_handle node_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));
	napi_ref handle_ref = NULL;
	napi_status status;

	if (node_impl == NULL || buffer == NULL || size == 0)
	{
		return NULL;
	}

	/* Set up load from memory safe arguments */
	node_impl->load_from_memory_safe->node_impl = node_impl;
	node_impl->load_from_memory_safe->name = name;
	node_impl->load_from_memory_safe->buffer = buffer;
	node_impl->load_from_memory_safe->size = size;
	node_impl->load_from_memory_safe->handle_ref = NULL;

	/* Check if we are in the JavaScript thread */
	if (node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_load_from_memory_safe(node_impl->env, node_impl->load_from_memory_safe);

		/* Retreive the result handle */
		handle_ref = node_impl->load_from_memory_safe->handle_ref;
	}
	/* Lock the mutex and set the parameters */
	else if (node_impl->locked.load() == false && uv_mutex_trylock(&node_impl->mutex) == 0)
	{
		node_impl->locked.store(true);

		/* Acquire the thread safe function in order to do the call */
		status = napi_acquire_threadsafe_function(node_impl->threadsafe_load_from_memory);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to aquire thread safe load from memory function in NodeJS loader");
		}

		/* Execute the thread safe call in a nonblocking manner */
		status = napi_call_threadsafe_function(node_impl->threadsafe_load_from_memory, nullptr, napi_tsfn_nonblocking);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to call to thread safe load from memory function in NodeJS loader");
		}

		/* Release call safe function */
		status = napi_release_threadsafe_function(node_impl->threadsafe_load_from_memory, napi_tsfn_release);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to release thread safe load from memory function in NodeJS loader");
		}

		/* Wait for the execution of the safe call */
		uv_cond_wait(&node_impl->cond, &node_impl->mutex);

		/* Retreive the result handle */
		handle_ref = node_impl->load_from_memory_safe->handle_ref;

		node_impl->locked.store(false);

		/* Unlock call safe mutex */
		uv_mutex_unlock(&node_impl->mutex);
	}
	else
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Potential deadlock detected in node_loader_impl_load_from_memory, the call has not been executed in order to avoid the deadlock");
	}

	return static_cast<loader_handle>(handle_ref);
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
	napi_status status;

	if (node_impl == NULL || handle_ref == NULL)
	{
		return 1;
	}

	/* Set up clear safe arguments */
	node_impl->clear_safe->node_impl = node_impl;
	node_impl->clear_safe->handle_ref = handle_ref;

	/* Check if we are in the JavaScript thread */
	if (node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_clear_safe(node_impl->env, node_impl->clear_safe);
	}
	/* Lock the mutex and set the parameters */
	else if (node_impl->locked.load() == false && uv_mutex_trylock(&node_impl->mutex) == 0)
	{
		node_impl->locked.store(true);

		/* Acquire the thread safe function in order to do the call */
		status = napi_acquire_threadsafe_function(node_impl->threadsafe_clear);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to aquire thread safe clear function in NodeJS loader");
		}

		/* Execute the thread safe call in a nonblocking manner */
		status = napi_call_threadsafe_function(node_impl->threadsafe_clear, nullptr, napi_tsfn_nonblocking);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to call to thread safe clear function in NodeJS loader");
		}

		/* Release call safe function */
		status = napi_release_threadsafe_function(node_impl->threadsafe_clear, napi_tsfn_release);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to release thread safe clear function in NodeJS loader");
		}

		/* Wait for the execution of the safe call */
		uv_cond_wait(&node_impl->cond, &node_impl->mutex);

		node_impl->locked.store(false);

		/* Unlock call safe mutex */
		uv_mutex_unlock(&node_impl->mutex);
	}
	else
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Potential deadlock detected in node_loader_impl_clear, the call has not been executed in order to avoid the deadlock");
	}

	return 0;
}

int node_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));
	napi_ref handle_ref = static_cast<napi_ref>(handle);
	napi_status status;

	if (node_impl == NULL || handle == NULL || ctx == NULL)
	{
		return 1;
	}

	/* Set up discover safe arguments */
	node_impl->discover_safe->impl = impl;
	node_impl->discover_safe->node_impl = node_impl;
	node_impl->discover_safe->handle_ref = handle_ref;
	node_impl->discover_safe->ctx = ctx;

	/* Check if we are in the JavaScript thread */
	if (node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_discover_safe(node_impl->env, node_impl->discover_safe);
	}
	/* Lock the mutex and set the parameters */
	else if (node_impl->locked.load() == false && uv_mutex_trylock(&node_impl->mutex) == 0)
	{
		node_impl->locked.store(true);

		/* Acquire the thread safe function in order to do the call */
		status = napi_acquire_threadsafe_function(node_impl->threadsafe_discover);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to aquire thread safe discover function in NodeJS loader");
		}

		/* Execute the thread safe call in a nonblocking manner */
		status = napi_call_threadsafe_function(node_impl->threadsafe_discover, nullptr, napi_tsfn_nonblocking);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to call to thread safe discover function in NodeJS loader");
		}

		/* Release call safe function */
		status = napi_release_threadsafe_function(node_impl->threadsafe_discover, napi_tsfn_release);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to release thread safe discover function in NodeJS loader");
		}

		/* Wait for the execution of the safe call */
		uv_cond_wait(&node_impl->cond, &node_impl->mutex);

		node_impl->locked.store(false);

		/* Unlock call safe mutex */
		uv_mutex_unlock(&node_impl->mutex);
	}
	else
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Potential deadlock detected in node_loader_impl_discover, the call has not been executed in order to avoid the deadlock");
	}

	return 0;
}

void node_loader_impl_destroy_safe(napi_env env, loader_impl_async_destroy_safe destroy_safe)
{
	uint32_t ref_count = 0;
	napi_status status;
	napi_handle_scope handle_scope;

	loader_impl_node node_impl = destroy_safe->node_impl;

	/* Destroy children loaders (any loader initialized in the current V8 thread should be deleted first) */
	loader_unload_children();

	/* Create scope */
	status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Call destroy function */
	{
		static const char destroy_str[] = "destroy";
		napi_value destroy_str_value;
		napi_value function_table_object;
		bool result = false;

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
	}

	/* Clear thread safe functions */
	{
		/* Safe initialize */
		{
			node_loader_impl_thread_safe_function_destroy<loader_impl_async_initialize_safe_type>(
				env,
				(loader_impl_async_initialize_safe_type **)(&node_impl->initialize_safe),
				&node_impl->threadsafe_initialize);
		}

		/* Safe load from file */
		{
			node_loader_impl_thread_safe_function_destroy<loader_impl_async_load_from_file_safe_type>(
				env,
				(loader_impl_async_load_from_file_safe_type **)(&node_impl->load_from_file_safe),
				&node_impl->threadsafe_load_from_file);
		}

		/* Safe load from memory */
		{
			node_loader_impl_thread_safe_function_destroy<loader_impl_async_load_from_memory_safe_type>(
				env,
				(loader_impl_async_load_from_memory_safe_type **)(&node_impl->load_from_memory_safe),
				&node_impl->threadsafe_load_from_memory);
		}

		/* Safe clear */
		{
			node_loader_impl_thread_safe_function_destroy<loader_impl_async_clear_safe_type>(
				env,
				(loader_impl_async_clear_safe_type **)(&node_impl->clear_safe),
				&node_impl->threadsafe_clear);
		}

		/* Safe discover */
		{
			node_loader_impl_thread_safe_function_destroy<loader_impl_async_discover_safe_type>(
				env,
				(loader_impl_async_discover_safe_type **)(&node_impl->discover_safe),
				&node_impl->threadsafe_discover);
		}

		/* Safe function call */
		{
			node_loader_impl_thread_safe_function_destroy<loader_impl_async_func_call_safe_type>(
				env,
				(loader_impl_async_func_call_safe_type **)(&node_impl->func_call_safe),
				&node_impl->threadsafe_func_call);
		}

		/* Safe function await */
		{
			node_loader_impl_thread_safe_function_destroy<loader_impl_async_func_await_safe_type>(
				env,
				(loader_impl_async_func_await_safe_type **)(&node_impl->func_await_safe),
				&node_impl->threadsafe_func_await);
		}

		/* Safe function destroy */
		{
			node_loader_impl_thread_safe_function_destroy<loader_impl_async_func_destroy_safe_type>(
				env,
				(loader_impl_async_func_destroy_safe_type **)(&node_impl->func_destroy_safe),
				&node_impl->threadsafe_func_destroy);
		}

		/* Safe future delete */
		{
			node_loader_impl_thread_safe_function_destroy<loader_impl_async_future_delete_safe_type>(
				env,
				(loader_impl_async_future_delete_safe_type **)(&node_impl->future_delete_safe),
				&node_impl->threadsafe_future_delete);
		}

		/* Safe destroy */
		{
			node_loader_impl_thread_safe_function_destroy<loader_impl_async_destroy_safe_type>(
				env,
				(loader_impl_async_destroy_safe_type **)(&node_impl->destroy_safe),
				&node_impl->threadsafe_destroy);
		}
	}

	/* Clear persistent references */
	status = napi_reference_unref(env, node_impl->global_ref, &ref_count);

	node_loader_impl_exception(env, status);

	if (ref_count != 0)
	{
		/* TODO: Error handling */
	}

	status = napi_delete_reference(env, node_impl->global_ref);

	node_loader_impl_exception(env, status);

	status = napi_reference_unref(env, node_impl->function_table_object_ref, &ref_count);

	node_loader_impl_exception(env, status);

	if (ref_count != 0)
	{
		/* TODO: Error handling */
	}

	status = napi_delete_reference(env, node_impl->function_table_object_ref);

	node_loader_impl_exception(env, status);

	/* Clear event loop */
	{
		/* Stop event loop */
		uv_stop(node_impl->thread_loop);

		/* Clear event loop */
		uv_walk(node_impl->thread_loop, node_loader_impl_walk, NULL);

		while (uv_run(node_impl->thread_loop, UV_RUN_DEFAULT) != 0);

		/* Destroy node loop */
		if (uv_loop_alive(node_impl->thread_loop) != 0)
		{
			/* TODO: Make logs thread safe */
			/* log_write("metacall", LOG_LEVEL_ERROR, "NodeJS event loop should not be alive"); */
			printf("NodeJS Loader Error: NodeJS event loop should not be alive\n");
			fflush(stdout);
		}

		/* Note: This evaluates to true always due to stdin and stdout handles,
		which are closed anyway on thread join. So it is removed by now. */
		#if 0
		/* TODO: Check how to delete properly all handles */
		if (uv_loop_close(node_impl->thread_loop) == UV_EBUSY)
		{
			/* TODO: Make logs thread safe */
			/* log_write("metacall", LOG_LEVEL_ERROR, "NodeJS event loop should not be busy"); */
			printf("NodeJS Loader Error: NodeJS event loop should not be busy\n");
			fflush(stdout);
		}
		#endif
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

napi_value node_loader_impl_async_destroy_safe(napi_env env, napi_callback_info info)
{
	loader_impl_node node_impl;
	loader_impl_async_destroy_safe destroy_safe = NULL;

	napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, nullptr, (void**)&destroy_safe);

	node_loader_impl_exception(env, status);

	/* Lock the call safe mutex and get the parameters */
	uv_mutex_lock(&destroy_safe->node_impl->mutex);

	/* Store node impl reference because destroy_safe gets deteled after calling node_loader_impl_destroy_safe */
	node_impl = destroy_safe->node_impl;

	/* Store environment for reentrant calls */
	node_impl->env = env;

	/* Call to the implementation function */
	node_loader_impl_destroy_safe(env, destroy_safe);

	/* Clear environment */
	// node_impl->env = NULL;

	/* Signal destroy condition */
	uv_cond_signal(&node_impl->cond);

	uv_mutex_unlock(&node_impl->mutex);

	return nullptr;
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

	/* Call destroy function with thread safe */
	{
		napi_status status;

		/* Set up destroy safe arguments */
		node_impl->destroy_safe->node_impl = node_impl;

		/* Check if we are in the JavaScript thread */
		if (node_impl->js_thread_id == std::this_thread::get_id())
		{
			/* We are already in the V8 thread, we can call safely */
			node_loader_impl_destroy_safe(node_impl->env, node_impl->destroy_safe);
		}
		/* Lock the mutex and set the parameters */
		else if (node_impl->locked.load() == false && uv_mutex_trylock(&node_impl->mutex) == 0)
		{
			node_impl->locked.store(true);

			/* Acquire the thread safe function in order to do the call */
			status = napi_acquire_threadsafe_function(node_impl->threadsafe_destroy);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to aquire thread safe destroy function in NodeJS loader");
			}

			/* Execute the thread safe call in a nonblocking manner */
			status = napi_call_threadsafe_function(node_impl->threadsafe_destroy, nullptr, napi_tsfn_nonblocking);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to call to thread safe destroy function in NodeJS loader");
			}

			/* Release call safe function */
			status = napi_release_threadsafe_function(node_impl->threadsafe_destroy, napi_tsfn_release);

			if (status != napi_ok)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid to release thread safe destroy function in NodeJS loader");
			}

			/* Wait for the execution of the safe call */
			uv_cond_wait(&node_impl->cond, &node_impl->mutex);

			node_impl->locked.store(false);

			/* Unlock call safe mutex */
			uv_mutex_unlock(&node_impl->mutex);
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Potential deadlock detected in node_loader_impl_destroy, the call has not been executed in order to avoid the deadlock");
		}
	}

	/* Wait for node thread to finish */
	uv_thread_join(&node_impl->thread);

	/* Clear condition syncronization object */
	uv_cond_destroy(&node_impl->cond);

	/* Clear mutex syncronization object */
	uv_mutex_destroy(&node_impl->mutex);

	#ifdef __ANDROID__
		/* Close file descriptors */
		close(node_impl->pfd[0]);
		close(node_impl->pfd[1]);

		/* Wait for node log thread to finish */
		uv_thread_join(&node_impl->thread_log_id);
	#endif

	/* Print NodeJS execution result */
	log_write("metacall", LOG_LEVEL_INFO, "NodeJS execution return status %d", node_impl->result);

	/* Restore stdin, stdout, stderr */
	dup2(node_impl->stdin_copy, STDIN_FILENO);
	dup2(node_impl->stdout_copy, STDOUT_FILENO);
	dup2(node_impl->stderr_copy, STDERR_FILENO);

	delete node_impl;

	return 0;
}
