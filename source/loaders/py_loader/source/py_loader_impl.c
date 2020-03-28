/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <py_loader/py_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>

#include <log/log.h>

#include <stdlib.h>

#include <Python.h>

#define PY_LOADER_IMPL_FUNCTION_TYPE_INVOKE_MOD "____metacall_py_loader_impl_function_type_invoke_module____"
#define PY_LOADER_IMPL_FUNCTION_TYPE_INVOKE_FUNC "py_loader_impl_function_type_invoke"

typedef struct loader_impl_py_function_type
{
	PyObject * func;
	PyObject ** values;
	loader_impl impl;

} * loader_impl_py_function;

typedef struct loader_impl_py_handle_module_type
{
	PyObject * instance;
	PyObject * name;

} * loader_impl_py_handle_module;

typedef struct loader_impl_py_handle_type
{
	loader_impl_py_handle_module modules;
	size_t size;

} * loader_impl_py_handle;

typedef struct loader_impl_py_type
{
	PyObject * inspect_module;
	PyObject * inspect_signature;
	PyObject * builtins_module;
	PyObject * traceback_module;
	PyObject * traceback_format_exception;

	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		PyObject * gc_module;
		PyObject * gc_set_debug;
		PyObject * gc_debug_leak;
		PyObject * gc_debug_stats;
	#endif

	PyObject * function_type_invoke_mod;
	PyObject * function_type_invoke_func;

} * loader_impl_py;

typedef struct loader_impl_py_function_type_invoke_state_type
{
	loader_impl impl;
	loader_impl_py py_impl;
	function callback;

} * loader_impl_py_function_type_invoke_state;

static void py_loader_impl_error_print(loader_impl_py py_impl);

#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	static void py_loader_impl_gc_print(loader_impl_py py_impl);
#endif

static void py_loader_impl_sys_path_print(PyObject * sys_path_list);

static PyObject * py_loader_impl_function_type_invoke(PyObject * self, PyObject * args);

static function_interface function_py_singleton(void);

static int py_loader_impl_discover_func_args_count(PyObject * func);

static int py_loader_impl_discover_func(loader_impl impl, PyObject * func, function f);

static void py_loader_impl_value_owner_finalize(value v, void * owner);

static PyMethodDef py_loader_impl_function_type_invoke_defs[] =
{
	{
		PY_LOADER_IMPL_FUNCTION_TYPE_INVOKE_FUNC,
		py_loader_impl_function_type_invoke,
		METH_VARARGS,
		PyDoc_STR("Implements a trampoline for functions as values in the type system.")
	},
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef py_loader_impl_function_type_invoke_module =
{
	PyModuleDef_HEAD_INIT,
	PY_LOADER_IMPL_FUNCTION_TYPE_INVOKE_MOD,
	PyDoc_STR("Module for providing support for functions as values in the MetaCall type system."),
	sizeof(struct loader_impl_py_function_type_invoke_state_type),
	py_loader_impl_function_type_invoke_defs,
	NULL,
	NULL,
	NULL,
	NULL
};

static void * py_loader_impl_value_ownership = NULL;

void py_loader_impl_value_owner_finalize(value v, void * owner)
{
	type_id id = value_type_id(v);

	if (owner == &py_loader_impl_value_ownership)
	{
		if (id == TYPE_PTR)
		{
			Py_XDECREF(value_to_ptr(v));
		}
	}
}

int type_py_interface_create(type t, type_impl impl)
{
	(void)t;
	(void)impl;

	return 0;
}

void type_py_interface_destroy(type t, type_impl impl)
{
	PyObject * builtin = (PyObject *)impl;

	(void)t;

	Py_DECREF(builtin);
}

type_interface type_py_singleton(void)
{
	static struct type_interface_type py_type_interface =
	{
		&type_py_interface_create,
		&type_py_interface_destroy
	};

	return &py_type_interface;
}

int function_py_interface_create(function func, function_impl impl)
{
	loader_impl_py_function py_func = (loader_impl_py_function)impl;

	signature s = function_signature(func);

	const size_t args_size = signature_count(s);

	if (args_size > 0)
	{
		py_func->values = malloc(sizeof(PyObject *) * args_size);

		if (py_func->values != NULL)
		{
			size_t iterator;

			for (iterator = 0; iterator < args_size; ++iterator)
			{
				py_func->values[iterator] = NULL;
			}

			return 0;
		}

		return 1;
	}

	py_func->values = NULL;

	return 0;
}

type_id py_loader_impl_capi_to_value_type(PyObject * obj)
{
	if (PyBool_Check(obj))
	{
		return TYPE_BOOL;
	}
	#if PY_MAJOR_VERSION == 2
		else if (PyInt_Check(obj))
		{
			return TYPE_INT;
		}
	#elif PY_MAJOR_VERSION == 3
		else if (PyLong_Check(obj))
		{
			return TYPE_LONG;
		}
	#endif
	else if (PyFloat_Check(obj))
	{
		return TYPE_DOUBLE;
	}
	#if PY_MAJOR_VERSION == 2
		else if (PyString_Check(obj))
		{
			return TYPE_STRING;
		}
	#elif PY_MAJOR_VERSION == 3
		else if (PyUnicode_Check(obj))
		{
			return TYPE_STRING;
		}
	#endif
	else if (PyBytes_Check(obj))
	{
		return TYPE_BUFFER;
	}
	else if (PyList_Check(obj))
	{
		return TYPE_ARRAY;
	}
	else if (PyDict_Check(obj))
	{
		return TYPE_MAP;
	}
	else if (PyCapsule_CheckExact(obj))
	{
		return TYPE_PTR;
	}
	else if (PyFunction_Check(obj) || PyCFunction_Check(obj))
	{
		return TYPE_FUNCTION;
	}
	else if (obj == Py_None)
	{
		return TYPE_NULL;
	}

	return TYPE_INVALID;
}

value py_loader_impl_capi_to_value(loader_impl impl, PyObject * obj, type_id id)
{
	value v = NULL;

	if (id == TYPE_BOOL)
	{
		boolean b = (PyObject_IsTrue(obj) == 1) ? 1 : 0;

		v = value_create_bool(b);
	}
	else if (id == TYPE_INT)
	{
		#if PY_MAJOR_VERSION == 2
			long l = PyInt_AsLong(obj);
		#elif PY_MAJOR_VERSION == 3
			long l = PyLong_AsLong(obj);
		#endif

		/* TODO: Review overflow */
		int i = (int)l;

		v = value_create_int(i);
	}
	else if (id == TYPE_LONG)
	{
		long l = PyLong_AsLong(obj);

		v = value_create_long(l);
	}
	else if (id == TYPE_FLOAT)
	{
		double d = PyFloat_AsDouble(obj);

		v = value_create_float((float)d);
	}
	else if (id == TYPE_DOUBLE)
	{
		double d = PyFloat_AsDouble(obj);

		v = value_create_double(d);
	}
	else if (id == TYPE_STRING)
	{
		char * str = NULL;

		Py_ssize_t length = 0;

		#if PY_MAJOR_VERSION == 2
			if (PyString_AsStringAndSize(obj, &str, &length) == -1)
			{
				if (PyErr_Occurred() != NULL)
				{
					loader_impl_py py_impl = loader_impl_get(impl);

					py_loader_impl_error_print(py_impl);
				}
			}
		#elif PY_MAJOR_VERSION == 3
			str = PyUnicode_AsUTF8AndSize(obj, &length);
		#endif

		v = value_create_string(str, (size_t)length);
	}
	else if (id == TYPE_BUFFER)
	{
		char * str = NULL;

		Py_ssize_t length = 0;

		#if PY_MAJOR_VERSION == 2

			/* TODO */

		#elif PY_MAJOR_VERSION == 3
			if (PyBytes_AsStringAndSize(obj, &str, &length) != -1)
			{
				v = value_create_buffer((const void *)str, (size_t)length + 1);
			}
		#endif
	}
	else if (id == TYPE_ARRAY)
	{
		Py_ssize_t iterator, length = 0;
		value * array_value;

		length = PyList_Size(obj);

		v = value_create_array(NULL, (size_t)length);

		array_value = value_to_array(v);

		for (iterator = 0; iterator < length; ++iterator)
		{
			PyObject * element = PyList_GetItem(obj, iterator);

			/* TODO: Review recursion overflow */
			array_value[iterator] = py_loader_impl_capi_to_value(impl, element, py_loader_impl_capi_to_value_type(element));
		}
	}
	else if (id == TYPE_MAP)
	{
		Py_ssize_t key_iterator, iterator, keys_size, length = 0;
		value * map_value;
		PyObject * keys;

		keys = PyDict_Keys(obj);
		keys_size = PyList_Size(keys);

		/* TODO: Allow different key types in the future */
		for (iterator = 0; iterator < keys_size; ++iterator)
		{
			PyObject * key = PyList_GetItem(keys, iterator);

			#if PY_MAJOR_VERSION == 2
				if (PyString_Check(key))
				{
					++length;
				}
			#elif PY_MAJOR_VERSION == 3
				if (PyUnicode_Check(key))
				{
					++length;
				}
			#endif
		}

		v = value_create_map(NULL, (size_t)length);

		map_value = value_to_map(v);

		for (iterator = 0, key_iterator = 0; iterator < keys_size; ++iterator)
		{
			char * key_str = NULL;

			Py_ssize_t key_length = 0;

			PyObject * element, * key;

			value * array_value;

			key = PyList_GetItem(keys, iterator);

			#if PY_MAJOR_VERSION == 2
				if (PyString_Check(key))
				{
					if (PyString_AsStringAndSize(key, &key_str, &key_length) == -1)
					{
						if (PyErr_Occurred() != NULL)
						{
							loader_impl_py py_impl = loader_impl_get(impl);

							py_loader_impl_error_print(py_impl);
						}
					}
				}
			#elif PY_MAJOR_VERSION == 3
				if (PyUnicode_Check(key))
				{
					key_str = PyUnicode_AsUTF8AndSize(key, &key_length);
				}
			#endif

			/* Allow only string keys by the moment */
			if (key_str != NULL)
			{
				element = PyDict_GetItem(obj, key);

				map_value[key_iterator] = value_create_array(NULL, 2);

				array_value = value_to_array(map_value[key_iterator]);

				array_value[0] = value_create_string(key_str, (size_t)key_length);

				/* TODO: Review recursion overflow */
				array_value[1] = py_loader_impl_capi_to_value(impl, element, py_loader_impl_capi_to_value_type(element));

				++key_iterator;
			}
		}
	}
	else if (id == TYPE_PTR)
	{
		void * ptr = NULL;

		#if PY_MAJOR_VERSION == 2

			/* TODO */

		#elif PY_MAJOR_VERSION == 3
			ptr = PyCapsule_GetPointer(obj, NULL);

			v = value_create_ptr(ptr);
		#endif
	}
	else if (id == TYPE_FUNCTION)
	{
		int discover_args_count;

		/* Check if we are passing our own hook to the callback */
		if (PyCFunction_Check(obj) && PyCFunction_GET_FUNCTION(obj) == py_loader_impl_function_type_invoke)
		{
			loader_impl_py py_impl = loader_impl_get(impl);

			loader_impl_py_function_type_invoke_state invoke_state;

			invoke_state = (loader_impl_py_function_type_invoke_state)PyModule_GetState(py_impl->function_type_invoke_mod);

			return value_create_function(invoke_state->callback);
		}

		discover_args_count = py_loader_impl_discover_func_args_count(obj);

		if (discover_args_count >= 0)
		{
			size_t args_count = (size_t)discover_args_count;

			loader_impl_py_function py_func = malloc(sizeof(struct loader_impl_py_function_type));

			function f = NULL;

			if (py_func == NULL)
			{
				return NULL;
			}

			/* TODO: Why two refs? Understand what is happening */
			Py_INCREF(obj);

			Py_INCREF(obj);

			py_func->func = obj;

			py_func->impl = impl;

			f = function_create(NULL, args_count, py_func, &function_py_singleton);

			if (py_loader_impl_discover_func(impl, obj, f) != 0)
			{
				function_destroy(f);

				return NULL;
			}

			return value_create_function(f);
		}
	}
	else if (id == TYPE_NULL)
	{
		v = value_create_null();
	}
	else
	{
		/* Return the value as opaque pointer */
		v = value_create_ptr(obj);

		/* Set up the ownership to python loader */
		value_own(v, &py_loader_impl_value_ownership);

		/* Create reference to the value so it does not get garbage collected */
		Py_INCREF(obj);

		/* Set up finalizer in order to free the value */
		value_finalizer(v, &py_loader_impl_value_owner_finalize);

		log_write("metacall", LOG_LEVEL_WARNING, "Unrecognized python type");
	}

	return v;
}

PyObject * py_loader_impl_value_to_capi(loader_impl impl, loader_impl_py py_impl, type_id id, value v)
{
	if (id == TYPE_BOOL)
	{
		boolean b = value_to_bool(v);

		long l = (b == 0) ? 0L : 1L;

		return PyBool_FromLong(l);
	}
	else if (id == TYPE_INT)
	{
		int i = value_to_int(v);

		#if PY_MAJOR_VERSION == 2
			return PyInt_FromLong(i);
		#elif PY_MAJOR_VERSION == 3
			long l = (long)i;

			return PyLong_FromLong(l);
		#endif
	}
	else if (id == TYPE_LONG)
	{
		long l = value_to_long(v);

		return PyLong_FromLong(l);
	}
	else if (id == TYPE_FLOAT)
	{
		float f = value_to_float(v);

		return PyFloat_FromDouble((double)f);
	}
	else if (id == TYPE_DOUBLE)
	{
		double d = value_to_double(v);

		return PyFloat_FromDouble(d);
	}
	else if (id == TYPE_STRING)
	{
		const char * str = value_to_string(v);

		#if PY_MAJOR_VERSION == 2
			return PyString_FromString(str);
		#elif PY_MAJOR_VERSION == 3
			return PyUnicode_FromString(str);
		#endif
	}
	else if (id == TYPE_BUFFER)
	{
		/* This forces that you wont never be able to pass a buffer as a pointer to metacall without be wrapped into a value type */
		/* If a pointer is passed this will produce a garbage read from outside of the memory range of the parameter */
		size_t size = value_type_size(v);

		const char * buffer = value_to_buffer(v);

		#if PY_MAJOR_VERSION == 2

			/* TODO */

		#elif PY_MAJOR_VERSION == 3
			return PyBytes_FromStringAndSize(buffer, (Py_ssize_t)size);
		#endif
	}
	else if (id == TYPE_ARRAY)
	{
		value * array_value = value_to_array(v);

		Py_ssize_t iterator, array_size = (Py_ssize_t)value_type_count(v);

		PyObject * list = PyList_New(array_size);

		for (iterator = 0; iterator < array_size; ++iterator)
		{
			PyObject * item = py_loader_impl_value_to_capi(impl, py_impl, value_type_id((value)array_value[iterator]), (value)array_value[iterator]);

			if (PyList_SetItem(list, iterator, item) != 0)
			{
				/* TODO: Report error */
			}
		}

		return list;
	}
	else if (id == TYPE_MAP)
	{
		/* TODO */
	}
	else if (id == TYPE_PTR)
	{
		void * ptr = value_to_ptr(v);

		if (value_owner(v) == &py_loader_impl_value_ownership)
		{
			return ptr;
		}
		else
		{
			#if PY_MAJOR_VERSION == 2

				/* TODO */

			#elif PY_MAJOR_VERSION == 3
				return PyCapsule_New(ptr, NULL, NULL);
			#endif
		}

	}
	else if (id == TYPE_FUNCTION)
	{
		loader_impl_py_function_type_invoke_state invoke_state;

		invoke_state = (loader_impl_py_function_type_invoke_state)PyModule_GetState(py_impl->function_type_invoke_mod);

		invoke_state->callback = value_to_function(v);

		return py_impl->function_type_invoke_func;
	}
	else if (id == TYPE_NULL)
	{
		return Py_None;
	}
	else
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Unrecognized value type");
	}

	return NULL;
}

function_return function_py_interface_invoke(function func, function_impl impl, function_args args)
{
	loader_impl_py_function py_func = (loader_impl_py_function)impl;

	signature s = function_signature(func);

	const size_t args_size = signature_count(s);

	type ret_type = signature_get_return(s);

	PyObject * result = NULL;

	size_t args_count;

	loader_impl_py py_impl = loader_impl_get(py_func->impl);

	PyGILState_STATE gstate = PyGILState_Ensure();

	PyObject * tuple_args;

	/* Possibly a recursive call */
	if (Py_EnterRecursiveCall(" while executing a function in Python Loader") != 0)
	{
		PyGILState_Release(gstate);

		return NULL;
	}

	tuple_args = PyTuple_New(args_size);

	for (args_count = 0; args_count < args_size; ++args_count)
	{
		type t = signature_get_type(s, args_count);

		type_id id = TYPE_INVALID;

		if (t == NULL)
		{
			id = value_type_id((value)args[args_count]);

			log_write("metacall", LOG_LEVEL_DEBUG, "Argument #%u Type: %s", args_count, type_id_name(id));
		}
		else
		{
			id = type_index(t);

			log_write("metacall", LOG_LEVEL_DEBUG, "Argument #%u Type (%p): %s", args_count, (void *)t, type_name(t));
		}

		py_func->values[args_count] = py_loader_impl_value_to_capi(impl, py_impl, id, args[args_count]);

		if (py_func->values[args_count] != NULL)
		{
			PyTuple_SetItem(tuple_args, args_count, py_func->values[args_count]);
		}
	}

	result = PyObject_CallObject(py_func->func, tuple_args);

	/* End of recursive call */
	Py_LeaveRecursiveCall();

	if (PyErr_Occurred() != NULL)
	{
		py_loader_impl_error_print(py_impl);
	}

	Py_DECREF(tuple_args);

	if (result != NULL)
	{
		value v = NULL;

		type_id id = TYPE_INVALID;

		if (ret_type == NULL)
		{
			id = py_loader_impl_capi_to_value_type(result);

			log_write("metacall", LOG_LEVEL_DEBUG, "Return Type: %s", type_id_name(id));
		}
		else
		{
			id = type_index(ret_type);

			log_write("metacall", LOG_LEVEL_DEBUG, "Return Type (%p): %s", (void *)ret_type, type_name(ret_type));
		}


		v = py_loader_impl_capi_to_value(py_func->impl, result, id);

		Py_DECREF(result);

		PyGILState_Release(gstate);

		return v;
	}

	PyGILState_Release(gstate);

	return NULL;
}

function_return function_py_interface_await(function func, function_impl impl, function_args args, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void * context)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)resolve_callback;
	(void)reject_callback;
	(void)context;

	return NULL;
}

void function_py_interface_destroy(function func, function_impl impl)
{
	loader_impl_py_function py_func = (loader_impl_py_function)impl;

	if (py_func != NULL)
	{
		if (py_func->values != NULL)
		{
			/* TODO: Check why Py_DECREF of each value segfaults */
			(void)func;

			/*
			PyGILState_STATE gstate;

			signature s = function_signature(func);

			const size_t args_size = signature_count(s);

			size_t iterator;

			gstate = PyGILState_Ensure();

			for (iterator = 0; iterator < args_size; ++iterator)
			{
				if (py_func->values[iterator] != NULL)
				{
					Py_DECREF(py_func->values[iterator]);
				}
			}

			PyGILState_Release(gstate);
			*/

			free(py_func->values);
		}

		Py_DECREF(py_func->func);

		free(py_func);
	}
}

function_interface function_py_singleton()
{
	static struct function_interface_type py_function_interface =
	{
		&function_py_interface_create,
		&function_py_interface_invoke,
		&function_py_interface_await,
		&function_py_interface_destroy
	};

	return &py_function_interface;
}

PyObject * py_loader_impl_function_type_invoke(PyObject * self, PyObject * args)
{
	static void * null_args[1] = { NULL };

	loader_impl_py_function_type_invoke_state invoke_state = (loader_impl_py_function_type_invoke_state)PyModule_GetState(self);

	signature s;

	size_t args_size, args_count, min_args_size;

	Py_ssize_t callee_args_size;

	void ** value_args;

	value ret;

	if (invoke_state == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Fatal error when invoking a function, state cannot be recovered, avoiding the function call");

		return Py_None;
	}

	s = function_signature(invoke_state->callback);

	args_size = signature_count(s);

	callee_args_size = PyTuple_Size(args);

	min_args_size = args_size < (size_t)callee_args_size ? args_size : (size_t)callee_args_size;

	if (args_size != (size_t)callee_args_size)
	{
		log_write("metacall", LOG_LEVEL_WARNING, "Callback being executed without different number of arguments %u (signature) != %u (call)", args_size, callee_args_size);
	}

	value_args = min_args_size == 0 ? null_args : malloc(sizeof(void *) * min_args_size);

	if (value_args == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid allocation of arguments for callback");

		return Py_None;
	}

	/* Generate metacall values from python values */
	for (args_count = 0; args_count < min_args_size; ++args_count)
	{
		PyObject * arg = PyTuple_GetItem(args, (Py_ssize_t)args_count);

		type_id id = py_loader_impl_capi_to_value_type(arg);

		value_args[args_count] = py_loader_impl_capi_to_value(invoke_state->impl, arg, id);
	}

	/* Execute the callback */
	ret = (value)function_call(invoke_state->callback, value_args);

	/* Destroy argument values */
	for (args_count = 0; args_count < min_args_size; ++args_count)
	{
		value_type_destroy(value_args[args_count]);
	}

	if (value_args != null_args)
	{
		free(value_args);
	}

	/* Transform the return value into a python value */
	if (ret != NULL)
	{
		PyObject * py_ret = py_loader_impl_value_to_capi(invoke_state->impl, invoke_state->py_impl, value_type_id(ret), ret);

		value_type_destroy(ret);

		return py_ret;
	}

	return Py_None;
}

PyObject * py_loader_impl_get_builtin(loader_impl_py py_impl, const char * builtin_name)
{
	PyObject * builtin = PyObject_GetAttrString(py_impl->builtins_module, builtin_name);

	Py_XINCREF(builtin);

	if (builtin != NULL && PyType_Check(builtin))
	{
		return builtin;
	}

	Py_XDECREF(builtin);

	return NULL;
}

int py_loader_impl_get_builtin_type(loader_impl impl, loader_impl_py py_impl, type_id id, const char * name)
{
	PyObject * builtin = py_loader_impl_get_builtin(py_impl, name);

	if (builtin != NULL)
	{
		type builtin_type = type_create(id, name, builtin, &type_py_singleton);

		if (builtin_type != NULL)
		{
			if (loader_impl_type_define(impl, type_name(builtin_type), builtin_type) == 0)
			{
				return 0;
			}

			type_destroy(builtin_type);
		}

		Py_DECREF(builtin);
	}

	return 1;
}

int py_loader_impl_initialize_inspect_types(loader_impl impl, loader_impl_py py_impl)
{
	PyObject * module_name = PyUnicode_DecodeFSDefault("builtins");

	py_impl->builtins_module = PyImport_Import(module_name);

	if (PyErr_Occurred() != NULL)
	{
		py_loader_impl_error_print(py_impl);

		Py_DECREF(module_name);

		return 1;
	}

	Py_DECREF(module_name);

	if (py_impl->builtins_module != NULL)
	{
		/* TODO: move this to loader_impl */

		static struct
		{
			type_id id;
			const char * name;
		}
		type_id_name_pair[] =
		{
			{ TYPE_BOOL, "bool" },
			{ TYPE_LONG, "int" },

			#if PY_MAJOR_VERSION == 2
				{ TYPE_LONG, "long" },
			#endif

			{ TYPE_DOUBLE, "float" },

			{ TYPE_STRING, "str" },
			{ TYPE_BUFFER, "bytes" },
			{ TYPE_ARRAY, "list" },
			{ TYPE_MAP, "dict" }
		};

		size_t index, size = sizeof(type_id_name_pair) / sizeof(type_id_name_pair[0]);

		for (index = 0; index < size; ++index)
		{
			if (py_loader_impl_get_builtin_type(impl, py_impl,
				type_id_name_pair[index].id,
				type_id_name_pair[index].name) != 0)
			{
				if (PyErr_Occurred() != NULL)
				{
					py_loader_impl_error_print(py_impl);
				}

				Py_DECREF(py_impl->builtins_module);

				return 1;
			}
		}

		return 0;
	}

	return 1;
}

int py_loader_impl_initialize_inspect(loader_impl impl, loader_impl_py py_impl)
{
	PyObject * module_name = PyUnicode_DecodeFSDefault("inspect");

	py_impl->inspect_module = PyImport_Import(module_name);

	if (PyErr_Occurred() != NULL)
	{
		py_loader_impl_error_print(py_impl);

		Py_DECREF(module_name);

		return 1;
	}

	Py_DECREF(module_name);

	if (py_impl->inspect_module != NULL)
	{
		py_impl->inspect_signature = PyObject_GetAttrString(py_impl->inspect_module, "signature");

		if (py_impl->inspect_signature != NULL)
		{
			if (PyCallable_Check(py_impl->inspect_signature))
			{
				if (py_loader_impl_initialize_inspect_types(impl, py_impl) == 0)
				{
					return 0;
				}
			}

			Py_XDECREF(py_impl->inspect_signature);
		}

		Py_DECREF(py_impl->inspect_module);
	}

	return 1;
}

int py_loader_impl_initialize_traceback(loader_impl impl, loader_impl_py py_impl)
{
	PyObject * module_name = PyUnicode_DecodeFSDefault("traceback");

	(void)impl;

	py_impl->traceback_module = PyImport_Import(module_name);

	if (PyErr_Occurred() != NULL)
	{
		py_loader_impl_error_print(py_impl);

		Py_DECREF(module_name);

		return 1;
	}

	Py_DECREF(module_name);

	if (py_impl->traceback_module != NULL)
	{
		py_impl->traceback_format_exception = PyObject_GetAttrString(py_impl->traceback_module, "format_exception");

		if (py_impl->traceback_format_exception != NULL)
		{
			if (PyCallable_Check(py_impl->traceback_format_exception))
			{
				return 0;
			}

			Py_XDECREF(py_impl->traceback_format_exception);
		}

		Py_DECREF(py_impl->traceback_module);
	}

	return 1;
}

int py_loader_impl_initialize_gc(loader_impl_py py_impl)
{
	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	{
		PyObject * module_name = PyUnicode_DecodeFSDefault("gc");

		py_impl->gc_module = PyImport_Import(module_name);

		if (PyErr_Occurred() != NULL)
		{
			py_loader_impl_error_print(py_impl);

			Py_DECREF(module_name);

			return 1;
		}

		Py_DECREF(module_name);

		if (py_impl->gc_module != NULL)
		{
			py_impl->gc_set_debug = PyObject_GetAttrString(py_impl->gc_module, "set_debug");

			if (py_impl->gc_set_debug != NULL)
			{
				if (PyCallable_Check(py_impl->gc_set_debug))
				{
					py_impl->gc_debug_leak = PyDict_GetItemString(PyModule_GetDict(py_impl->gc_module), "DEBUG_LEAK");
					py_impl->gc_debug_stats = PyDict_GetItemString(PyModule_GetDict(py_impl->gc_module), "DEBUG_STATS");

					if (py_impl->gc_debug_leak != NULL && py_impl->gc_debug_stats != NULL)
					{
						Py_INCREF(py_impl->gc_debug_leak);
						Py_INCREF(py_impl->gc_debug_stats);

						return 0;
					}
				}

				Py_XDECREF(py_impl->gc_set_debug);
			}

			Py_DECREF(py_impl->gc_module);
		}

		return 1;
	}
	#else
	{
		(void)py_impl;

		return 1;
	}
	#endif
}

loader_impl_data py_loader_impl_initialize(loader_impl impl, configuration config, loader_host host)
{
	loader_impl_py py_impl;
	loader_impl_py_function_type_invoke_state invoke_state;

	PyGILState_STATE gstate;

	(void)impl;
	(void)config;

	log_copy(host->log);

	py_impl = malloc(sizeof(struct loader_impl_py_type));

	if (py_impl == NULL)
	{
		return NULL;
	}

	Py_InitializeEx(0);

	if (Py_IsInitialized() == 0)
	{
		return NULL;
	}

	if (PyEval_ThreadsInitialized() == 0)
	{
		PyEval_InitThreads();
	}

	gstate = PyGILState_Ensure();

	if (py_loader_impl_initialize_traceback(impl, py_impl) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid traceback module creation");
	}

	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	{
		if (py_loader_impl_initialize_gc(py_impl) != 0)
		{
			PyObject_CallMethodObjArgs(py_impl->gc_module, py_impl->gc_set_debug, py_impl->gc_debug_leak /* py_impl->gc_debug_stats */);
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid garbage collector module creation");
		}
	}
	#endif

	if (py_loader_impl_initialize_inspect(impl, py_impl) != 0)
	{
		PyGILState_Release(gstate);

		free(py_impl);

		return NULL;
	}

	/* Create module for allowing callbacks */
	py_impl->function_type_invoke_mod = PyModule_Create(&py_loader_impl_function_type_invoke_module);
	py_impl->function_type_invoke_func = PyObject_GetAttrString(py_impl->function_type_invoke_mod, PY_LOADER_IMPL_FUNCTION_TYPE_INVOKE_FUNC);

	/* Store python loader into the module state */
	invoke_state = (loader_impl_py_function_type_invoke_state)PyModule_GetState(py_impl->function_type_invoke_mod);

	invoke_state->impl = impl;
	invoke_state->py_impl = py_impl;
	invoke_state->callback = NULL;

	PyGILState_Release(gstate);

	log_write("metacall", LOG_LEVEL_DEBUG, "Python loader initialized correctly");

	return py_impl;
}

int py_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	loader_impl_py py_impl = loader_impl_get(impl);

	if (py_impl != NULL)
	{
		PyGILState_STATE gstate;

		PyObject * system_path, * current_path;

		gstate = PyGILState_Ensure();

		system_path = PySys_GetObject("path");

		current_path = PyUnicode_DecodeFSDefault(path);

		PyList_Append(system_path, current_path);

		py_loader_impl_sys_path_print(system_path);

		Py_DECREF(current_path);

		PyGILState_Release(gstate);

		return 0;
	}

	return 1;
}

loader_impl_py_handle py_loader_impl_handle_create(size_t size)
{
	loader_impl_py_handle py_handle = malloc(sizeof(struct loader_impl_py_handle_type));

	if (py_handle == NULL)
	{
		return NULL;
	}

	py_handle->size = size;
	py_handle->modules = malloc(sizeof(struct loader_impl_py_handle_module_type) * size);

	if (py_handle->modules == NULL)
	{
		free(py_handle);

		return NULL;
	}

	return py_handle;
}

void py_loader_impl_handle_destroy(loader_impl_py_handle py_handle)
{
	size_t iterator;

	/* PyGILState_STATE gstate; */

	PyObject * system_modules;

	/* gstate = PyGILState_Ensure(); */

	system_modules = PySys_GetObject("modules");

	for (iterator = 0; iterator < py_handle->size; ++iterator)
	{
		/* TODO: Not sure why, but this generate a memory leak */
		/* In fact, from the interpreter it must be done, but from */
		/* C API it looks like is not needed */
		/*PyObject_Del(py_handle->modules[iterator].instance);*/

		PyObject_DelItem(system_modules, py_handle->modules[iterator].name);

		Py_XDECREF(py_handle->modules[iterator].instance);
		Py_XDECREF(py_handle->modules[iterator].name);
	}

	/* PyGILState_Release(gstate); */

	free(py_handle->modules);

	free(py_handle);
}

loader_handle py_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	PyGILState_STATE gstate;

	size_t iterator;

	loader_impl_py_handle py_handle = py_loader_impl_handle_create(size);

	if (py_handle == NULL)
	{
		return NULL;
	}

	gstate = PyGILState_Ensure();

	for (iterator = 0; iterator < size; ++iterator)
	{
		loader_naming_name module_name;

		loader_path_get_name(paths[iterator], module_name);

		py_handle->modules[iterator].name = PyUnicode_DecodeFSDefault(module_name);

		/* TODO: Implement a map in the core to hold all directories by each loader impl */

		/* if (loader_impl_has_execution_path(location_path) != 0) */
		{
			loader_naming_path location_path;

			loader_path_get_path(paths[iterator], strlen(paths[iterator]) + 1, location_path);

			if (py_loader_impl_execution_path(impl, location_path) != 0)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Python loader invalid execution path: %s", location_path);

				PyGILState_Release(gstate);

				py_loader_impl_handle_destroy(py_handle);

				return NULL;
			}
		}

		py_handle->modules[iterator].instance = PyImport_Import(py_handle->modules[iterator].name);

		if (PyErr_Occurred() != NULL || py_handle->modules[iterator].instance == NULL)
		{
			loader_impl_py py_impl = loader_impl_get(impl);

			py_loader_impl_error_print(py_impl);

			PyGILState_Release(gstate);

			py_loader_impl_handle_destroy(py_handle);

			return NULL;
		}
	}

	PyGILState_Release(gstate);

	return (loader_handle)py_handle;
}

loader_handle py_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char * buffer, size_t size)
{
	PyGILState_STATE gstate;

	PyObject * compiled;

	loader_impl_py_handle py_handle = py_loader_impl_handle_create(1);

	if (py_handle == NULL)
	{
		return NULL;
	}

	gstate = PyGILState_Ensure();

	compiled = Py_CompileString(buffer, name, Py_file_input);

	(void)size;

	if (compiled != NULL)
	{
		py_handle->modules[0].instance = PyImport_ExecCodeModule(name, compiled);

		if (PyErr_Occurred() != NULL)
		{
			loader_impl_py py_impl = loader_impl_get(impl);

			py_loader_impl_error_print(py_impl);

			PyGILState_Release(gstate);

			py_loader_impl_handle_destroy(py_handle);

			return NULL;
		}

		py_handle->modules[0].name = PyUnicode_DecodeFSDefault(name);

		PyGILState_Release(gstate);

		log_write("metacall", LOG_LEVEL_DEBUG, "Python loader (%p) importing %s. from memory module at (%p)", (void *)impl, name, (void *)py_handle->modules[0].instance);

		return (loader_handle)py_handle;
	}

	if (PyErr_Occurred() != NULL)
	{
		loader_impl_py py_impl = loader_impl_get(impl);

		py_loader_impl_error_print(py_impl);
	}

	PyGILState_Release(gstate);

	py_loader_impl_handle_destroy(py_handle);

	return NULL;
}

loader_handle py_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	/* TODO */

	(void)impl;
	(void)path;

	return NULL;
}

int py_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	(void)impl;

	if (handle != NULL)
	{
		loader_impl_py_handle py_handle = (loader_impl_py_handle)handle;

		py_loader_impl_handle_destroy(py_handle);

		return 0;
	}

	return 1;
}

type py_loader_impl_discover_type(loader_impl impl, PyObject * annotation)
{
	type t = NULL;

	if (annotation != NULL)
	{
		PyObject * annotation_qualname = PyObject_GetAttrString(annotation, "__qualname__");

		const char * annotation_name = PyUnicode_AsUTF8(annotation_qualname);

		if (strcmp(annotation_name, "_empty") != 0)
		{
			t = loader_impl_type(impl, annotation_name);

			log_write("metacall", LOG_LEVEL_DEBUG, "Discover type (%p) (%p): %s", (void *)annotation, (void *)type_derived(t), annotation_name);

			Py_DECREF(annotation_qualname);
		}
	}

	return t;
}

int py_loader_impl_discover_func_args_count(PyObject * func)
{
	int args_count = -1;

	if (PyObject_HasAttrString(func, "__call__"))
	{
		PyObject * func_code = NULL;

		if (PyObject_HasAttrString(func, "__code__"))
		{
			func_code = PyObject_GetAttrString(func, "__code__");
		}
		else
		{
			PyObject * func_call = PyObject_GetAttrString(func, "__call__");

			if (func_call != NULL && PyObject_HasAttrString(func_call, "__code__"))
			{
				func_code = PyObject_GetAttrString(func_call, "__code__");
			}

			Py_XDECREF(func_call);
		}

		if (func_code != NULL)
		{
			PyObject * func_code_args_count = PyObject_GetAttrString(func_code, "co_argcount");

			if (func_code_args_count != NULL)
			{
				args_count = PyLong_AsLong(func_code_args_count);

				Py_DECREF(func_code_args_count);
			}

			Py_DECREF(func_code);
		}
		else if (PyCFunction_Check(func))
		{
			int flags = PyCFunction_GetFlags(func);

			if (flags & METH_NOARGS)
			{
				args_count = 0;
			}
			else if (flags & METH_VARARGS)
			{
				/* TODO: Varidic arguments are not supported */
				log_write("metacall", LOG_LEVEL_ERROR, "Builtins (C Python Functions) with varidic arguments are not supported");
				args_count = -1;
			}
		}
	}

	return args_count;
}

int py_loader_impl_discover_func(loader_impl impl, PyObject * func, function f)
{
	loader_impl_py py_impl = loader_impl_get(impl);

	PyObject * args = PyTuple_New(1);
	PyObject * result = NULL;

	if (args != NULL)
	{
		PyTuple_SetItem(args, 0, func);

		if (PyErr_Occurred() != NULL)
		{
			py_loader_impl_error_print(py_impl);

			return 1;
		}

		result = PyObject_CallObject(py_impl->inspect_signature, args);
	}

	if (result != NULL)
	{
		signature s = function_signature(f);

		PyObject * parameters = PyObject_GetAttrString(result, "parameters");

		PyObject * return_annotation = PyObject_GetAttrString(result, "return_annotation");

		if (parameters != NULL && PyMapping_Check(parameters))
		{
			PyObject * parameter_list = PyMapping_Values(parameters);

			if (parameter_list != NULL && PyList_Check(parameter_list))
			{
				Py_ssize_t iterator;

				Py_ssize_t parameter_list_size = PyMapping_Size(parameters);

				if ((size_t)parameter_list_size != signature_count(s))
				{
					if (PyErr_Occurred() != NULL)
					{
						py_loader_impl_error_print(py_impl);
					}

					return 1;
				}

				for (iterator = 0; iterator < parameter_list_size; ++iterator)
				{
					PyObject * parameter = PyList_GetItem(parameter_list, iterator);

					if (parameter != NULL)
					{
						PyObject * name = PyObject_GetAttrString(parameter, "name");

						const char * parameter_name = PyUnicode_AsUTF8(name);

						PyObject * annotation = PyObject_GetAttrString(parameter, "annotation");

						type t = py_loader_impl_discover_type(impl, annotation);

						signature_set(s, iterator, parameter_name, t);
					}
				}
			}
		}

		signature_set_return(s, py_loader_impl_discover_type(impl, return_annotation));

		return 0;
	}
	else
	{
		/* TODO: Implement builtins with varidic arguments */
		if (PyCFunction_Check(func))
		{
			signature s = function_signature(f);

			signature_set_return(s, NULL);

			return 0;
		}

	}

	return 1;
}

int py_loader_impl_discover_module(loader_impl impl, PyObject * module, context ctx)
{
	PyGILState_STATE gstate;

	gstate = PyGILState_Ensure();

	if (module != NULL && PyModule_Check(module))
	{
		PyObject * module_dict = PyModule_GetDict(module);

		if (module_dict != NULL)
		{
			Py_ssize_t position = 0;

			PyObject * key, * value;

			while (PyDict_Next(module_dict, &position, &key, &value))
			{
				if (PyCallable_Check(value))
				{
					char * func_name = PyUnicode_AsUTF8(key);

					int discover_args_count = py_loader_impl_discover_func_args_count(value);

					if (discover_args_count >= 0)
					{
						size_t args_count = (size_t)discover_args_count;

						loader_impl_py_function py_func = malloc(sizeof(struct loader_impl_py_function_type));

						function f = NULL;

						if (py_func == NULL)
						{
							PyGILState_Release(gstate);

							return 1;
						}

						/* TODO: Why two refs? Understand what is happening */

						Py_INCREF(value);

						Py_INCREF(value);

						py_func->func = value;

						py_func->impl = impl;

						f = function_create(func_name, args_count, py_func, &function_py_singleton);

						log_write("metacall", LOG_LEVEL_DEBUG, "Introspection: function %s, args count %ld", func_name, args_count);

						if (py_loader_impl_discover_func(impl, value, f) == 0)
						{
							scope sp = context_scope(ctx);

							scope_define(sp, func_name, f);
						}
						else
						{
							function_destroy(f);
						}
					}
				}
			}
		}

		PyGILState_Release(gstate);

		return 0;
	}

	PyGILState_Release(gstate);

	return 1;
}

int py_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_py_handle py_handle = (loader_impl_py_handle)handle;

	size_t iterator;

	for (iterator = 0; iterator < py_handle->size; ++iterator)
	{
		if (py_loader_impl_discover_module(impl, py_handle->modules[iterator].instance, ctx) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Introspection module discovering error #%" PRIuS" <%p>", iterator, (void *)py_handle->modules[iterator].instance);

			return 1;
		}
	}

	return 0;
}

void py_loader_impl_error_print(loader_impl_py py_impl)
{
	static const char error_format_str[] = "Python Error [Type: %s]: %s\n{\n%s\n}";
	static const char separator_str[] = "\n";
	static const char traceback_not_found[] = "Traceback not available";

	PyObject * type, * value, * traceback;

	PyObject * type_str_obj, * value_str_obj, * traceback_str_obj;

	PyObject * traceback_list, * separator;

	char * type_str, * value_str, * traceback_str;

	PyErr_Fetch(&type, &value, &traceback);

	type_str_obj = PyObject_Str(type);

	value_str_obj = PyObject_Str(value);

	traceback_list = PyObject_CallFunctionObjArgs(py_impl->traceback_format_exception, type, value, traceback, NULL);

	#if PY_MAJOR_VERSION == 2
		separator = PyString_FromString(separator_str);

		traceback_str_obj = PyString_Join(separator, traceback_list);

		type_str = PyString_AsString(type_str_obj);
		value_str = PyString_AsString(value_str_obj);
		traceback_str = traceback_str_obj ? PyString_AsString(traceback_str_obj) : NULL;
	#elif PY_MAJOR_VERSION == 3
		separator = PyUnicode_FromString(separator_str);

		traceback_str_obj = PyUnicode_Join(separator, traceback_list);

		type_str = PyUnicode_AsUTF8(type_str_obj);
		value_str = PyUnicode_AsUTF8(value_str_obj);
		traceback_str = traceback_str_obj ? PyUnicode_AsUTF8(traceback_str_obj) : NULL;
	#endif

	log_write("metacall", LOG_LEVEL_ERROR, error_format_str, type_str, value_str, traceback_str ? traceback_str : traceback_not_found);

	Py_XDECREF(traceback_list);
	Py_DECREF(separator);
	Py_XDECREF(traceback_str_obj);

	PyErr_Restore(type, value, traceback);
}

#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	void py_loader_impl_gc_print(loader_impl_py py_impl)
	{
		static const char garbage_format_str[] = "Python Garbage Collector:\n%s";
		static const char separator_str[] = "\n";

		PyObject * garbage_list, * separator, * garbage_str_obj;

		garbage_list = PyObject_GetAttrString(py_impl->gc_module, "garbage");

		#if PY_MAJOR_VERSION == 2
			separator = PyString_FromString(separator_str);

			garbage_str_obj = PyString_Join(separator, garbage_list);

			log_write("metacall", LOG_LEVEL_DEBUG, garbage_format_str, PyString_AsString(garbage_str_obj));
		#elif PY_MAJOR_VERSION == 3
			separator = PyUnicode_FromString(separator_str);

			garbage_str_obj = PyUnicode_Join(separator, garbage_list);

			log_write("metacall", LOG_LEVEL_DEBUG, garbage_format_str, PyUnicode_AsUTF8(garbage_str_obj));
		#endif

		Py_DECREF(garbage_list);
		Py_DECREF(separator);
		Py_DECREF(garbage_str_obj);
	}
#endif

void py_loader_impl_sys_path_print(PyObject * sys_path_list)
{
	static const char sys_path_format_str[] = "Python System Paths:\n%s";
	static const char separator_str[] = "\n";

	PyObject * separator, * sys_path_str_obj;

	char * sys_path_str = NULL;

	#if PY_MAJOR_VERSION == 2
		separator = PyString_FromString(separator_str);

		sys_path_str_obj = PyString_Join(separator, sys_path_list);

		sys_path_str = PyString_AsString(sys_path_str_obj);
	#elif PY_MAJOR_VERSION == 3
		separator = PyUnicode_FromString(separator_str);

		sys_path_str_obj = PyUnicode_Join(separator, sys_path_list);

		sys_path_str = PyUnicode_AsUTF8(sys_path_str_obj);
	#endif

	log_write("metacall", LOG_LEVEL_DEBUG, sys_path_format_str, sys_path_str);

	Py_XDECREF(separator);
}

int py_loader_impl_destroy(loader_impl impl)
{
	loader_impl_py py_impl = loader_impl_get(impl);

	if (py_impl != NULL)
	{
		/* PyGILState_STATE gstate; */

		/* gstate = PyGILState_Ensure(); */

		Py_DECREF(py_impl->inspect_signature);

		Py_DECREF(py_impl->inspect_module);

		Py_DECREF(py_impl->builtins_module);

		Py_DECREF(py_impl->traceback_format_exception);

		Py_DECREF(py_impl->traceback_module);

		#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		{
			py_loader_impl_gc_print(py_impl);

			Py_DECREF(py_impl->gc_set_debug);

			Py_DECREF(py_impl->gc_debug_leak);

			Py_DECREF(py_impl->gc_debug_stats);

			Py_DECREF(py_impl->gc_module);
		}
		#endif

		Py_XDECREF(py_impl->function_type_invoke_mod);
		Py_XDECREF(py_impl->function_type_invoke_func);

		if (Py_IsInitialized() != 0)
		{
			if (PyErr_Occurred() != NULL)
			{
				py_loader_impl_error_print(py_impl);
			}

			/* PyGILState_Release(gstate); */

			#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 6
			{
				if (Py_FinalizeEx() != 0)
				{
					log_write("metacall", LOG_LEVEL_DEBUG, "Error when executing Py_FinalizeEx");
				}
			}
			#else
			{
				Py_Finalize();
			}
			#endif
		}
		else
		{
			/* PyGILState_Release(gstate); */
		}

		free(py_impl);

		return 0;
	}

	return 1;
}
