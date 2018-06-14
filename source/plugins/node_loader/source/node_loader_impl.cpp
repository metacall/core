/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading nodejs code at run-time into a process.
 *
 */

#include <node_loader/node_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <log/log.h>

#include <cstdlib>
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

#ifndef NODE_LOADER_PROCESS_TITLE
#	define NODE_LOADER_PROCESS_TITLE "node-loader-testd"
#endif /* NODE_LOADER_PROCESS_TITLE */

#ifndef container_of
#	define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))
#endif

typedef struct loader_impl_node_type
{
	napi_env env;
	napi_ref global_ref;
	napi_ref function_table_object_ref;

	uv_thread_t thread_id;
	uv_loop_t * thread_loop;

	uv_mutex_t mutex_initialize;
	uv_mutex_t mutex_load_from_file;
	uv_mutex_t mutex_clear;
	uv_mutex_t mutex_discover;

	uv_cond_t cond_initialize;
	uv_cond_t cond_load_from_file;
	uv_cond_t cond_clear;
	uv_cond_t cond_discover;

	uv_async_t async_initialize;
	uv_async_t async_load_from_file;
	uv_async_t async_clear;
	uv_async_t async_discover;
	uv_async_t async_call;
	uv_async_t async_destroy;

} * loader_impl_node;

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

typedef struct loader_impl_node_function_type
{
	loader_impl_node node_impl;
	/* TODO: Check if func and sig has to be persistent */
	napi_value func;
	napi_value sig;

} * loader_impl_node_function;

/* Function */
int function_node_interface_create(function func, function_impl impl);

function_return function_node_interface_invoke(function func, function_impl impl, function_args args);

void function_node_interface_destroy(function func, function_impl impl);

function_interface function_node_singleton(void);

/* Async */
void node_loader_impl_async_initialize(uv_async_t * async);

void node_loader_impl_async_call(uv_async_t * async);

void node_loader_impl_async_load_from_file(uv_async_t * async);

void node_loader_impl_async_clear(uv_async_t * async);

void node_loader_impl_async_discover(uv_async_t * async);

/* Loader */
void * node_loader_impl_register(void * node_impl_ptr, void * env_ptr, void * function_table_object_ptr);

void node_loader_impl_thread(void * data);

void node_loader_impl_walk(uv_handle_t * handle, void * data);

void node_loader_impl_async_destroy(uv_async_t * async);

int function_node_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;
	/*
	loader_impl_node_function node_func = (loader_impl_node_function)impl;

	signature s = function_signature(func);

	const size_t args_size = signature_count(s);

	if (args_size > 0)
	{
		node_func->values = malloc(sizeof(PyObject *) * args_size);

		if (node_func->values != NULL)
		{
			size_t iterator;

			for (iterator = 0; iterator < args_size; ++iterator)
			{
				node_func->values[iterator] = NULL;
			}

			return 0;
		}

		return 1;
	}

	node_func->values = NULL;
	*/

	return 0;
}

function_return function_node_interface_invoke(function func, function_impl impl, function_args args)
{
	(void)func;
	(void)impl;
	(void)args;
	/*
	loader_impl_node_function node_func = (loader_impl_node_function)impl;

	signature s = function_signature(func);

	const size_t args_size = signature_count(s);

	type ret_type = signature_get_return(s);

	PyObject * tuple_args = PyTuple_New(args_size);

	PyObject * result = NULL;

	size_t args_count;

	PyGILState_STATE gstate = PyGILState_Ensure();

	for (args_count = 0; args_count < args_size; ++args_count)
	{
		type t = signature_get_type(s, args_count);

		type_id id = TYPE_INVALID;

		if (t == NULL)
		{
			id = value_type_id((value)args[args_count]);
		}
		else
		{
			id = type_index(t);
		}

		log_write("metacall", LOG_LEVEL_DEBUG, "Type (%p): %d", (void *)t, id);

		if (id == TYPE_BOOL)
		{
			boolean * value_ptr = (boolean *)(args[args_count]);

			long l = (*value_ptr == 0) ? 0L : 1L;

			node_func->values[args_count] = PyBool_FromLong(l);
		}
		else if (id == TYPE_INT)
		{
			int * value_ptr = (int *)(args[args_count]);

			#if PY_MAJOR_VERSION == 2
				node_func->values[args_count] = PyInt_FromLong(*value_ptr);
			#elif PY_MAJOR_VERSION == 3
				long l = (long)(*value_ptr);

				node_func->values[args_count] = PyLong_FromLong(l);
			#endif
		}
		else if (id == TYPE_LONG)
		{
			long * value_ptr = (long *)(args[args_count]);

			node_func->values[args_count] = PyLong_FromLong(*value_ptr);
		}
		else if (id == TYPE_FLOAT)
		{
			float * value_ptr = (float *)(args[args_count]);

			node_func->values[args_count] = PyFloat_FromDouble((double)*value_ptr);
		}
		else if (id == TYPE_DOUBLE)
		{
			double * value_ptr = (double *)(args[args_count]);

			node_func->values[args_count] = PyFloat_FromDouble(*value_ptr);
		}
		else if (id == TYPE_STRING)
		{
			const char * value_ptr = (const char *)(args[args_count]);

			#if PY_MAJOR_VERSION == 2
				node_func->values[args_count] = PyString_FromString(value_ptr);
			#elif PY_MAJOR_VERSION == 3
				node_func->values[args_count] = PyUnicode_FromString(value_ptr);
			#endif

		}
		else if (id == TYPE_PTR)
		{
		}

		if (node_func->values[args_count] != NULL)
		{
			PyTuple_SetItem(tuple_args, args_count, node_func->values[args_count]);
		}
	}

	result = PyObject_CallObject(node_func->func, tuple_args);

	if (PyErr_Occurred() != NULL)
	{
		loader_impl_node node_impl = loader_impl_get(node_func->impl);

		node_loader_impl_error_print(node_impl);
	}

	Py_DECREF(tuple_args);

	if (result != NULL)
	{
		value v = NULL;

		type_id id = TYPE_INVALID;

		if (ret_type == NULL)
		{
			id = node_loader_impl_get_return_type(result);
		}
		else
		{
			id = type_index(ret_type);
		}

		log_write("metacall", LOG_LEVEL_DEBUG, "Return type %p, %d", (void *)ret_type, id);

		if (id == TYPE_BOOL)
		{
			boolean b = (PyObject_IsTrue(result) == 1) ? 1 : 0;

			v = value_create_bool(b);
		}
		else if (id == TYPE_INT)
		{
			#if PY_MAJOR_VERSION == 2
				long l = PyInt_AsLong(result);
			#elif PY_MAJOR_VERSION == 3
				long l = PyLong_AsLong(result);
			#endif

			*//* TODO: Review overflow *//*
			int i = (int)l;

			v = value_create_int(i);
		}
		else if (id == TYPE_LONG)
		{
			long l = PyLong_AsLong(result);

			v = value_create_long(l);
		}
		else if (id == TYPE_FLOAT)
		{
			double d = PyFloat_AsDouble(result);

			v = value_create_float((float)d);
		}
		else if (id == TYPE_DOUBLE)
		{
			double d = PyFloat_AsDouble(result);

			v = value_create_double(d);
		}
		else if (id == TYPE_STRING)
		{
			char * str = NULL;

			Py_ssize_t length = 0;

			#if PY_MAJOR_VERSION == 2
				if (PyString_AsStringAndSize(result, &str, &length) == -1)
				{
					if (PyErr_Occurred() != NULL)
					{
						loader_impl_node node_impl = loader_impl_get(node_func->impl);

						node_loader_impl_error_print(node_impl);
					}
				}
			#elif PY_MAJOR_VERSION == 3
				str = PyUnicode_AsUTF8AndSize(result, &length);
			#endif

			v = value_create_string(str, (size_t)length);
		}
		else if (id == TYPE_PTR)
		{
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Unrecognized return type");
		}

		Py_DECREF(result);

		PyGILState_Release(gstate);

		return v;
	}

	PyGILState_Release(gstate);
	*/

	return NULL;
}

void function_node_interface_destroy(function func, function_impl impl)
{
	loader_impl_node_function node_func = (loader_impl_node_function)impl;

	(void)func;

	if (node_func != NULL)
	{
		/*
		if (node_func->values != NULL)
		{
			(void)func;

			free(node_func->values);
		}

		Py_DECREF(node_func->func);
		*/

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
	loader_impl_node node_impl = *(static_cast<loader_impl_node *>(async->data));

	(void)node_impl;

	/* Signal start condition */
	uv_mutex_lock(&node_impl->mutex_initialize);

	uv_cond_signal(&node_impl->cond_initialize);

	uv_mutex_unlock(&node_impl->mutex_initialize);
}

void node_loader_impl_async_call(uv_async_t * async)
{
	/* TODO: Parameter will be a reflect function type in the future */
	loader_impl_node node_impl = *(static_cast<loader_impl_node *>(async->data));

	(void)node_impl;
}

void node_loader_impl_async_load_from_file(uv_async_t * async)
{
	loader_impl_async_load_from_file async_data = static_cast<loader_impl_async_load_from_file>(async->data);

	napi_env env = async_data->node_impl->env;
	napi_value function_table_object;

	const char load_from_file_str[] = "load_from_file";
	napi_value load_from_file_str_value;

	bool result = false;

	napi_handle_scope handle_scope;

	/* Create scope */
	napi_status status = napi_open_handle_scope(async_data->node_impl->env, &handle_scope);

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
	uv_mutex_lock(&async_data->node_impl->mutex_load_from_file);

	uv_cond_signal(&async_data->node_impl->cond_load_from_file);

	uv_mutex_unlock(&async_data->node_impl->mutex_load_from_file);
}

void node_loader_impl_async_clear(uv_async_t * async)
{
	loader_impl_async_clear async_data = static_cast<loader_impl_async_clear>(async->data);

	napi_env env = async_data->node_impl->env;
	napi_value function_table_object;

	const char clear_str[] = "clear";
	napi_value clear_str_value;

	bool result = false;

	napi_handle_scope handle_scope;

	uint32_t ref_count = 0;

	/* Create scope */
	napi_status status = napi_open_handle_scope(async_data->node_impl->env, &handle_scope);

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
	uv_mutex_lock(&async_data->node_impl->mutex_clear);

	uv_cond_signal(&async_data->node_impl->cond_clear);

	uv_mutex_unlock(&async_data->node_impl->mutex_clear);
}

void node_loader_impl_async_discover(uv_async_t * async)
{
	loader_impl_async_discover async_data = static_cast<loader_impl_async_discover>(async->data);

	napi_env env = async_data->node_impl->env;
	napi_value function_table_object;

	const char discover_str[] = "discover";
	napi_value discover_str_value;

	bool result = false;

	napi_handle_scope handle_scope;

	/* Create scope */
	napi_status status = napi_open_handle_scope(async_data->node_impl->env, &handle_scope);

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

				/* Get function signature */
				status = napi_get_named_property(env, function_descriptor, "signature", &function_sig);

				assert(status == napi_ok);

				/* Get signature length */
				status = napi_get_array_length(env, function_sig, &function_sig_length);

				assert(status == napi_ok);

				/* Create node function */
				loader_impl_node_function node_func = static_cast<loader_impl_node_function>(malloc(sizeof(struct loader_impl_node_function_type)));

				node_func->func = function_ptr;

				node_func->sig = function_sig;

				node_func->node_impl = async_data->node_impl;

				function f = function_create(function_name_str, (size_t)function_sig_length, node_func, &function_node_singleton);

				if (f != NULL)
				{
					signature s = function_signature(f);
					scope sp = context_scope(async_data->ctx);
					size_t index;

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
	uv_mutex_lock(&async_data->node_impl->mutex_discover);

	uv_cond_signal(&async_data->node_impl->cond_discover);

	uv_mutex_unlock(&async_data->node_impl->mutex_discover);
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
			napi_value global, return_value;

			status = napi_get_global(env, &global);

			assert(status == napi_ok);

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

	/* TODO: Do a workaround with app title and argv_str (must be contigously allocated) */
	char argv_str[31 + 16 + 16 + 1] = "node-loader-testd\0bootstrap.js";

	snprintf(&argv_str[31], 16 + 1, "%p", (void *)node_impl);

	snprintf(&argv_str[46], 16 + 1, "%p", (void *)&node_loader_impl_register);

	char * argv[] = { &argv_str[0], &argv_str[18], &argv_str[31], &argv_str[46], NULL };

	int argc = 4;

	node_impl->thread_loop = uv_default_loop();

	/* Initialize initialize signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_initialize, &node_loader_impl_async_initialize);

	/* Initialize load from file signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_load_from_file, &node_loader_impl_async_load_from_file);

	/* Initialize clear signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_clear, &node_loader_impl_async_clear);

	/* Initialize discover signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_discover, &node_loader_impl_async_discover);

	/* Initialize call signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_call, &node_loader_impl_async_call);

	/* Initialize destroy signal */
	uv_async_init(node_impl->thread_loop, &node_impl->async_destroy, &node_loader_impl_async_destroy);

	/* Signal start condition */
	uv_mutex_lock(&node_impl->mutex_initialize);

	uv_cond_signal(&node_impl->cond_initialize);

	uv_mutex_unlock(&node_impl->mutex_initialize);

	/* Start NodeJS runtime */
	node::Start(argc, reinterpret_cast<char **>(argv));
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

	uv_cond_init(&node_impl->cond_initialize);

	uv_mutex_init(&node_impl->mutex_initialize);

	uv_cond_init(&node_impl->cond_load_from_file);

	uv_mutex_init(&node_impl->mutex_load_from_file);

	uv_cond_init(&node_impl->cond_clear);

	uv_mutex_init(&node_impl->mutex_clear);

	uv_cond_init(&node_impl->cond_discover);

	uv_mutex_init(&node_impl->mutex_discover);

	/* Create NodeJS thread */
	uv_thread_create(&node_impl->thread_id, node_loader_impl_thread, &node_impl);

	/* Wait until start has been launch */
	uv_mutex_lock(&node_impl->mutex_initialize);

	uv_cond_wait(&node_impl->cond_initialize, &node_impl->mutex_initialize);

	uv_mutex_unlock(&node_impl->mutex_initialize);

	/* Initialize node loader entry point */
	node_impl->async_initialize.data = static_cast<void *>(&node_impl);

	uv_async_send(&node_impl->async_initialize);

	/* Wait until script has been loaded */
	uv_mutex_lock(&node_impl->mutex_initialize);

	uv_cond_wait(&node_impl->cond_initialize, &node_impl->mutex_initialize);

	uv_mutex_unlock(&node_impl->mutex_initialize);

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
	uv_mutex_lock(&node_impl->mutex_load_from_file);

	uv_cond_wait(&node_impl->cond_load_from_file, &node_impl->mutex_load_from_file);

	uv_mutex_unlock(&node_impl->mutex_load_from_file);

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
	uv_mutex_lock(&node_impl->mutex_clear);

	uv_cond_wait(&node_impl->cond_clear, &node_impl->mutex_clear);

	uv_mutex_unlock(&node_impl->mutex_clear);

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
	uv_mutex_lock(&node_impl->mutex_discover);

	uv_cond_wait(&node_impl->cond_discover, &node_impl->mutex_discover);

	uv_mutex_unlock(&node_impl->mutex_discover);

	return 0;
}

void node_loader_impl_walk(uv_handle_t * handle, void * arg)
{
	(void)arg;

	if (!uv_is_closing(handle))
	{
		uv_close(handle, NULL);
	}
}

void node_loader_impl_async_destroy(uv_async_t * async)
{
	loader_impl_node node_impl = *(static_cast<loader_impl_node *>(async->data));

	uv_stop(node_impl->thread_loop);

	uv_walk(node_impl->thread_loop, node_loader_impl_walk, NULL);

	while (uv_run(node_impl->thread_loop, UV_RUN_DEFAULT) != 0);

	if (uv_loop_alive(node_impl->thread_loop) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "NodeJS event loop should not be alive");
	}

	if (uv_loop_close(node_impl->thread_loop) == UV_EBUSY)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "NodeJS event loop should not be busy");
	}
}

int node_loader_impl_destroy(loader_impl impl)
{
	loader_impl_node node_impl = static_cast<loader_impl_node>(loader_impl_get(impl));

	napi_status status;

	uint32_t ref_count = 0;

	if (node_impl == NULL)
	{
		return 1;
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

	/* Destroy syncronization objects */
	uv_mutex_destroy(&node_impl->mutex_initialize);

	uv_cond_destroy(&node_impl->cond_initialize);

	uv_mutex_destroy(&node_impl->mutex_load_from_file);

	uv_cond_destroy(&node_impl->cond_load_from_file);

	uv_mutex_destroy(&node_impl->mutex_clear);

	uv_cond_destroy(&node_impl->cond_clear);

	uv_mutex_destroy(&node_impl->mutex_discover);

	uv_cond_destroy(&node_impl->cond_discover);

	node_impl->async_destroy.data = static_cast<void *>(&node_impl);

	uv_async_send(&node_impl->async_destroy);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_destroy), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_call), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_discover), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_clear), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_load_from_file), NULL);

	uv_close(reinterpret_cast<uv_handle_t *>(&node_impl->async_initialize), NULL);

	uv_thread_join(&node_impl->thread_id);

	free(node_impl);

	return 0;
}
