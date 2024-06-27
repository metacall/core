/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading nodejs code at run-time into a process.
 *
 */

#if defined(WIN32) || defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <io.h>
	#ifndef dup
		#define dup _dup
	#endif
	#ifndef dup2
		#define dup2 _dup2
	#endif
	#ifndef STDIN_FILENO
		#define STDIN_FILENO _fileno(stdin)
	#endif
	#ifndef STDOUT_FILENO
		#define STDOUT_FILENO _fileno(stdout)
	#endif
	#ifndef STDERR_FILENO
		#define STDERR_FILENO _fileno(stderr)
	#endif
#else
	#include <unistd.h>
#endif

#if defined(__POSIX__)
	#include <signal.h>
#endif

#ifdef __linux__
	#include <elf.h>
	#ifdef __LP64__
		#define Elf_auxv_t Elf64_auxv_t
	#else
		#define Elf_auxv_t Elf32_auxv_t
	#endif /* __LP64__ */
extern char **environ;
#endif /* __linux__ */

#include <node_loader/node_loader_bootstrap.h>
#include <node_loader/node_loader_impl.h>
#include <node_loader/node_loader_port.h>
#include <node_loader/node_loader_trampoline.h>

#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER >= 1200)
	#include <node_loader/node_loader_win32_delay_load.h>

	/* Required for the DelayLoad hook interposition, solves bug of NodeJS extensions requiring node.exe instead of node.dll*/
	#include <intrin.h>
	#pragma intrinsic(_ReturnAddress)
#endif

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_future.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <portability/portability_executable_path.h>

/* TODO: Make logs thread safe */
#include <log/log.h>

#include <metacall/metacall.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <atomic>
#include <fstream>
#include <new>
#include <streambuf>
#include <string>
#include <thread>

/* Disable warnings from V8 and NodeJS */
#if defined(_MSC_VER)
	#pragma warning(push)
	#pragma warning(disable : 4100)
	#pragma warning(disable : 4275)
	#pragma warning(disable : 4251)
#elif defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wunused-parameter"
	#pragma clang diagnostic ignored "-Wstrict-aliasing"
#elif defined(__GNUC__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

/* NodeJS Includes */
#include <node_api.h>

#ifdef NAPI_VERSION
	#undef NAPI_VERSION
#endif

#include <node.h>

#include <v8.h> /* version: 6.2.414.50 */

#ifdef ENABLE_DEBUGGER_SUPPORT
	#include <v8-debug.h>
#endif /* ENALBLE_DEBUGGER_SUPPORT */

#include <uv.h>

/* Disable warnings from V8 and NodeJS */
#if defined(_MSC_VER)
	#pragma warning(pop)
#elif defined(__clang__)
	#pragma clang diagnostic pop
#elif defined(__GNUC__)
	#pragma GCC diagnostic pop
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
	#error "NodeJS version not supported"
#endif

template <typename T>
union loader_impl_async_safe_cast
{
	T safe;
	void *ptr;
};

template <typename T>
union loader_impl_handle_safe_cast
{
	T *safe;
	uv_handle_t *handle;
};

typedef struct loader_impl_node_function_type
{
	loader_impl_node node_impl;
	loader_impl impl;
	napi_ref func_ref;
	napi_value *argv;

} * loader_impl_node_function;

typedef struct loader_impl_node_future_type
{
	loader_impl_node node_impl;
	napi_ref promise_ref;

} * loader_impl_node_future;

template <typename T>
struct loader_impl_async_safe_type
{
	uv_mutex_t mutex;
	uv_cond_t cond;
	T &args;

	loader_impl_async_safe_type(T &args) :
		args(args)
	{
		uv_mutex_init(&mutex);
		uv_cond_init(&cond);
	}

	~loader_impl_async_safe_type()
	{
		uv_mutex_destroy(&mutex);
		uv_cond_destroy(&cond);
	}

	void lock()
	{
		uv_mutex_lock(&mutex);
	}

	void unlock()
	{
		uv_mutex_unlock(&mutex);
	}

	void wait()
	{
		uv_cond_wait(&cond, &mutex);
	}

	void notify()
	{
		uv_cond_signal(&cond);
	}
};

template <typename T>
struct loader_impl_async_safe_notify_type
{
	loader_impl_async_safe_type<T> *async_safe;

	loader_impl_async_safe_notify_type(loader_impl_async_safe_type<T> *async_safe) :
		async_safe(async_safe)
	{
		async_safe->lock();
	}

	~loader_impl_async_safe_notify_type()
	{
		async_safe->notify();
		async_safe->unlock();
	}
};

template <typename T>
union node_loader_impl_func_call_js_safe_cast
{
	void (*func_ptr)(napi_env, T *);
	void *context;

	node_loader_impl_func_call_js_safe_cast(void *context) :
		context(context) {}
	node_loader_impl_func_call_js_safe_cast(void (*func_ptr)(napi_env, T *)) :
		func_ptr(func_ptr) {}
};

template <typename T>
void node_loader_impl_func_call_js_safe(napi_env env, napi_value js_callback, void *context, void *data)
{
	(void)js_callback;

	if (env == nullptr || js_callback == nullptr || context == nullptr || data == nullptr)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid arguments passed to js thread safe function");
	}

	loader_impl_async_safe_type<T> *async_safe = static_cast<loader_impl_async_safe_type<T> *>(data);
	node_loader_impl_func_call_js_safe_cast<T> safe_cast(context);
	loader_impl_async_safe_notify_type<T> notify(async_safe);

	/* Store environment for reentrant calls */
	async_safe->args.node_impl->env = env;

	/* Call to the implementation function */
	safe_cast.func_ptr(env, &async_safe->args);

	/* Clear environment */
	// async_safe->args->node_impl->env = nullptr;
}

template <typename T>
void node_loader_impl_func_call_js_async_safe(napi_env env, napi_value js_callback, void *context, void *data)
{
	(void)js_callback;

	if (env == nullptr || context == nullptr || data == nullptr)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid arguments passed to js thread async safe function");
		return;
	}

	T *args = static_cast<T *>(data);
	node_loader_impl_func_call_js_safe_cast<T> safe_cast(context);

	/* Store environment for reentrant calls */
	args->node_impl->env = env;

	/* Call to the implementation function */
	safe_cast.func_ptr(env, args);

	/* Clear environment */
	// async_safe->args->node_impl->env = nullptr;
}

static napi_value node_loader_impl_async_threadsafe_empty(napi_env, napi_callback_info)
{
	/* This is a dirty hack in order to make the threadsafe API work properly,
	* as soon as possible it will be good to return to the old method we used in NodeJS 8,
	* it was better than this API
	*/

	return nullptr;
}

template <typename T>
struct loader_impl_threadsafe_type
{
	napi_threadsafe_function threadsafe_function;

	void initialize(napi_env env, std::string name, void (*safe_func_ptr)(napi_env, T *))
	{
		napi_value func_safe_ptr;

		/* Initialize safe function with context */
		napi_status status = napi_create_function(env, nullptr, 0, &node_loader_impl_async_threadsafe_empty, nullptr, &func_safe_ptr);

		node_loader_impl_exception(env, status);

		/* Create safe function */
		napi_value threadsafe_func_name;

		status = napi_create_string_utf8(env, name.c_str(), name.length(), &threadsafe_func_name);

		node_loader_impl_exception(env, status);

		/* Use the amoun of available threads as initial thread count */
		unsigned int processor_count = std::thread::hardware_concurrency();

		/* Cast the safe function */
		node_loader_impl_func_call_js_safe_cast<T> safe_cast(safe_func_ptr);

		status = napi_create_threadsafe_function(env, func_safe_ptr,
			nullptr, threadsafe_func_name,
			0, processor_count,
			nullptr, nullptr,
			safe_cast.context, &node_loader_impl_func_call_js_safe<T>,
			&threadsafe_function);

		node_loader_impl_exception(env, status);
	}

	void invoke(loader_impl_async_safe_type<T> &async_safe)
	{
		/* Lock the mutex */
		async_safe.lock();

		/* Acquire the thread safe function in order to do the call */
		napi_status status = napi_acquire_threadsafe_function(threadsafe_function);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to aquire thread safe function invoke function in NodeJS loader");
		}

		/* Execute the thread safe call in a nonblocking manner */
		status = napi_call_threadsafe_function(threadsafe_function, &async_safe, napi_tsfn_nonblocking);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to call to thread safe function invoke function in NodeJS loader");
		}

		/* Wait for the execution of the safe call */
		async_safe.wait();
	}

	void release(loader_impl_async_safe_type<T> &async_safe)
	{
		/* Unlock the mutex */
		async_safe.unlock();

		/* Release call safe function */
		napi_status status = napi_release_threadsafe_function(threadsafe_function, napi_tsfn_release);

		if (status != napi_ok)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid to release thread safe function invoke function in NodeJS loader");
		}
	}

	void abort(napi_env env)
	{
		napi_status status = napi_release_threadsafe_function(threadsafe_function, napi_tsfn_abort);

		node_loader_impl_exception(env, status);
	}
};

struct loader_impl_async_initialize_safe_type
{
	loader_impl_node node_impl;
	char *loader_library_path;
	int result;

	loader_impl_async_initialize_safe_type(loader_impl_node node_impl, char *loader_library_path) :
		node_impl(node_impl), loader_library_path(loader_library_path), result(0) {}
};

struct loader_impl_async_execution_path_safe_type
{
	loader_impl_node node_impl;
	const char *path;

	loader_impl_async_execution_path_safe_type(loader_impl_node node_impl, const char *path) :
		node_impl(node_impl), path(path) {}
};

struct loader_impl_async_load_from_file_safe_type
{
	loader_impl_node node_impl;
	const loader_path *paths;
	size_t size;
	napi_ref handle_ref;

	loader_impl_async_load_from_file_safe_type(loader_impl_node node_impl, const loader_path *paths, size_t size) :
		node_impl(node_impl), paths(paths), size(size), handle_ref(nullptr) {}
};

struct loader_impl_async_load_from_memory_safe_type
{
	loader_impl_node node_impl;
	const char *name;
	const char *buffer;
	size_t size;
	napi_ref handle_ref;

	loader_impl_async_load_from_memory_safe_type(loader_impl_node node_impl, const char *name, const char *buffer, size_t size) :
		node_impl(node_impl), name(name), buffer(buffer), size(size), handle_ref(nullptr) {}
};

struct loader_impl_async_clear_safe_type
{
	loader_impl_node node_impl;
	napi_ref handle_ref;

	loader_impl_async_clear_safe_type(loader_impl_node node_impl, napi_ref handle_ref) :
		node_impl(node_impl), handle_ref(handle_ref) {}
};

struct loader_impl_async_discover_function_safe_type
{
	loader_impl_node node_impl;
	napi_value func;

	loader_impl_async_discover_function_safe_type(loader_impl_node node_impl, napi_value func) :
		node_impl(node_impl), func(func) {}
};

struct loader_impl_async_discover_safe_type
{
	loader_impl_node node_impl;
	napi_ref handle_ref;
	context ctx;
	int result;

	loader_impl_async_discover_safe_type(loader_impl_node node_impl, napi_ref handle_ref, context ctx) :
		node_impl(node_impl), handle_ref(handle_ref), ctx(ctx), result(0) {}
};

struct loader_impl_async_func_call_safe_type
{
	loader_impl_node node_impl;
	function func;
	loader_impl_node_function node_func;
	void **args;
	size_t size;
	napi_value recv;
	function_return ret;

	loader_impl_async_func_call_safe_type(loader_impl_node node_impl, function func, loader_impl_node_function node_func, function_args args, size_t size) :
		node_impl(node_impl), func(func), node_func(node_func), args(static_cast<void **>(args)), size(size), recv(nullptr), ret(NULL) {}
};

struct loader_impl_async_func_await_safe_type
{
	loader_impl_node node_impl;
	function func;
	loader_impl_node_function node_func;
	void **args;
	size_t size;
	function_resolve_callback resolve_callback;
	function_reject_callback reject_callback;
	void *context;
	napi_value recv;
	function_return ret;

	loader_impl_async_func_await_safe_type(loader_impl_node node_impl, function func, loader_impl_node_function node_func, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context) :
		node_impl(node_impl), func(func), node_func(node_func), args(static_cast<void **>(args)), size(size), resolve_callback(resolve_callback), reject_callback(reject_callback), context(context), recv(nullptr), ret(NULL) {}
};

struct loader_impl_async_future_await_safe_type
{
	loader_impl_node node_impl;
	future f;
	loader_impl_node_future node_future;
	function_resolve_callback resolve_callback;
	function_reject_callback reject_callback;
	void *context;
	napi_value recv;
	future_return ret;

	loader_impl_async_future_await_safe_type(loader_impl_node node_impl, future f, loader_impl_node_future node_future, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context) :
		node_impl(node_impl), f(f), node_future(node_future), resolve_callback(resolve_callback), reject_callback(reject_callback), context(context), recv(nullptr), ret(NULL) {}
};

struct loader_impl_async_func_destroy_safe_type
{
	loader_impl_node node_impl;
	loader_impl_node_function node_func;

	loader_impl_async_func_destroy_safe_type(loader_impl_node node_impl, loader_impl_node_function node_func) :
		node_impl(node_impl), node_func(node_func) {}
};

struct loader_impl_async_future_delete_safe_type
{
	loader_impl_node node_impl;
	future f;
	loader_impl_node_future node_future;

	loader_impl_async_future_delete_safe_type(loader_impl_node node_impl, future f, loader_impl_node_future node_future) :
		node_impl(node_impl), f(f), node_future(node_future) {}
};

struct loader_impl_async_destroy_safe_type
{
	loader_impl_node node_impl;
	bool has_finished;

	loader_impl_async_destroy_safe_type(loader_impl_node node_impl) :
		node_impl(node_impl), has_finished(false) {}
};

struct loader_impl_node_type
{
	/* TODO: The current implementation may not support multi-isolate environments. We should test it. */
	napi_env env;						/* Used for storing environment for reentrant calls */
	napi_ref global_ref;				/* Store global reference */
	napi_ref function_table_object_ref; /* Store function table reference registered by the trampoline */

	loader_impl_threadsafe_type<loader_impl_async_initialize_safe_type> threadsafe_initialize;
	loader_impl_threadsafe_type<loader_impl_async_execution_path_safe_type> threadsafe_execution_path;
	loader_impl_threadsafe_type<loader_impl_async_load_from_file_safe_type> threadsafe_load_from_file;
	loader_impl_threadsafe_type<loader_impl_async_load_from_memory_safe_type> threadsafe_load_from_memory;
	loader_impl_threadsafe_type<loader_impl_async_clear_safe_type> threadsafe_clear;
	loader_impl_threadsafe_type<loader_impl_async_discover_safe_type> threadsafe_discover;
	loader_impl_threadsafe_type<loader_impl_async_func_call_safe_type> threadsafe_func_call;
	loader_impl_threadsafe_type<loader_impl_async_func_await_safe_type> threadsafe_func_await;
	loader_impl_threadsafe_type<loader_impl_async_func_destroy_safe_type> threadsafe_func_destroy;
	loader_impl_threadsafe_type<loader_impl_async_future_await_safe_type> threadsafe_future_await;
	loader_impl_threadsafe_type<loader_impl_async_future_delete_safe_type> threadsafe_future_delete;
	loader_impl_threadsafe_type<loader_impl_async_destroy_safe_type> threadsafe_destroy;

	uv_thread_t thread;
	uv_loop_t *thread_loop;

	uv_mutex_t mutex;
	uv_cond_t cond;

	int stdin_copy;
	int stdout_copy;
	int stderr_copy;

#ifdef __ANDROID__
	int pfd[2];
	uv_thread_t thread_log_id;
#endif

	int result;
	const char *error_message;

	/* TODO: This implementation won't work for multi-isolate environments. We should test it. */
	std::thread::id js_thread_id;

	int64_t base_active_handles;
	std::atomic_int64_t extra_active_handles;
	uv_prepare_t destroy_prepare;
	uv_check_t destroy_check;
	std::atomic_bool event_loop_empty;
	loader_impl impl;
};

template <typename T>
struct loader_impl_threadsafe_async_type
{
	uv_async_t async_handle;
	bool initialized;

	loader_impl_threadsafe_async_type() :
		initialized(false) {}

	int initialize(loader_impl_node node_impl, void (*async_cb)(uv_async_t *))
	{
		int result = uv_async_init(node_impl->thread_loop, &async_handle, async_cb);

		initialized = (result == 0);

		return result;
	}

	void invoke(T *data)
	{
		if (initialized)
		{
			async_handle.data = static_cast<T *>(data);
			uv_async_send(&async_handle);
		}
	}

	void close(void (*close_cb)(uv_handle_t *handle))
	{
		if (initialized)
		{
			union
			{
				uv_handle_t *handle;
				uv_async_t *async;
			} handle_cast;

			handle_cast.async = &async_handle;

			uv_close(handle_cast.handle, close_cb);
		}
	}
};

struct loader_impl_async_handle_promise_safe_type
{
	loader_impl_node node_impl;
	napi_env env;
	napi_deferred deferred;
	void *result;
	napi_status (*deferred_fn)(napi_env, napi_deferred, napi_value);
	const char *error_str;
	loader_impl_threadsafe_async_type<loader_impl_async_handle_promise_safe_type> threadsafe_async;

	loader_impl_async_handle_promise_safe_type(loader_impl_node node_impl, napi_env env) :
		node_impl(node_impl), env(env), result(NULL) {}

	void destroy()
	{
		threadsafe_async.close([](uv_handle_t *handle) {
			loader_impl_async_handle_promise_safe_type *handle_promise_safe = static_cast<loader_impl_async_handle_promise_safe_type *>(handle->data);

			if (handle_promise_safe->result != NULL)
			{
				metacall_value_destroy(handle_promise_safe->result);
			}

			delete handle_promise_safe;
		});
	}
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
	void *context;

} * loader_impl_async_func_await_trampoline;

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
static napi_value node_loader_impl_napi_to_value_callback(napi_env env, napi_callback_info info);

/* Function */
static int function_node_interface_create(function func, function_impl impl);

static function_return function_node_interface_invoke(function func, function_impl impl, function_args args, size_t size);

static function_return function_node_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context);

static void function_node_interface_destroy(function func, function_impl impl);

static function_interface function_node_singleton(void);

/* Future */
static int future_node_interface_create(future f, future_impl impl);

static future_return future_node_interface_await(future f, future_impl impl, future_resolve_callback resolve_callback, future_reject_callback reject_callback, void *context);

static void future_node_interface_destroy(future f, future_impl impl);

static future_interface future_node_singleton(void);

/* JavaScript Thread Safe */
static void node_loader_impl_initialize_safe(napi_env env, loader_impl_async_initialize_safe_type *initialize_safe);

static void node_loader_impl_execution_path_safe(napi_env env, loader_impl_async_execution_path_safe_type *execution_path_safe);

static void node_loader_impl_func_call_safe(napi_env env, loader_impl_async_func_call_safe_type *func_call_safe);

static void node_loader_impl_func_await_safe(napi_env env, loader_impl_async_func_await_safe_type *func_await_safe);

static void node_loader_impl_func_destroy_safe(napi_env env, loader_impl_async_func_destroy_safe_type *func_destroy_safe);

static void node_loader_impl_future_await_safe(napi_env env, loader_impl_async_future_await_safe_type *future_await_safe);

static void node_loader_impl_future_delete_safe(napi_env env, loader_impl_async_future_delete_safe_type *future_delete_safe);

static void node_loader_impl_load_from_file_safe(napi_env env, loader_impl_async_load_from_file_safe_type *load_from_file_safe);

static void node_loader_impl_load_from_memory_safe(napi_env env, loader_impl_async_load_from_memory_safe_type *load_from_memory_safe);

static void node_loader_impl_clear_safe(napi_env env, loader_impl_async_clear_safe_type *clear_safe);

static value node_loader_impl_discover_function_safe(napi_env env, loader_impl_async_discover_function_safe_type *discover_function_safe);

static void node_loader_impl_discover_safe(napi_env env, loader_impl_async_discover_safe_type *discover_safe);

static void node_loader_impl_handle_promise_safe(napi_env env, loader_impl_async_handle_promise_safe_type *handle_promise_safe);

static void node_loader_impl_destroy_safe(napi_env env, loader_impl_async_destroy_safe_type *destroy_safe);

static char *node_loader_impl_get_property_as_char(napi_env env, napi_value obj, const char *prop);

/* Loader */
static void *node_loader_impl_register(void *node_impl_ptr, void *env_ptr, void *function_table_object_ptr);

static void node_loader_impl_thread(void *data);

#ifdef __ANDROID__
static void node_loader_impl_thread_log(void *data);
#endif

/* static void node_loader_impl_walk(uv_handle_t *handle, void *data); */

#if (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
static void node_loader_impl_walk_async_handles_count(uv_handle_t *handle, void *arg);
#endif

static int64_t node_loader_impl_async_handles_count(loader_impl_node node_impl);

static void node_loader_impl_try_destroy(loader_impl_node node_impl);

#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER >= 1200)
/* Required for the DelayLoad hook interposition, solves bug of NodeJS extensions requiring node.exe instead of node.dll */
static HMODULE node_loader_node_dll_handle = NULL;
static HMODULE (*get_module_handle_a_ptr)(_In_opt_ LPCSTR) = NULL; /* TODO: Implement W version too? */
#endif

/* -- Methods -- */

#if 1 // NODE_MAJOR_VERSION < 18
	#if NODE_MAJOR_VERSION >= 12
		#define node_loader_impl_register_module_id node::ModuleFlags::kLinked | 0x08 /* NM_F_DELETEME */
	#else
		#define node_loader_impl_register_module_id 0x02 | 0x08 /* NM_F_LINKED | NM_F_DELETEME */
	#endif

	#define node_loader_impl_register_module(name, fn) \
		do \
		{ \
			static napi_module node_loader_module = { \
				NAPI_MODULE_VERSION, \
				node_loader_impl_register_module_id, \
				__FILE__, \
				fn, \
				name, \
				NULL, \
				{ 0 } \
			}; \
			napi_module_register(&node_loader_module); \
		} while (0)

void node_loader_impl_register_linked_bindings()
{
	/* Initialize Node Loader Trampoline */
	node_loader_impl_register_module("node_loader_trampoline_module", node_loader_trampoline_initialize);

	/* Initialize Node Loader Port */
	node_loader_impl_register_module("node_loader_port_module", node_loader_port_initialize);
}
#else
// TODO: New register implementation
#endif

void node_loader_impl_exception(napi_env env, napi_status status)
{
	if (status != napi_ok)
	{
		if (status != napi_pending_exception)
		{
			const napi_extended_error_info *error_info = nullptr;

			bool pending;

			napi_get_last_error_info(env, &error_info);

			napi_is_exception_pending(env, &pending);

			const char *message = (error_info != nullptr && error_info->error_message != nullptr) ? error_info->error_message : "Error message not available";

			/* TODO: Notify MetaCall error handling system when it is implemented */
			/* ... */

			if (pending)
			{
				napi_throw_error(env, nullptr, message);
			}
		}
		else
		{
			napi_value error, message;
			bool result;
			napi_valuetype valuetype;

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

			size_t length;

			status = napi_get_value_string_utf8(env, message, nullptr, 0, &length);

			node_loader_impl_exception(env, status);

			char *str = new char[length + 1];

			if (str == nullptr)
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

			delete[] str;
		}
	}
}

value node_loader_impl_exception_value(loader_impl_node node_impl, napi_env env, napi_status status, napi_value recv)
{
	value ret = NULL;

	if (status != napi_ok)
	{
		if (status != napi_pending_exception)
		{
			const napi_extended_error_info *error_info = nullptr;

			bool pending;

			napi_get_last_error_info(env, &error_info);

			napi_is_exception_pending(env, &pending);

			const char *message = (error_info != nullptr && error_info->error_message != nullptr) ? error_info->error_message : "Error message not available";

			exception ex = exception_create_const(message, "ExceptionPending", (int64_t)(error_info != nullptr ? error_info->error_code : status), "");

			throwable th = throwable_create(value_create_exception(ex));

			ret = value_create_throwable(th);

			if (pending)
			{
				napi_throw_error(env, nullptr, message);
			}
		}
		else
		{
			napi_value error;
			bool result;

			status = napi_get_and_clear_last_exception(env, &error);

			node_loader_impl_exception(env, status);

			status = napi_is_error(env, error, &result);

			node_loader_impl_exception(env, status);

			if (result == false)
			{
				value thrown_value = node_loader_impl_napi_to_value(node_impl, env, recv, error);

				throwable th = throwable_create(thrown_value);

				ret = value_create_throwable(th);
			}
			else
			{
				exception ex = exception_create(
					node_loader_impl_get_property_as_char(env, error, "message"),
					node_loader_impl_get_property_as_char(env, error, "code"),
					(int64_t)status,
					node_loader_impl_get_property_as_char(env, error, "stack"));

				throwable th = throwable_create(value_create_exception(ex));

				ret = value_create_throwable(th);
			}
		}
	}

	return ret;
}

template <typename T>
void node_loader_impl_finalizer_impl(napi_env env, napi_value v, void *data, T finalizer)
{
	napi_status status;

// Create a finalizer for the value
#if (NAPI_VERSION < 5)
	{
		napi_value symbol, external;

		status = napi_create_symbol(env, nullptr, &symbol);

		node_loader_impl_exception(env, status);

		status = napi_create_external(env, data, finalizer, nullptr, &external);

		node_loader_impl_exception(env, status);

		napi_property_descriptor desc = {
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

void node_loader_impl_finalizer(napi_env env, napi_value v, void *data)
{
	if (value_type_id(data) == TYPE_NULL)
	{
		value_type_destroy(data);
		return;
	}

	auto finalizer = [](napi_env, void *finalize_data, void *) {
		value_type_destroy(finalize_data);
	};

	node_loader_impl_finalizer_impl(env, v, data, finalizer);
}

napi_value node_loader_impl_get_property_as_string(napi_env env, napi_value obj, const char *prop)
{
	napi_valuetype valuetype;
	napi_value result;
	napi_status status = napi_get_named_property(env, obj, prop, &result);

	node_loader_impl_exception(env, status);

	status = napi_typeof(env, result, &valuetype);

	node_loader_impl_exception(env, status);

	if (valuetype != napi_string)
	{
		napi_value result_as_string;

		status = napi_coerce_to_string(env, result, &result_as_string);

		node_loader_impl_exception(env, status);

		return result_as_string;
	}

	return result;
}

char *node_loader_impl_get_property_as_char(napi_env env, napi_value obj, const char *prop)
{
	napi_value prop_value = node_loader_impl_get_property_as_string(env, obj, prop);
	size_t length;
	napi_status status = napi_get_value_string_utf8(env, prop_value, nullptr, 0, &length);

	node_loader_impl_exception(env, status);

	char *str = static_cast<char *>(malloc(sizeof(char) * (length + 1)));

	if (str == NULL)
	{
		/* TODO: Notify MetaCall error handling system when it is implemented */
		return NULL;
	}

	status = napi_get_value_string_utf8(env, prop_value, str, length + 1, &length);

	node_loader_impl_exception(env, status);

	return str;
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

		status = napi_get_value_string_utf8(env, v, nullptr, 0, &length);

		node_loader_impl_exception(env, status);

		ret = value_create_string(NULL, length);

		if (ret != NULL)
		{
			char *str = value_to_string(ret);

			status = napi_get_value_string_utf8(env, v, str, length + 1, &length);

			node_loader_impl_exception(env, status);
		}
	}
	else if (valuetype == napi_symbol)
	{
		/* TODO */
		napi_throw_error(env, nullptr, "NodeJS Loader symbol is not implemented");
	}
	else if (valuetype == napi_object)
	{
		bool result = false;

		if (napi_is_array(env, v, &result) == napi_ok && result == true)
		{
			uint32_t iterator, length = 0;

			value *array_value;

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
			napi_throw_error(env, nullptr, "NodeJS Loader buffer is not implemented");
		}
		else if (napi_is_error(env, v, &result) == napi_ok && result == true)
		{
			exception ex = exception_create(
				node_loader_impl_get_property_as_char(env, v, "message"),
				node_loader_impl_get_property_as_char(env, v, "code"),
				0,
				node_loader_impl_get_property_as_char(env, v, "stack"));

			ret = value_create_exception(ex);
		}
		else if (napi_is_typedarray(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
			napi_throw_error(env, nullptr, "NodeJS Loader typed array is not implemented");
		}
		else if (napi_is_dataview(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
			napi_throw_error(env, nullptr, "NodeJS Loader data view is not implemented");
		}
		else if (napi_is_promise(env, v, &result) == napi_ok && result == true)
		{
			loader_impl_node_future node_future = new loader_impl_node_future_type();

			future f;

			if (node_future == nullptr)
			{
				return static_cast<function_return>(NULL);
			}

			f = future_create(node_future, &future_node_singleton);

			if (f == NULL)
			{
				delete node_future;

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

			value *map_value;

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

				value *tupla;

				/* Create tupla */
				map_value[iterator] = value_create_array(NULL, 2);

				tupla = value_to_array(map_value[iterator]);

				/* Get key from object */
				status = napi_get_element(env, keys, iterator, &key);

				node_loader_impl_exception(env, status);

				/* Set key string in the tupla */
				status = napi_get_value_string_utf8(env, key, nullptr, 0, &key_length);

				node_loader_impl_exception(env, status);

				tupla[0] = value_create_string(NULL, key_length);

				if (tupla[0] != NULL)
				{
					napi_value element;

					char *str = value_to_string(tupla[0]);

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
		struct loader_impl_async_discover_function_safe_type discover_function_safe = {
			node_impl,
			v
		};

		/* Discover and create the function */
		return node_loader_impl_discover_function_safe(env, &discover_function_safe);
	}
	else if (valuetype == napi_external)
	{
		/* Returns the previously allocated copy */
		void *c = nullptr;

		status = napi_get_value_external(env, v, &c);

		node_loader_impl_exception(env, status);

		return c;
	}

	return ret;
}

napi_value node_loader_impl_napi_to_value_callback(napi_env env, napi_callback_info info)
{
	size_t iterator, argc = 0;

	napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);

	napi_value *argv = new napi_value[argc];
	void **args = new void *[argc];
	napi_value recv;
	loader_impl_async_safe_cast<loader_impl_napi_to_value_callback_closure> closure_cast = { nullptr };

	napi_get_cb_info(env, info, &argc, argv, &recv, &closure_cast.ptr);

	/* Set environment */
	closure_cast.safe->node_impl->env = env;

	for (iterator = 0; iterator < argc; ++iterator)
	{
		args[iterator] = node_loader_impl_napi_to_value(closure_cast.safe->node_impl, env, recv, argv[iterator]);
	}

	void *ret = metacallfv_s(value_to_function(closure_cast.safe->func), args, argc);

	napi_value result = node_loader_impl_value_to_napi(closure_cast.safe->node_impl, env, ret);

	if (value_type_id(ret) == TYPE_THROWABLE)
	{
		napi_throw(env, result);
	}

	for (iterator = 0; iterator < argc; ++iterator)
	{
		value_type_destroy(args[iterator]);
	}

	value_type_destroy(ret);

	/* Reset environment */
	// closure_cast.safe->node_impl->env = nullptr;

	delete[] argv;
	delete[] args;

	return result;
}

napi_value node_loader_impl_value_to_napi(loader_impl_node node_impl, napi_env env, value arg_value)
{
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
		const char *str_value = value_to_string(arg_value);

		size_t length = value_type_size(arg_value) - 1;

		status = napi_create_string_utf8(env, str_value, length, &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_BUFFER)
	{
		void *buff_value = value_to_buffer(arg_value);

		size_t size = value_type_size(arg_value);

		status = napi_create_buffer(env, size, &buff_value, &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_ARRAY)
	{
		value *array_value = value_to_array(arg_value);

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
		value *map_value = value_to_map(arg_value);

		size_t iterator, map_size = value_type_count(arg_value);

		status = napi_create_object(env, &v);

		node_loader_impl_exception(env, status);

		for (iterator = 0; iterator < map_size; ++iterator)
		{
			value *pair_value = value_to_array(map_value[iterator]);

			const char *key = value_to_string(pair_value[0]);

			/* TODO: Review recursion overflow */
			napi_value element_v = node_loader_impl_value_to_napi(node_impl, env, static_cast<value>(pair_value[1]));

			status = napi_set_named_property(env, v, key, element_v);

			node_loader_impl_exception(env, status);
		}
	}
	else if (id == TYPE_PTR)
	{
		/* Copy value and set the ownership, the old value will be deleted after the call */
		void *c = value_copy(arg_value);

		value_move(arg_value, c);

		status = napi_create_external(env, c, nullptr, nullptr, &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_FUTURE)
	{
		/* TODO: Implement promise properly for await */
		napi_throw_error(env, nullptr, "NodeJS Loader future is not implemented");
	}
	else if (id == TYPE_FUNCTION)
	{
		loader_impl_napi_to_value_callback_closure closure = new loader_impl_napi_to_value_callback_closure_type();

		closure->func = value_type_copy(arg_value);
		closure->node_impl = node_impl;

		status = napi_create_function(env, nullptr, 0, node_loader_impl_napi_to_value_callback, closure, &v);

		node_loader_impl_exception(env, status);

		auto finalizer = [](napi_env, void *finalize_data, void *) {
			loader_impl_napi_to_value_callback_closure closure = static_cast<loader_impl_napi_to_value_callback_closure>(finalize_data);
			value_type_destroy(closure->func);
			delete closure;
		};

		node_loader_impl_finalizer_impl(env, v, closure, finalizer);
	}
	else if (id == TYPE_CLASS)
	{
		/* TODO */
		/* napi_throw_error(env, nullptr, "NodeJS Loader class is not implemented"); */

		/*
		klass cls = value_to_class(arg_value);

		napi_define_class(env, cls->name, NAPI_AUTO_LENGTH, )
		*/
	}
	else if (id == TYPE_OBJECT)
	{
		/* TODO */
		napi_throw_error(env, nullptr, "NodeJS Loader object is not implemented");
	}
	else if (id == TYPE_NULL)
	{
		status = napi_get_undefined(env, &v);

		node_loader_impl_exception(env, status);
	}
	else if (id == TYPE_EXCEPTION)
	{
		napi_value message_value, label_value, stack_value;

		exception ex = value_to_exception(arg_value);

		status = napi_create_string_utf8(env, exception_message(ex), strlen(exception_message(ex)), &message_value);

		node_loader_impl_exception(env, status);

		status = napi_create_string_utf8(env, exception_label(ex), strlen(exception_label(ex)), &label_value);

		node_loader_impl_exception(env, status);

		/* Passing code here seems not to work, instead set it once the error value has been created */
		status = napi_create_error(env, label_value, message_value, &v);

		node_loader_impl_exception(env, status);

		/* Define the stack */
		char *stack = node_loader_impl_get_property_as_char(env, v, "stack");

		std::string str_stack(exception_stacktrace(ex));

		str_stack += stack;

		status = napi_create_string_utf8(env, str_stack.c_str(), str_stack.length(), &stack_value);

		node_loader_impl_exception(env, status);

		status = napi_set_named_property(env, v, "stack", stack_value);

		node_loader_impl_exception(env, status);

		free(stack);
	}
	else if (id == TYPE_THROWABLE)
	{
		throwable th = value_to_throwable(arg_value);

		return node_loader_impl_value_to_napi(node_impl, env, throwable_value(th));
	}
	else
	{
		std::string error_str("NodeJS Loader could not convert the value of type '");
		error_str += type_id_name(id);
		error_str += "' to N-API";

		napi_throw_error(env, nullptr, error_str.c_str());
	}

	return v;
}

void node_loader_impl_env(loader_impl_node node_impl, napi_env env)
{
	node_impl->env = env;
}

int function_node_interface_create(function func, function_impl impl)
{
	loader_impl_node_function node_func = static_cast<loader_impl_node_function>(impl);

	signature s = function_signature(func);

	const size_t args_size = signature_count(s);

	node_func->argv = new napi_value[args_size];

	return (node_func->argv == nullptr);
}

template <typename T>
struct loader_impl_threadsafe_invoke_type
{
	loader_impl_threadsafe_type<T> &threadsafe_func;
	loader_impl_async_safe_type<T> async_safe;

	loader_impl_threadsafe_invoke_type(loader_impl_threadsafe_type<T> &threadsafe_func, T &func_safe) :
		threadsafe_func(threadsafe_func), async_safe(func_safe)
	{
		threadsafe_func.invoke(async_safe);
	}

	~loader_impl_threadsafe_invoke_type()
	{
		threadsafe_func.release(async_safe);
	}
};

function_return function_node_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	loader_impl_node_function node_func = static_cast<loader_impl_node_function>(impl);

	if (node_func == nullptr)
	{
		return NULL;
	}

	loader_impl_node node_impl = node_func->node_impl;
	loader_impl_async_func_call_safe_type func_call_safe(node_impl, func, node_func, args, size);

	/* Check if we are in the JavaScript thread */
	if (node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_func_call_safe(node_impl->env, &func_call_safe);

		return func_call_safe.ret;
	}

	/* Submit the task to the async queue */
	loader_impl_threadsafe_invoke_type<loader_impl_async_func_call_safe_type> invoke(node_impl->threadsafe_func_call, func_call_safe);

	return func_call_safe.ret;
}

function_return function_node_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	loader_impl_node_function node_func = static_cast<loader_impl_node_function>(impl);

	if (node_func == nullptr)
	{
		return NULL;
	}

	loader_impl_node node_impl = node_func->node_impl;
	loader_impl_async_func_await_safe_type func_await_safe(node_impl, func, node_func, args, size, resolve_callback, reject_callback, context);

	/* Check if we are in the JavaScript thread */
	if (node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_func_await_safe(node_impl->env, &func_await_safe);

		return func_await_safe.ret;
	}

	/* Submit the task to the async queue */
	loader_impl_threadsafe_invoke_type<loader_impl_async_func_await_safe_type> invoke(node_impl->threadsafe_func_await, func_await_safe);

	return func_await_safe.ret;
}

void function_node_interface_destroy(function func, function_impl impl)
{
	loader_impl_node_function node_func = static_cast<loader_impl_node_function>(impl);

	(void)func;

	if (node_func == nullptr)
	{
		return;
	}

	if (loader_is_destroyed(node_func->node_impl->impl) != 0)
	{
		loader_impl_node node_impl = node_func->node_impl;
		loader_impl_async_func_destroy_safe_type func_destroy_safe(node_impl, node_func);

		/* Check if we are in the JavaScript thread */
		if (node_impl->js_thread_id == std::this_thread::get_id())
		{
			/* We are already in the V8 thread, we can call safely */
			node_loader_impl_func_destroy_safe(node_impl->env, &func_destroy_safe);
		}
		else
		{
			/* Submit the task to the async queue */
			loader_impl_threadsafe_invoke_type<loader_impl_async_func_destroy_safe_type> invoke(node_impl->threadsafe_func_destroy, func_destroy_safe);
		}
	}

	/* Free node function arguments */
	delete[] node_func->argv;

	/* Free node function */
	delete node_func;
}

function_interface function_node_singleton()
{
	static struct function_interface_type node_function_interface = {
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

future_return future_node_interface_await(future f, future_impl impl, future_resolve_callback resolve_callback, future_reject_callback reject_callback, void *context)
{
	loader_impl_node_future node_future = static_cast<loader_impl_node_future>(impl);

	if (node_future == nullptr)
	{
		return NULL;
	}

	loader_impl_node node_impl = node_future->node_impl;
	loader_impl_async_future_await_safe_type future_await_safe(node_impl, f, node_future, resolve_callback, reject_callback, context);

	/* Check if we are in the JavaScript thread */
	if (node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_future_await_safe(node_impl->env, &future_await_safe);

		return future_await_safe.ret;
	}

	/* Submit the task to the async queue */
	loader_impl_threadsafe_invoke_type<loader_impl_async_future_await_safe_type> invoke(node_impl->threadsafe_future_await, future_await_safe);

	return future_await_safe.ret;
}

void future_node_interface_destroy(future f, future_impl impl)
{
	loader_impl_node_future node_future = static_cast<loader_impl_node_future>(impl);

	if (node_future == nullptr)
	{
		return;
	}

	if (loader_is_destroyed(node_future->node_impl->impl) != 0)
	{
		loader_impl_node node_impl = node_future->node_impl;
		loader_impl_async_future_delete_safe_type future_delete_safe(node_impl, f, node_future);

		/* Check if we are in the JavaScript thread */
		if (node_impl->js_thread_id == std::this_thread::get_id())
		{
			/* We are already in the V8 thread, we can call safely */
			node_loader_impl_future_delete_safe(node_impl->env, &future_delete_safe);
		}
		else
		{
			/* Submit the task to the async queue */
			loader_impl_threadsafe_invoke_type<loader_impl_async_future_delete_safe_type> invoke(node_impl->threadsafe_future_delete, future_delete_safe);
		}
	}

	/* Free node future */
	delete node_future;
}

future_interface future_node_singleton()
{
	static struct future_interface_type node_future_interface = {
		&future_node_interface_create,
		&future_node_interface_await,
		&future_node_interface_destroy
	};

	return &node_future_interface;
}

void node_loader_impl_initialize_safe(napi_env env, loader_impl_async_initialize_safe_type *initialize_safe)
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
		napi_value argv[1];

		status = napi_get_named_property(env, function_table_object, initialize_str, &function_trampoline_initialize);

		node_loader_impl_exception(env, status);

		status = napi_typeof(env, function_trampoline_initialize, &valuetype);

		node_loader_impl_exception(env, status);

		if (valuetype != napi_function)
		{
			napi_throw_type_error(env, nullptr, "Invalid function initialize in function table object");
		}

		/* Create parameters */
		status = napi_create_string_utf8(env, initialize_safe->loader_library_path, strlen(initialize_safe->loader_library_path), &argv[0]);

		node_loader_impl_exception(env, status);

		/* Call to load from file function */
		napi_value global, return_value;

		status = napi_get_reference_value(env, node_impl->global_ref, &global);

		node_loader_impl_exception(env, status);

		status = napi_call_function(env, global, function_trampoline_initialize, 1, argv, &return_value);

		node_loader_impl_exception(env, status);
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

void node_loader_impl_execution_path_safe(napi_env env, loader_impl_async_execution_path_safe_type *execution_path_safe)
{
	static const char execution_path_str[] = "execution_path";
	napi_value function_table_object;
	napi_value execution_path_str_value;
	bool result = false;
	napi_handle_scope handle_scope;
	loader_impl_node node_impl = execution_path_safe->node_impl;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Get function table object from reference */
	status = napi_get_reference_value(env, node_impl->function_table_object_ref, &function_table_object);

	node_loader_impl_exception(env, status);

	/* Create function string */
	status = napi_create_string_utf8(env, execution_path_str, sizeof(execution_path_str) - 1, &execution_path_str_value);

	node_loader_impl_exception(env, status);

	/* Check if exists in the table */
	status = napi_has_own_property(env, function_table_object, execution_path_str_value, &result);

	node_loader_impl_exception(env, status);

	if (result == true)
	{
		napi_value function_trampoline_execution_path;
		napi_valuetype valuetype;
		napi_value argv[1];

		status = napi_get_named_property(env, function_table_object, execution_path_str, &function_trampoline_execution_path);

		node_loader_impl_exception(env, status);

		status = napi_typeof(env, function_trampoline_execution_path, &valuetype);

		node_loader_impl_exception(env, status);

		if (valuetype != napi_function)
		{
			napi_throw_type_error(env, nullptr, "Invalid function execution_path in function table object");
		}

		/* Create parameters */
		status = napi_create_string_utf8(env, execution_path_safe->path, strlen(execution_path_safe->path), &argv[0]);

		node_loader_impl_exception(env, status);

		/* Call to load from file function */
		napi_value global, return_value;

		status = napi_get_reference_value(env, node_impl->global_ref, &global);

		node_loader_impl_exception(env, status);

		status = napi_call_function(env, global, function_trampoline_execution_path, 1, argv, &return_value);

		node_loader_impl_exception(env, status);
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

void node_loader_impl_func_call_safe(napi_env env, loader_impl_async_func_call_safe_type *func_call_safe)
{
	napi_handle_scope handle_scope;
	size_t args_size;
	value *args;
	napi_value *argv;
	loader_impl_node_function node_func;
	size_t args_count;
	signature s = function_signature(func_call_safe->func);
	const size_t signature_args_size = signature_count(s);

	/* Get function data */
	args_size = func_call_safe->size;
	node_func = func_call_safe->node_func;
	args = func_call_safe->args;

	/* Allocate dynamically more space for values in case of variable arguments */
	argv = args_size > signature_args_size ? new napi_value[args_size] : node_func->argv;

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

	func_call_safe->ret = node_loader_impl_exception_value(func_call_safe->node_impl, env, status, func_call_safe->recv);

	if (func_call_safe->ret == NULL)
	{
		/* Convert function return to value */
		func_call_safe->ret = node_loader_impl_napi_to_value(func_call_safe->node_impl, env, func_call_safe->recv, func_return);
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);

	if (args_size > signature_args_size)
	{
		delete[] argv;
	}
}

void node_loader_impl_async_func_await_finalize(napi_env, void *finalize_data, void *)
{
	loader_impl_async_func_await_trampoline trampoline = static_cast<loader_impl_async_func_await_trampoline>(finalize_data);

	delete trampoline;
}

napi_value node_loader_impl_async_func_resolve(loader_impl_node node_impl, napi_env env, function_resolve_callback resolve, napi_value recv, napi_value v, void *context)
{
	napi_value result;
	value arg, ret;

	if (node_impl == nullptr || resolve == NULL)
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

napi_value node_loader_impl_async_func_reject(loader_impl_node node_impl, napi_env env, function_reject_callback reject, napi_value recv, napi_value v, void *context)
{
	napi_value result;
	value arg, ret;

	if (node_impl == nullptr || reject == NULL)
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

void node_loader_impl_func_await_safe(napi_env env, loader_impl_async_func_await_safe_type *func_await_safe)
{
	static const char await_str[] = "await_function";
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
			loader_impl_async_func_await_trampoline trampoline = new loader_impl_async_func_await_trampoline_type();

			if (trampoline != nullptr)
			{
				size_t args_size;
				value *args;
				loader_impl_node_function node_func;
				size_t args_count;
				napi_value *func_argv;
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
				func_argv = args_size > signature_args_size ? new napi_value[args_size] : node_func->argv;

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

				status = napi_wrap(env, argv[2], static_cast<void *>(trampoline), &node_loader_impl_async_func_await_finalize, NULL, NULL);

				node_loader_impl_exception(env, status);

				/* Call to function */
				napi_value global, await_return;

				status = napi_get_reference_value(env, func_await_safe->node_impl->global_ref, &global);

				node_loader_impl_exception(env, status);

				status = napi_call_function(env, global, function_await, 3, argv, &await_return);

				node_loader_impl_exception(env, status);

				/* Proccess the await return */
				func_await_safe->ret = node_loader_impl_napi_to_value(func_await_safe->node_impl, env, func_await_safe->recv, await_return);

				if (args_size > signature_args_size)
				{
					delete[](func_argv);
				}
			}
		}
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

void node_loader_impl_func_destroy_safe(napi_env env, loader_impl_async_func_destroy_safe_type *func_destroy_safe)
{
	napi_handle_scope handle_scope;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Clear function persistent reference */
	status = napi_delete_reference(env, func_destroy_safe->node_func->func_ref);

	node_loader_impl_exception(env, status);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

void node_loader_impl_future_await_safe(napi_env env, loader_impl_async_future_await_safe_type *future_await_safe)
{
	static const char await_str[] = "await_future";
	napi_value await_str_value;
	napi_value function_table_object;
	napi_value future_await;
	bool result = false;
	napi_value argv[2];
	napi_handle_scope handle_scope;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Get function table object from reference */
	status = napi_get_reference_value(env, future_await_safe->node_impl->function_table_object_ref, &function_table_object);

	node_loader_impl_exception(env, status);

	/* Retrieve resolve function from object table */
	status = napi_create_string_utf8(env, await_str, sizeof(await_str) - 1, &await_str_value);

	node_loader_impl_exception(env, status);

	status = napi_has_own_property(env, function_table_object, await_str_value, &result);

	node_loader_impl_exception(env, status);

	if (result == true)
	{
		napi_valuetype valuetype;

		status = napi_get_named_property(env, function_table_object, await_str, &future_await);

		node_loader_impl_exception(env, status);

		status = napi_typeof(env, future_await, &valuetype);

		node_loader_impl_exception(env, status);

		if (valuetype != napi_function)
		{
			napi_throw_type_error(env, nullptr, "Invalid function await_future in function table object");
		}
		else
		{
			/* Allocate trampoline object */
			loader_impl_async_func_await_trampoline trampoline = new loader_impl_async_func_await_trampoline_type();

			if (trampoline != nullptr)
			{
				/* Get function reference */
				status = napi_get_reference_value(env, future_await_safe->node_future->promise_ref, &argv[0]);

				node_loader_impl_exception(env, status);

				/* Set trampoline object values */
				trampoline->node_loader = future_await_safe->node_impl;
				trampoline->resolve_trampoline = &node_loader_impl_async_func_resolve;
				trampoline->reject_trampoline = &node_loader_impl_async_func_reject;
				trampoline->resolve_callback = future_await_safe->resolve_callback;
				trampoline->reject_callback = future_await_safe->reject_callback;
				trampoline->context = future_await_safe->context;

				/* Set the C trampoline object as JS wrapped object */
				status = napi_create_object(env, &argv[1]);

				node_loader_impl_exception(env, status);

				status = napi_wrap(env, argv[1], static_cast<void *>(trampoline), &node_loader_impl_async_func_await_finalize, nullptr, nullptr);

				node_loader_impl_exception(env, status);

				/* Call to function */
				napi_value global, await_return;

				status = napi_get_reference_value(env, future_await_safe->node_impl->global_ref, &global);

				node_loader_impl_exception(env, status);

				status = napi_call_function(env, global, future_await, 2, argv, &await_return);

				node_loader_impl_exception(env, status);

				/* Proccess the await return */
				future_await_safe->ret = node_loader_impl_napi_to_value(future_await_safe->node_impl, env, future_await_safe->recv, await_return);
			}
		}
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

void node_loader_impl_future_delete_safe(napi_env env, loader_impl_async_future_delete_safe_type *future_delete_safe)
{
	napi_handle_scope handle_scope;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Clear promise reference */
	status = napi_delete_reference(env, future_delete_safe->node_future->promise_ref);

	node_loader_impl_exception(env, status);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

void node_loader_impl_load_from_file_safe(napi_env env, loader_impl_async_load_from_file_safe_type *load_from_file_safe)
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

			size_t length = strnlen(load_from_file_safe->paths[index], LOADER_PATH_SIZE);

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

void node_loader_impl_load_from_memory_safe(napi_env env, loader_impl_async_load_from_memory_safe_type *load_from_memory_safe)
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

void node_loader_impl_clear_safe(napi_env env, loader_impl_async_clear_safe_type *clear_safe)
{
	static const char clear_str[] = "clear";
	napi_value function_table_object;
	napi_value clear_str_value;
	bool result = false;
	napi_handle_scope handle_scope;

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
	status = napi_delete_reference(env, clear_safe->handle_ref);

	node_loader_impl_exception(env, status);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

value node_loader_impl_discover_function_safe(napi_env env, loader_impl_async_discover_function_safe_type *discover_function_safe)
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
		char *func_name_str = nullptr;
		bool has_name = false;

		status = napi_has_named_property(env, function_descriptor, "name", &has_name);

		node_loader_impl_exception(env, status);

		/* Retrieve the function name if any */
		if (has_name == true)
		{
			size_t func_name_length = 0;

			status = napi_get_named_property(env, function_descriptor, "name", &func_name);

			node_loader_impl_exception(env, status);

			status = napi_get_value_string_utf8(env, func_name, nullptr, 0, &func_name_length);

			node_loader_impl_exception(env, status);

			if (func_name_length > 0)
			{
				func_name_str = new char[func_name_length + 1];
			}

			/* Get function name */
			status = napi_get_value_string_utf8(env, func_name, func_name_str, func_name_length + 1, &func_name_length);

			node_loader_impl_exception(env, status);
		}

		/* Retrieve the function properties */
		napi_value function_sig;
		napi_value function_types = nullptr;
		napi_value function_ret = nullptr;
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
		loader_impl_node_function node_func = new loader_impl_node_function_type();

		/* Create reference to function pointer */
		status = napi_create_reference(env, discover_function_safe->func, 1, &node_func->func_ref);

		node_loader_impl_exception(env, status);

		node_func->node_impl = discover_function_safe->node_impl;
		node_func->impl = discover_function_safe->node_impl->impl;

		/* Create function */
		function f = function_create(func_name_str, (size_t)function_sig_length, node_func, &function_node_singleton);

		if (f != NULL)
		{
			signature s = function_signature(f);
			bool is_async = false;

			/* Set function async */
			status = napi_get_value_bool(env, function_is_async, &is_async);

			node_loader_impl_exception(env, status);

			function_async(f, is_async == true ? ASYNCHRONOUS : SYNCHRONOUS);

			/* Set return value if any */
			if (has_ret)
			{
				size_t return_type_length;
				char *return_type_str = nullptr;

				/* Get return value string length */
				status = napi_get_value_string_utf8(env, function_ret, nullptr, 0, &return_type_length);

				node_loader_impl_exception(env, status);

				if (return_type_length > 0)
				{
					return_type_str = new char[return_type_length + 1];
				}

				if (return_type_str != nullptr)
				{
					/* Get parameter name string */
					status = napi_get_value_string_utf8(env, function_ret, return_type_str, return_type_length + 1, &return_type_length);

					node_loader_impl_exception(env, status);

					/* TODO: Implement return type with impl (may need an important refactor) */
					signature_set_return(s, /*loader_impl_type(discover_function_safe->impl, return_type_str)*/ NULL);

					delete[] return_type_str;
				}
			}

			/* Set signature */
			for (uint32_t arg_index = 0; arg_index < function_sig_length; ++arg_index)
			{
				napi_value parameter_name;
				size_t parameter_name_length;
				char *parameter_name_str = nullptr;

				/* Get signature parameter name */
				status = napi_get_element(env, function_sig, arg_index, &parameter_name);

				node_loader_impl_exception(env, status);

				/* Get parameter name string length */
				status = napi_get_value_string_utf8(env, parameter_name, nullptr, 0, &parameter_name_length);

				node_loader_impl_exception(env, status);

				if (parameter_name_length > 0)
				{
					parameter_name_str = new char[parameter_name_length + 1];
				}

				/* Get parameter name string */
				status = napi_get_value_string_utf8(env, parameter_name, parameter_name_str, parameter_name_length + 1, &parameter_name_length);

				node_loader_impl_exception(env, status);

				/* Check if type info is available */
				if (has_types)
				{
					napi_value parameter_type;
					size_t parameter_type_length;
					char *parameter_type_str = nullptr;

					/* Get signature parameter type */
					status = napi_get_element(env, function_types, arg_index, &parameter_type);

					node_loader_impl_exception(env, status);

					/* Get parameter type string length */
					status = napi_get_value_string_utf8(env, parameter_type, nullptr, 0, &parameter_type_length);

					node_loader_impl_exception(env, status);

					if (parameter_type_length > 0)
					{
						parameter_type_str = new char[parameter_type_length + 1];
					}

					/* Get parameter type string */
					status = napi_get_value_string_utf8(env, parameter_type, parameter_type_str, parameter_type_length + 1, &parameter_type_length);

					node_loader_impl_exception(env, status);

					/* TODO: Implement parameter type with impl (may need an important refactor) */
					signature_set(s, static_cast<size_t>(arg_index), parameter_name_str, /*loader_impl_type(discover_function_safe->impl, parameter_type_str)*/ NULL);

					if (parameter_type_str != nullptr)
					{
						delete[] parameter_type_str;
					}
				}
				else
				{
					signature_set(s, static_cast<size_t>(arg_index), parameter_name_str, NULL);
				}

				if (parameter_name_str != nullptr)
				{
					delete[] parameter_name_str;
				}
			}

			/* Create value with the function */
			function_value = value_create_function(f);
		}
		else
		{
			delete node_func;
		}

		if (func_name_str != nullptr)
		{
			delete[] func_name_str;
		}
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);

	return function_value;
}

void node_loader_impl_discover_safe(napi_env env, loader_impl_async_discover_safe_type *discover_safe)
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
			char *func_name_str = nullptr;

			status = napi_get_element(env, func_names, index, &func_name);

			node_loader_impl_exception(env, status);

			status = napi_get_value_string_utf8(env, func_name, nullptr, 0, &func_name_length);

			node_loader_impl_exception(env, status);

			if (func_name_length > 0)
			{
				func_name_str = new char[func_name_length + 1];
			}

			if (func_name_str != nullptr)
			{
				napi_value function_descriptor;
				napi_value function_ptr;
				napi_value function_sig;
				napi_value function_types = nullptr;
				napi_value function_ret = nullptr;
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
				loader_impl_node_function node_func = new loader_impl_node_function_type();

				/* Create reference to function pointer */
				status = napi_create_reference(env, function_ptr, 1, &node_func->func_ref);

				node_loader_impl_exception(env, status);

				node_func->node_impl = discover_safe->node_impl;
				node_func->impl = discover_safe->node_impl->impl;

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

					function_async(f, is_async == true ? ASYNCHRONOUS : SYNCHRONOUS);

					/* Set return value if any */
					if (has_ret)
					{
						size_t return_type_length;
						char *return_type_str = nullptr;

						/* Get return value string length */
						status = napi_get_value_string_utf8(env, function_ret, nullptr, 0, &return_type_length);

						node_loader_impl_exception(env, status);

						if (return_type_length > 0)
						{
							return_type_str = new char[return_type_length + 1];
						}

						if (return_type_str != nullptr)
						{
							/* Get parameter name string */
							status = napi_get_value_string_utf8(env, function_ret, return_type_str, return_type_length + 1, &return_type_length);

							node_loader_impl_exception(env, status);

							signature_set_return(s, loader_impl_type(discover_safe->node_impl->impl, return_type_str));

							delete[] return_type_str;
						}
					}

					/* Set signature */
					for (uint32_t arg_index = 0; arg_index < function_sig_length; ++arg_index)
					{
						napi_value parameter_name;
						size_t parameter_name_length;
						char *parameter_name_str = nullptr;

						/* Get signature parameter name */
						status = napi_get_element(env, function_sig, arg_index, &parameter_name);

						node_loader_impl_exception(env, status);

						/* Get parameter name string length */
						status = napi_get_value_string_utf8(env, parameter_name, nullptr, 0, &parameter_name_length);

						node_loader_impl_exception(env, status);

						if (parameter_name_length > 0)
						{
							parameter_name_str = new char[parameter_name_length + 1];
						}

						/* Get parameter name string */
						status = napi_get_value_string_utf8(env, parameter_name, parameter_name_str, parameter_name_length + 1, &parameter_name_length);

						node_loader_impl_exception(env, status);

						/* Check if type info is available */
						if (has_types)
						{
							napi_value parameter_type;
							size_t parameter_type_length;
							char *parameter_type_str = nullptr;

							/* Get signature parameter type */
							status = napi_get_element(env, function_types, arg_index, &parameter_type);

							node_loader_impl_exception(env, status);

							/* Get parameter type string length */
							status = napi_get_value_string_utf8(env, parameter_type, nullptr, 0, &parameter_type_length);

							node_loader_impl_exception(env, status);

							if (parameter_type_length > 0)
							{
								parameter_type_str = new char[parameter_type_length + 1];
							}

							/* Get parameter type string */
							status = napi_get_value_string_utf8(env, parameter_type, parameter_type_str, parameter_type_length + 1, &parameter_type_length);

							node_loader_impl_exception(env, status);

							signature_set(s, static_cast<size_t>(arg_index), parameter_name_str, loader_impl_type(discover_safe->node_impl->impl, parameter_type_str));

							if (parameter_type_str != nullptr)
							{
								delete[] parameter_type_str;
							}
						}
						else
						{
							signature_set(s, static_cast<size_t>(arg_index), parameter_name_str, NULL);
						}

						if (parameter_name_str != nullptr)
						{
							delete[] parameter_name_str;
						}
					}

					value v = value_create_function(f);

					if (scope_define(sp, function_name(f), v) != 0)
					{
						value_type_destroy(v);
						discover_safe->result = 1;
						break;
					}
				}
				else
				{
					delete node_func;
					discover_safe->result = 1;
					break;
				}

				delete[] func_name_str;
			}
		}
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

void node_loader_impl_handle_promise_safe(napi_env env, loader_impl_async_handle_promise_safe_type *handle_promise_safe)
{
	napi_handle_scope handle_scope;

	/* Create scope */
	napi_status status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Convert MetaCall value to napi value and call resolve or reject of NodeJS */
	napi_value js_result = node_loader_impl_value_to_napi(handle_promise_safe->node_impl, env, handle_promise_safe->result);
	status = handle_promise_safe->deferred_fn(env, handle_promise_safe->deferred, js_result);

	if (status != napi_ok)
	{
		napi_throw_error(env, nullptr, handle_promise_safe->error_str);
	}

	/* Close the handle */
	handle_promise_safe->destroy();

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER >= 1200)
/* TODO: _Ret_maybenull_ HMODULE WINAPI GetModuleHandleW(_In_opt_ LPCWSTR lpModuleName); */
_Ret_maybenull_ HMODULE WINAPI get_module_handle_a_hook(_In_opt_ LPCSTR lpModuleName)
{
	/* This hooks GetModuleHandle, which is called as DelayLoad hook inside NodeJS
	* extensions in order to retrieve the executable handle, which is supposed
	* to have all N-API symbols. This trick is used because the design of NodeJS forces
	* to compile statically node.dll into the executable, but this does not happen on
	* MetaCall as it is embedded. We cannot change this behavior because it depends on
	* NodeJS extension build system, which relies on DelayLoad mechanism. So what we are
	* doing here is intercepting the GetModuleHandle call inside the DelayLoad hook, then
	* getting the address from where this Win32 API was called, and if it commes from a
	* NodeJS extension, then we return the node.dll module, otherwise we call to the original
	* GetModuleHandle funciton. This method successfully hooks into the NodeJS mechanism and
	* redirects properly the linker resolver system to the node.dll where symbols are located.
	*/
	if (lpModuleName == NULL)
	{
		HMODULE mod = NULL;

		if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
								  GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, /* Behave like GetModuleHandle */
				(LPCTSTR)_ReturnAddress(), &mod) == TRUE)
		{
			static const char node_ext[] = ".node";
			char mod_name[MAX_PATH];
			size_t length = GetModuleFileName(mod, mod_name, MAX_PATH);

			/* It must contain a letter a part from the .node extension */
			if (length > sizeof(node_ext))
			{
				char *ext = &mod_name[length - sizeof(node_ext) + 1];

				if (strncmp(ext, node_ext, sizeof(node_ext)) == 0)
				{
					return node_loader_node_dll_handle;
				}
			}
		}
	}

	return get_module_handle_a_ptr(lpModuleName);
}
#endif

void *node_loader_impl_register(void *node_impl_ptr, void *env_ptr, void *function_table_object_ptr)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(node_impl_ptr);
	napi_env env;
	napi_value function_table_object;
	napi_value global;
	napi_status status;
	napi_handle_scope handle_scope;

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl->mutex);

	/* Retrieve the js thread id */
	node_impl->js_thread_id = std::this_thread::get_id();

	/* Obtain environment and function table */
	env = static_cast<napi_env>(env_ptr);
	function_table_object = static_cast<napi_value>(function_table_object_ptr);

	/* Create scope */
	status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

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
		node_impl->threadsafe_initialize.initialize(env, "node_loader_impl_async_initialize_safe", &node_loader_impl_initialize_safe);
		node_impl->threadsafe_execution_path.initialize(env, "node_loader_impl_async_execution_path_safe", &node_loader_impl_execution_path_safe);
		node_impl->threadsafe_load_from_file.initialize(env, "node_loader_impl_async_load_from_file_safe", &node_loader_impl_load_from_file_safe);
		node_impl->threadsafe_load_from_memory.initialize(env, "node_loader_impl_async_load_from_memory_safe", &node_loader_impl_load_from_memory_safe);
		node_impl->threadsafe_clear.initialize(env, "node_loader_impl_async_clear_safe", &node_loader_impl_clear_safe);
		node_impl->threadsafe_discover.initialize(env, "node_loader_impl_async_discover_safe", &node_loader_impl_discover_safe);
		node_impl->threadsafe_func_call.initialize(env, "node_loader_impl_async_func_call_safe", &node_loader_impl_func_call_safe);
		node_impl->threadsafe_func_await.initialize(env, "node_loader_impl_async_func_await_safe", &node_loader_impl_func_await_safe);
		node_impl->threadsafe_func_destroy.initialize(env, "node_loader_impl_async_func_destroy_safe", &node_loader_impl_func_destroy_safe);
		node_impl->threadsafe_future_await.initialize(env, "node_loader_impl_async_future_await_safe", &node_loader_impl_future_await_safe);
		node_impl->threadsafe_future_delete.initialize(env, "node_loader_impl_async_future_delete_safe", &node_loader_impl_future_delete_safe);
		node_impl->threadsafe_destroy.initialize(env, "node_loader_impl_async_destroy_safe", &node_loader_impl_destroy_safe);
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

	/* Store the amount of async handles that we have for the node loader,
	* so we can count the user defined async handles */
	node_impl->base_active_handles = node_loader_impl_async_handles_count(node_impl);
	node_impl->extra_active_handles.store(0);
	node_impl->event_loop_empty.store(false);

#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	node_loader_impl_print_handles(node_impl);
#endif

	/* On Windows, hook node extension loading mechanism in order to patch extensions linked to node.exe */
#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER >= 1200)
	node_loader_node_dll_handle = GetModuleHandle(NODEJS_LIBRARY_NAME);
	get_module_handle_a_ptr = (HMODULE(*)(_In_opt_ LPCSTR))node_loader_hook_import_address_table("kernel32.dll", "GetModuleHandleA", &get_module_handle_a_hook);
#endif

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);

	/* Signal start condition */
	uv_cond_signal(&node_impl->cond);

	uv_mutex_unlock(&node_impl->mutex);

	/* TODO: Return */
	return NULL;
}

void node_loader_impl_thread(void *data)
{
	loader_impl_thread thread_data = static_cast<loader_impl_thread>(data);
	loader_impl_node node_impl = thread_data->node_impl;
	configuration config = thread_data->config;

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl->mutex);

	/* TODO: Reimplement from here to ... */
	portability_executable_path_str exe_path_str = { 0 };
	portability_executable_path_length length = 0;
	size_t exe_path_str_size = 0, exe_path_str_offset = 0;

	if (portability_executable_path(exe_path_str, &length) != 0)
	{
		/* Report error (TODO: Implement it with thread safe logs) */
		node_impl->error_message = "Node loader failed to retrieve the executable path";

		/* TODO: Make logs thread safe */
		/* log_write("metacall", LOG_LEVEL_ERROR, "Node loader failed to retrieve the executable path (%s)", exe_path_str); */

		/* Signal start condition */
		uv_cond_signal(&node_impl->cond);

		/* Unlock node implementation mutex */
		uv_mutex_unlock(&node_impl->mutex);

		return;
	}

	for (size_t iterator = 0; iterator <= (size_t)length; ++iterator)
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
	loader_path bootstrap_path_str = { 0 };
	size_t bootstrap_path_str_size = 0;

	if (node_loader_impl_bootstrap_path(bootstrap_file_str, sizeof(bootstrap_file_str) - 1, config, bootstrap_path_str, &bootstrap_path_str_size) != 0)
	{
		/* Report error (TODO: Implement it with thread safe logs) */
		node_impl->error_message = "LOADER_LIBRARY_PATH environment variable or loader_library_path field in configuration is not defined, bootstrap.js cannot be found";

		/* Signal start condition */
		uv_cond_signal(&node_impl->cond);

		/* Unlock node implementation mutex */
		uv_mutex_unlock(&node_impl->mutex);

		return;
	}

	/* Get node impl pointer */
	char *node_impl_ptr_str;
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

	node_impl_ptr_str_size = static_cast<size_t>(node_impl_ptr_length + 1);
	node_impl_ptr_str = new char[node_impl_ptr_str_size];

	if (node_impl_ptr_str == nullptr)
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
	char *register_ptr_str;
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

	register_ptr_str_size = static_cast<size_t>(register_ptr_length + 1);
	register_ptr_str = new char[register_ptr_str_size];

	if (register_ptr_str == nullptr)
	{
		delete[] node_impl_ptr_str;

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
	char *argv_str = new char[argv_str_size];

	if (argv_str == nullptr)
	{
		delete[] node_impl_ptr_str;
		delete[] register_ptr_str;

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

	delete[] node_impl_ptr_str;
	delete[] register_ptr_str;

	/* Define argv */
	char *argv[] = {
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

	// #if NODE_MAJOR_VERSION < 18
	node_loader_impl_register_linked_bindings();
	// #endif

	/* Unlock node implementation mutex */
	uv_mutex_unlock(&node_impl->mutex);

	/* Register bindings for versions older than 18 */

	/* Start NodeJS runtime */
	int result = node::Start(argc, reinterpret_cast<char **>(argv));

	/* Lock node implementation mutex */
	uv_mutex_lock(&node_impl->mutex);

	node_impl->result = result;
	delete[] argv_str;

	/* Unlock node implementation mutex */
	uv_mutex_unlock(&node_impl->mutex);
}

#ifdef __ANDROID__
void node_loader_impl_thread_log(void *data)
{
	loader_impl_node node_impl = *(static_cast<loader_impl_node *>(data));
	char buffer[128];
	size_t size = 0;

	/* Implement manual buffering for NodeJS stdio */
	while ((size = read(node_impl->pfd[0], buffer, sizeof(buffer) - 1)) > 0)
	{
		if (size > 0 && buffer[size - 1] == '\n')
		{
			--size;
		}

		buffer[size] = '\0';

		__android_log_print(ANDROID_LOG_DEBUG, "MetaCall", buffer);
	}
}
#endif

loader_impl_data node_loader_impl_initialize(loader_impl impl, configuration config)
{
	loader_impl_node node_impl;

	(void)impl;

	node_impl = new loader_impl_node_type();

	if (node_impl == nullptr)
	{
		return NULL;
	}

	/* Initialize environment for reentrant calls */
	node_impl->env = nullptr;

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

	/* Initialize execution result */
	node_impl->result = 1;
	node_impl->error_message = NULL;

	/* Initialize the reference to the loader so we can use it on the destruction */
	node_impl->impl = impl;

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

	struct loader_impl_thread_type thread_data = {
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
		loader_impl_async_initialize_safe_type initialize_safe(node_impl, value_to_string(configuration_value(config, "loader_library_path")));
		int result = 1;

		/* Check if we are in the JavaScript thread */
		if (node_impl->js_thread_id == std::this_thread::get_id())
		{
			/* We are already in the V8 thread, we can call safely */
			node_loader_impl_initialize_safe(node_impl->env, &initialize_safe);

			/* Set up return of the function call */
			result = initialize_safe.result;
		}
		else
		{
			/* Submit the task to the async queue */
			loader_impl_threadsafe_invoke_type<loader_impl_async_initialize_safe_type> invoke(node_impl->threadsafe_initialize, initialize_safe);

			/* Set up return of the function call */
			result = initialize_safe.result;
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

int node_loader_impl_execution_path(loader_impl impl, const loader_path path)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	if (node_impl == nullptr)
	{
		return 1;
	}

	loader_impl_async_execution_path_safe_type execution_path_safe(node_impl, static_cast<const char *>(path));

	/* Check if we are in the JavaScript thread */
	if (node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_execution_path_safe(node_impl->env, &execution_path_safe);
	}
	else
	{
		/* Submit the task to the async queue */
		loader_impl_threadsafe_invoke_type<loader_impl_async_execution_path_safe_type> invoke(node_impl->threadsafe_execution_path, execution_path_safe);
	}

	return 0;
}

loader_handle node_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	if (node_impl == nullptr || size == 0)
	{
		return NULL;
	}

	loader_impl_async_load_from_file_safe_type load_from_file_safe(node_impl, paths, size);

	/* Check if we are in the JavaScript thread */
	if (node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_load_from_file_safe(node_impl->env, &load_from_file_safe);

		return static_cast<loader_handle>(load_from_file_safe.handle_ref);
	}

	/* Submit the task to the async queue */
	loader_impl_threadsafe_invoke_type<loader_impl_async_load_from_file_safe_type> invoke(node_impl->threadsafe_load_from_file, load_from_file_safe);

	return static_cast<loader_handle>(load_from_file_safe.handle_ref);
}

loader_handle node_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	if (node_impl == nullptr || buffer == NULL || size == 0)
	{
		return NULL;
	}

	loader_impl_async_load_from_memory_safe_type load_from_memory_safe(node_impl, name, buffer, size);

	/* Check if we are in the JavaScript thread */
	if (node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_load_from_memory_safe(node_impl->env, &load_from_memory_safe);

		return static_cast<loader_handle>(load_from_memory_safe.handle_ref);
	}

	/* Submit the task to the async queue */
	loader_impl_threadsafe_invoke_type<loader_impl_async_load_from_memory_safe_type> invoke(node_impl->threadsafe_load_from_memory, load_from_memory_safe);

	return static_cast<loader_handle>(load_from_memory_safe.handle_ref);
}

loader_handle node_loader_impl_load_from_package(loader_impl impl, const loader_path path)
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

	loader_impl_async_clear_safe_type clear_safe(node_impl, handle_ref);

	/* Check if we are in the JavaScript thread */
	if (node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_clear_safe(node_impl->env, &clear_safe);
	}
	else
	{
		/* Submit the task to the async queue */
		loader_impl_threadsafe_invoke_type<loader_impl_async_clear_safe_type> invoke(node_impl->threadsafe_clear, clear_safe);
	}

	return 0;
}

int node_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));
	napi_ref handle_ref = static_cast<napi_ref>(handle);

	if (node_impl == nullptr || handle == nullptr || ctx == NULL)
	{
		return 1;
	}

	loader_impl_async_discover_safe_type discover_safe(node_impl, handle_ref, ctx);

	/* Check if we are in the JavaScript thread */
	if (node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_discover_safe(node_impl->env, &discover_safe);

		return discover_safe.result;
	}

	/* Submit the task to the async queue */
	loader_impl_threadsafe_invoke_type<loader_impl_async_discover_safe_type> invoke(node_impl->threadsafe_discover, discover_safe);

	return discover_safe.result;
}

void node_loader_impl_handle_promise(loader_impl_async_handle_promise_safe_type *handle_promise_safe, void *result, napi_status (*deferred_fn)(napi_env, napi_deferred, napi_value), const char error_str[])
{
	handle_promise_safe->result = metacall_value_copy(result);
	handle_promise_safe->deferred_fn = deferred_fn;
	handle_promise_safe->error_str = error_str;

	/* Check if we are in the JavaScript thread */
	if (handle_promise_safe->node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_handle_promise_safe(handle_promise_safe->env, handle_promise_safe);
	}
	else
	{
		/* Submit the task to the async queue */
		if (handle_promise_safe->threadsafe_async.initialize(handle_promise_safe->node_impl, [](uv_async_t *handle) {
				loader_impl_async_handle_promise_safe_type *handle_promise_safe = static_cast<loader_impl_async_handle_promise_safe_type *>(handle->data);
				node_loader_impl_handle_promise_safe(handle_promise_safe->env, handle_promise_safe);
			}) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Filed to initialize promise safe async handle");
		}
		else
		{
			handle_promise_safe->threadsafe_async.invoke(handle_promise_safe);
		}
	}
}

napi_value node_loader_impl_promise_await(loader_impl_node node_impl, napi_env env, const char *name, value *args, size_t size)
{
	loader_impl_async_handle_promise_safe_type *handle_promise_safe = new loader_impl_async_handle_promise_safe_type(node_impl, env);

	if (handle_promise_safe == nullptr)
	{
		napi_throw_error(env, nullptr, "Failed to allocate the promise context");

		return nullptr;
	}

	napi_value promise;

	/* Create the promise */
	napi_status status = napi_create_promise(env, &handle_promise_safe->deferred, &promise);

	if (status != napi_ok)
	{
		napi_throw_error(env, nullptr, "Failed to create the promise");

		delete handle_promise_safe;

		return nullptr;
	}

	auto resolve = [](void *result, void *data) -> void * {
		static const char promise_error_str[] = "Failed to resolve the promise";

		loader_impl_async_handle_promise_safe_type *handle_promise_safe = static_cast<loader_impl_async_handle_promise_safe_type *>(data);

		node_loader_impl_handle_promise(handle_promise_safe, result, &napi_resolve_deferred, promise_error_str);

		return NULL;
	};

	auto reject = [](void *result, void *data) -> void * {
		static const char promise_error_str[] = "Failed to reject the promise";

		loader_impl_async_handle_promise_safe_type *handle_promise_safe = static_cast<loader_impl_async_handle_promise_safe_type *>(data);

		node_loader_impl_handle_promise(handle_promise_safe, result, &napi_reject_deferred, promise_error_str);

		return NULL;
	};

	/* Await to the function */
	void *ret = metacall_await_s(name, args, size, resolve, reject, handle_promise_safe);

	if (metacall_value_id(ret) == METACALL_THROWABLE)
	{
		napi_value result = node_loader_impl_value_to_napi(node_impl, env, ret);

		napi_throw(env, result);
	}

	node_loader_impl_finalizer(env, promise, ret);

	return promise;
}

#define container_of(ptr, type, member) \
	(type *)((char *)(ptr) - (char *)&((type *)0)->member)

static void node_loader_impl_destroy_prepare_close_cb(uv_handle_t *handle)
{
	uv_prepare_t *prepare = (uv_prepare_t *)handle;
	loader_impl_node node_impl = container_of(prepare, struct loader_impl_node_type, destroy_prepare);

	if (--node_impl->extra_active_handles == 0)
	{
		node_loader_impl_try_destroy(node_impl);
	}
}

static void node_loader_impl_destroy_check_close_cb(uv_handle_t *handle)
{
	uv_check_t *check = (uv_check_t *)handle;
	loader_impl_node node_impl = container_of(check, struct loader_impl_node_type, destroy_check);

	if (--node_impl->extra_active_handles == 0)
	{
		node_loader_impl_try_destroy(node_impl);
	}
}

static void node_loader_impl_destroy_cb(loader_impl_node node_impl)
{
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	node_loader_impl_print_handles(node_impl);
#endif

	if (node_impl->event_loop_empty.load() == false && node_loader_impl_user_async_handles_count(node_impl) <= 0)
	{
		loader_impl_handle_safe_cast<uv_prepare_t> destroy_prepare_cast = { NULL };
		loader_impl_handle_safe_cast<uv_check_t> destroy_check_cast = { NULL };

		node_impl->event_loop_empty.store(true);
		uv_prepare_stop(&node_impl->destroy_prepare);
		uv_check_stop(&node_impl->destroy_check);

		destroy_prepare_cast.safe = &node_impl->destroy_prepare;
		destroy_check_cast.safe = &node_impl->destroy_check;

		uv_close(destroy_prepare_cast.handle, &node_loader_impl_destroy_prepare_close_cb);
		uv_close(destroy_check_cast.handle, &node_loader_impl_destroy_check_close_cb);
	}
}

static void node_loader_impl_destroy_prepare_cb(uv_prepare_t *handle)
{
	loader_impl_node node_impl = container_of(handle, struct loader_impl_node_type, destroy_prepare);

	node_loader_impl_destroy_cb(node_impl);
}

static void node_loader_impl_destroy_check_cb(uv_check_t *handle)
{
	loader_impl_node node_impl = container_of(handle, struct loader_impl_node_type, destroy_check);

	node_loader_impl_destroy_cb(node_impl);
}

void node_loader_impl_destroy_safe(napi_env env, loader_impl_async_destroy_safe_type *destroy_safe)
{
	napi_status status;
	napi_handle_scope handle_scope;

	loader_impl_node node_impl = destroy_safe->node_impl;

	/* Create scope */
	status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Check if there are async handles, destroy if the queue is empty, otherwise request the destroy */
	if (node_loader_impl_user_async_handles_count(node_impl) <= 0 || node_impl->event_loop_empty.load() == true)
	{
		node_loader_impl_destroy_safe_impl(node_impl, env);
		destroy_safe->has_finished = true;
	}
	else
	{
		node_impl->extra_active_handles.store(2);
		uv_prepare_init(node_impl->thread_loop, &node_impl->destroy_prepare);
		uv_check_init(node_impl->thread_loop, &node_impl->destroy_check);
		uv_prepare_start(&node_impl->destroy_prepare, &node_loader_impl_destroy_prepare_cb);
		uv_check_start(&node_impl->destroy_check, &node_loader_impl_destroy_check_cb);
	}

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);
}

#if !defined(WIN32) || defined(_WIN32)
struct node_loader_impl_uv__queue
{
	struct node_loader_impl_uv__queue *next;
	struct node_loader_impl_uv__queue *prev;
};

static inline int uv__queue_empty(const struct node_loader_impl_uv__queue *q)
{
	return q == q->next;
}
#endif

#if (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
void node_loader_impl_walk_async_handles_count(uv_handle_t *handle, void *arg)
{
	int64_t *async_count = static_cast<int64_t *>(arg);

	if (uv_is_active(handle) && !uv_is_closing(handle))
	{
		// NOTE: I am not sure if this check for timers is correct, at least for versions
		// previous from v11.0.0. Now the node loader seems to be working for versions >=v12.
		// If there is any other error when closing, improve counter for timers and timeouts.
		// Signals have been added because they do not prevent the event loop from closing.
		if (/*(!(handle->type == UV_TIMER && !uv_has_ref(handle))) ||*/ handle->type != UV_SIGNAL)
		{
			(*async_count)++;
		}
	}
}
#endif

int64_t node_loader_impl_async_closing_handles_count(loader_impl_node node_impl)
{
#if defined(WIN32) || defined(_WIN32)
	return (int64_t)(node_impl->thread_loop->pending_reqs_tail != NULL) +
		   (int64_t)(node_impl->thread_loop->endgame_handles != NULL);
#else
	union
	{
		void *data;
		const struct node_loader_impl_uv__queue *ptr;
	} uv__queue_cast;

	uv__queue_cast.data = (void *)&node_impl->thread_loop->pending_queue;

	return (int64_t)(!uv__queue_empty(uv__queue_cast.ptr)) +
		   (int64_t)(node_impl->thread_loop->closing_handles != NULL);
#endif
}

int64_t node_loader_impl_async_handles_count(loader_impl_node node_impl)
{
#if (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
	int64_t active_handles = 0;
	uv_walk(node_impl->thread_loop, node_loader_impl_walk_async_handles_count, (void *)&active_handles);

	return active_handles +
		   (int64_t)(node_impl->thread_loop->active_reqs.count > 0) +
		   node_loader_impl_async_closing_handles_count(node_impl);
#else
	int64_t active_handles = (int64_t)node_impl->thread_loop->active_handles +
							 (int64_t)(node_impl->thread_loop->active_reqs.count > 0) +
							 node_loader_impl_async_closing_handles_count(node_impl);
	return active_handles;
#endif
}

int64_t node_loader_impl_user_async_handles_count(loader_impl_node node_impl)
{
	int64_t active_handles = node_loader_impl_async_handles_count(node_impl);
	int64_t extra_active_handles = node_impl->extra_active_handles.load();

	/* TODO: Uncomment for debugging handles */
	/*
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	int64_t closing = node_loader_impl_async_closing_handles_count(node_impl);

	printf("[active_handles] - [base_active_handles] - [extra_active_handles] + [active_reqs] + [closing]\n");
	printf("       %" PRId64 "        -           %" PRId64 "          -            %" PRId64 "           +    %" PRId64 " [> 0]    +    %" PRId64 "\n",
		(int64_t)node_impl->thread_loop->active_handles,
		node_impl->base_active_handles,
		extra_active_handles,
		(int64_t)node_impl->thread_loop->active_reqs.count,
		closing);
#endif
*/

	return active_handles - node_impl->base_active_handles - extra_active_handles;
}

void node_loader_impl_print_handles(loader_impl_node node_impl)
{
	(void)node_impl;

	/* TODO: Uncomment for debugging handles */
	/*
	printf("Number of active handles: %" PRId64 "\n", node_loader_impl_async_handles_count(node_impl));
	printf("Number of user active handles: %" PRId64 "\n", node_loader_impl_user_async_handles_count(node_impl));
	uv_print_active_handles(node_impl->thread_loop, stdout);
	fflush(stdout);
	*/
}

#if 0
void node_loader_impl_walk(uv_handle_t *handle, void *arg)
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
#endif

void node_loader_impl_destroy_safe_impl(loader_impl_node node_impl, napi_env env)
{
	napi_status status;
	napi_handle_scope handle_scope;

	/* Destroy children loaders */
	loader_unload_children(node_impl->impl);

	/* Create scope */
	status = napi_open_handle_scope(env, &handle_scope);

	node_loader_impl_exception(env, status);

	/* Clear thread safe functions except by destroy one */
	{
		node_impl->threadsafe_initialize.abort(env);
		node_impl->threadsafe_execution_path.abort(env);
		node_impl->threadsafe_load_from_file.abort(env);
		node_impl->threadsafe_load_from_memory.abort(env);
		node_impl->threadsafe_clear.abort(env);
		node_impl->threadsafe_discover.abort(env);
		node_impl->threadsafe_func_call.abort(env);
		node_impl->threadsafe_func_await.abort(env);
		node_impl->threadsafe_func_destroy.abort(env);
		node_impl->threadsafe_future_await.abort(env);
		node_impl->threadsafe_future_delete.abort(env);
	}

	/* Clear persistent references */
	status = napi_delete_reference(env, node_impl->global_ref);

	node_loader_impl_exception(env, status);

	status = napi_delete_reference(env, node_impl->function_table_object_ref);

	node_loader_impl_exception(env, status);

	/* Close scope */
	status = napi_close_handle_scope(env, handle_scope);

	node_loader_impl_exception(env, status);

	/* NodeJS Loader needs to register that it is destroyed, because after this step
	* some destructors can be still triggered, before the node_loader->destroy() has
	* finished, so this destructors will try to execute the NodeJS unrefs while having
	* the runtime (at least the NodeJS Loader related part) destroyed.
	*/
	loader_set_destroyed(node_impl->impl);
}

void node_loader_impl_try_destroy(loader_impl_node node_impl)
{
	loader_impl_async_destroy_safe_type destroy_safe(node_impl);

	/* Check if we are in the JavaScript thread */
	if (node_impl->js_thread_id == std::this_thread::get_id())
	{
		/* We are already in the V8 thread, we can call safely */
		node_loader_impl_destroy_safe(node_impl->env, &destroy_safe);
	}
	else
	{
		/* Submit the task to the async queue */
		loader_impl_threadsafe_invoke_type<loader_impl_async_destroy_safe_type> invoke(node_impl->threadsafe_destroy, destroy_safe);
	}

	if (destroy_safe.has_finished)
	{
		node_impl->threadsafe_destroy.abort(node_impl->env);
	}
}

int node_loader_impl_destroy(loader_impl impl)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	if (node_impl == NULL)
	{
		return 1;
	}

	/* Call destroy function with thread safe */
	node_loader_impl_try_destroy(node_impl);

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
	log_write("metacall", LOG_LEVEL_DEBUG, "NodeJS execution return status %d", node_impl->result);

	/* Restore stdin, stdout, stderr */
	dup2(node_impl->stdin_copy, STDIN_FILENO);
	dup2(node_impl->stdout_copy, STDOUT_FILENO);
	dup2(node_impl->stderr_copy, STDERR_FILENO);

	delete node_impl;

	return 0;
}
