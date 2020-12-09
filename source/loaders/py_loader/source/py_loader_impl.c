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

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <reflect/reflect_type.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_context.h>
#include <reflect/reflect_class.h>

#include <log/log.h>

#include <stdlib.h>

#include <Python.h>

#define PY_LOADER_IMPL_FUNCTION_TYPE_INVOKE_FUNC "__py_loader_impl_function_type_invoke__"

typedef struct loader_impl_py_function_type
{
	PyObject *func;
	PyObject **values;
	loader_impl impl;

} * loader_impl_py_function;

typedef struct loader_impl_py_class_type
{
	PyObject *class;
	loader_impl impl;

} * loader_impl_py_class;

typedef struct loader_impl_py_object_type
{
	PyObject *object;
	PyTypeObject *object_class;
	loader_impl impl;

} * loader_impl_py_object;

typedef struct loader_impl_py_handle_module_type
{
	PyObject *instance;
	PyObject *name;

} * loader_impl_py_handle_module;

typedef struct loader_impl_py_handle_type
{
	loader_impl_py_handle_module modules;
	size_t size;

} * loader_impl_py_handle;

struct loader_impl_py_type
{
	PyObject *inspect_module;
	PyObject *inspect_signature;
	PyObject *builtins_module;
	PyObject *traceback_module;
	PyObject *traceback_format_exception;

#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	PyObject *gc_module;
	PyObject *gc_set_debug;
	PyObject *gc_debug_leak;
	PyObject *gc_debug_stats;
#endif
};

typedef struct loader_impl_py_function_type_invoke_state_type
{
	loader_impl impl;
	loader_impl_py py_impl;
	value callback;

} * loader_impl_py_function_type_invoke_state;

static void py_loader_impl_error_print(loader_impl_py py_impl);

#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
static void py_loader_impl_gc_print(loader_impl_py py_impl);
#endif

static void py_loader_impl_sys_path_print(PyObject *sys_path_list);

static PyObject *py_loader_impl_function_type_invoke(PyObject *self, PyObject *args);

static function_interface function_py_singleton(void);

static type_interface type_py_singleton(void);

static object_interface py_object_interface_singleton(void);

static class_interface py_class_interface_singleton(void);

static int py_loader_impl_discover_func_args_count(PyObject *func);

static int py_loader_impl_discover_func(loader_impl impl, PyObject *func, function f);

static int py_loader_impl_discover_class(loader_impl impl, PyObject *read_only_dict, klass c);

static void py_loader_impl_value_invoke_state_finalize(value v, void *data);

static void py_loader_impl_value_ptr_finalize(value v, void *data);

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

void py_loader_impl_value_invoke_state_finalize(value v, void *data)
{
	loader_impl_py_function_type_invoke_state invoke_state = (loader_impl_py_function_type_invoke_state)data;

	(void)v;

	free(invoke_state);
}

void py_loader_impl_value_ptr_finalize(value v, void *data)
{
	type_id id = value_type_id(v);

	if (id == TYPE_PTR)
	{
		if (data != NULL)
		{
			PyObject *obj = (PyObject *)data;
			Py_XDECREF(obj);
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
	PyObject *builtin = (PyObject *)impl;

	(void)t;

	Py_DECREF(builtin);
}

type_interface type_py_singleton(void)
{
	static struct type_interface_type py_type_interface =
		{
			&type_py_interface_create,
			&type_py_interface_destroy};

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


int py_object_interface_create(object obj, object_impl impl)
{
	(void)obj;

	loader_impl_py_object py_obj = impl;

	py_obj->object = NULL;
	py_obj->object_class = NULL;

	return 0;
}

/* TODO: get and set is actually the same as static_get and static_set but applied to an object instead of a class */
value py_object_interface_get(object obj, object_impl impl, const char * key)
{
	(void)obj;

	loader_impl_py_object py_object = (loader_impl_py_object)impl;

	PyObject * pyobject_object = py_object->object;

	PyObject * key_py_str = PyUnicode_FromString(key);
	PyObject * generic_attr = PyObject_GenericGetAttr(pyobject_object, key_py_str);
	Py_DECREF(key_py_str);

	return py_loader_impl_capi_to_value(impl, generic_attr, py_loader_impl_capi_to_value_type(generic_attr));
}

int py_object_interface_set(object obj, object_impl impl, const char * key, value v)
{
	(void)obj;

	loader_impl_py_object py_object = (loader_impl_py_object)impl;

	PyObject * pyobject_object = py_object->object;

	PyObject * key_py_str = PyUnicode_FromString(key);

	PyObject * pyvalue = py_loader_impl_value_to_capi(py_object->impl, value_type_id(v), v);

	int retval =  PyObject_GenericSetAttr(pyobject_object, key_py_str, pyvalue);

	Py_DECREF(key_py_str);

	return retval;
}

value py_object_interface_method_invoke(object obj, object_impl impl, const char * method_name, object_args args, size_t argc)
{
	(void)obj;

	loader_impl_py_object obj_impl = (loader_impl_py_object)impl;

	if (obj_impl == NULL || obj_impl->object == NULL)
	{
		return NULL;
	}
	
	PyObject * method = PyObject_GetAttrString(obj_impl->object, method_name);

    if (method == NULL)
	{
		return NULL;
	}

	PyObject * args_tuple = PyTuple_New(argc);

	if (args_tuple == NULL)
	{
		return NULL;
	}

	for (size_t i = 0; i < argc; i++)
	{
		PyTuple_SET_ITEM(args_tuple, i, py_loader_impl_value_to_capi(obj_impl->impl, value_type_id(args[i]), args[i]));
	}

	PyObject * python_object = PyObject_Call(method, args_tuple, NULL);

	Py_DECREF(args_tuple);
	Py_DECREF(method);

	if (python_object == NULL)
	{
		return NULL;
	}
	
	return py_loader_impl_capi_to_value(impl, python_object, py_loader_impl_capi_to_value_type(python_object));		
}

value py_object_interface_method_await(object obj, object_impl impl, const char * key, object_args args, size_t size, object_resolve_callback resolve, object_reject_callback reject, void * ctx)
{
	// TODO
	(void)obj;
	(void)impl;
	(void)key;
	(void)args;
	(void)size;
	(void)resolve;
	(void)reject;
	(void)ctx;

	return NULL;
}

int py_object_interface_destructor(object obj, object_impl impl)
{
	(void)obj;
	(void)impl;

	/* Python destructors are automatically called when ref count is zero and GC happens */
	
	return 0;
}


void py_object_interface_destroy(object obj, object_impl impl)
{
	(void)obj;

	loader_impl_py_object py_object = (loader_impl_py_object)impl;

	if (py_object != NULL)
	{
		Py_XDECREF(py_object->object);

		Py_XDECREF(py_object->object_class);

		free(py_object);
	}
}

object_interface py_object_interface_singleton(void)
{
	static struct object_interface_type py_object_interface =
	{
		&py_object_interface_create,
		&py_object_interface_get,
		&py_object_interface_set,
		&py_object_interface_method_invoke,
		&py_object_interface_method_await,
		&py_object_interface_destructor,
		&py_object_interface_destroy
	};

	return &py_object_interface;
}

int py_class_interface_create(klass cls, class_impl impl)
{
	(void)cls;

	loader_impl_py_class py_cls = impl;

	py_cls->class = NULL;
    
	return 0;
}

object py_class_interface_constructor(klass cls, class_impl impl, const char * name, class_args args, size_t argc)
{
	(void)cls;

	loader_impl_py_class py_cls = impl;

	loader_impl_py_object py_obj = malloc(sizeof(struct loader_impl_py_object_type));

	object obj = object_create(name, py_obj, &py_object_interface_singleton, cls);

	if (obj == NULL)
	{
		return NULL;
	}

	/* Get loader implementation from class */
	py_obj->impl = py_cls->impl;

	PyObject * args_tuple = PyTuple_New(argc);

	if (args_tuple == NULL)
		return NULL;
	
	for (size_t i = 0; i < argc; i++)
	{
		PyTuple_SET_ITEM(args_tuple, i, py_loader_impl_value_to_capi(py_cls->impl, value_type_id(args[i]), args[i]));
	}
	
	/* Calling the class will create an instance (object) */
	PyObject * python_object = PyObject_CallObject(py_cls->class, args_tuple);

	Py_DECREF(args_tuple);
	
	if (python_object == NULL)
	{
		object_destroy(obj);
		return NULL;
	}

	Py_INCREF(py_cls->class);
	py_obj->object = python_object;
	py_obj->object_class = Py_TYPE(py_cls->class);

	return obj;
}

value py_class_interface_static_get(klass cls, class_impl impl, const char * key)
{
	(void)cls;

	loader_impl_py_class py_class = (loader_impl_py_class)impl;

	PyObject * pyobject_class = py_class->class;

	PyObject * key_py_str = PyUnicode_FromString(key);
	PyObject * generic_attr = PyObject_GenericGetAttr(pyobject_class, key_py_str);
	Py_DECREF(key_py_str);

	return py_loader_impl_capi_to_value(impl, generic_attr, py_loader_impl_capi_to_value_type(generic_attr));
}

int py_class_interface_static_set(klass cls, class_impl impl, const char * key, value v)
{
	(void)cls;

	loader_impl_py_class py_class = (loader_impl_py_class)impl;

	PyObject * pyobject_class = py_class->class;

	PyObject *pyvalue = py_loader_impl_value_to_capi(py_class->impl, value_type_id(v), v);

	PyObject * key_py_str = PyUnicode_FromString(key);

	int retval =  PyObject_GenericSetAttr(pyobject_class, key_py_str, pyvalue);

	Py_DECREF(key_py_str);

	return retval;
}

value py_class_interface_static_invoke(klass cls, class_impl impl, const char * static_method_name, class_args args, size_t argc)
{
	// TODO
	(void)cls;
	(void)impl;
	(void)args;
	
	loader_impl_py_class cls_impl = (loader_impl_py_class)impl;

	if (cls_impl == NULL || cls_impl->class == NULL)
	{
		return NULL;
	}
	
	PyObject * method = PyObject_GetAttrString(cls_impl->class, static_method_name);

    if (method == NULL)
	{
		return NULL;
	}

	PyObject * args_tuple = PyTuple_New(argc);

	if (args_tuple == NULL)
	{
		return NULL;
	}

	for (size_t i = 0; i < argc; i++)
	{
		PyTuple_SET_ITEM(args_tuple, i, py_loader_impl_value_to_capi(cls_impl->impl, value_type_id(args[i]), args[i]));
	}

	PyObject * python_object = PyObject_Call(method, args_tuple, NULL);

	Py_DECREF(args_tuple);
	Py_DECREF(method);

	if (python_object == NULL)
	{
		return NULL;
	}
	
	return py_loader_impl_capi_to_value(impl, python_object, py_loader_impl_capi_to_value_type(python_object));	
}

value py_class_interface_static_await(klass cls, class_impl impl, const char * key, class_args args, size_t size, class_resolve_callback resolve, class_reject_callback reject, void * ctx)
{
	// TODO
	(void)cls;
	(void)impl;
	(void)key;
	(void)args;
	(void)size;
	(void)resolve;
	(void)reject;
	(void)ctx;

	return NULL;
}

void py_class_interface_destroy(klass cls, class_impl impl)
{
	(void)cls;

	loader_impl_py_class py_class = (loader_impl_py_class)impl;

	if (py_class != NULL)
	{
		Py_XDECREF(py_class->class);

		free(py_class);
	}

}

class_interface py_class_interface_singleton(void)
{
	static struct class_interface_type py_class_interface =
	{
		&py_class_interface_create,
		&py_class_interface_constructor,
		&py_class_interface_static_get,
		&py_class_interface_static_set,
		&py_class_interface_static_invoke,
		&py_class_interface_static_await,
		&py_class_interface_destroy
	};

	return &py_class_interface;
}

type_id py_loader_impl_capi_to_value_type(PyObject *obj)
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
	else if (PyObject_IsSubclass(obj, (PyObject *)&PyBaseObject_Type) != 0)
	{
		/* TODO: This is based on trial and error and is not correct, but hey, it works! (for now) */

		/* PyObject_IsSubclass: if the class derived is identical to or derived from PyBaseObject_Type returns 1 */
		/* in case of an error, returns -1 */
		if (PyErr_Occurred() != NULL)
		{
			PyErr_Clear(); // issubclass() arg 1 must be a class

			if (PyObject_TypeCheck(obj, &PyBaseObject_Type))
			{
				/* It's not a class, but it's an instance */
				return TYPE_OBJECT;
			}
		}
		else
		{
			/* Error didn't occur, it's a class! */
			return TYPE_CLASS;
		}
	}

	return TYPE_INVALID;
}

value py_loader_impl_capi_to_value(loader_impl impl, PyObject *obj, type_id id)
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
		const char *str = NULL;

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
		char *str = NULL;

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
		value *array_value;

		length = PyList_Size(obj);

		v = value_create_array(NULL, (size_t)length);

		array_value = value_to_array(v);

		for (iterator = 0; iterator < length; ++iterator)
		{
			PyObject *element = PyList_GetItem(obj, iterator);

			/* TODO: Review recursion overflow */
			array_value[iterator] = py_loader_impl_capi_to_value(impl, element, py_loader_impl_capi_to_value_type(element));
		}
	}
	else if (id == TYPE_MAP)
	{
		Py_ssize_t key_iterator, iterator, keys_size, length = 0;
		value *map_value;
		PyObject *keys;

		keys = PyDict_Keys(obj);
		keys_size = PyList_Size(keys);

		/* TODO: Allow different key types in the future */
		for (iterator = 0; iterator < keys_size; ++iterator)
		{
			PyObject *key = PyList_GetItem(keys, iterator);

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
			const char *key_str = NULL;

			Py_ssize_t key_length = 0;

			PyObject *element, *key;

			value *array_value;

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
		void *ptr = NULL;

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
			PyObject *invoke_state_capsule = PyCFunction_GET_SELF(obj);

			loader_impl_py_function_type_invoke_state invoke_state = PyCapsule_GetPointer(invoke_state_capsule, NULL);

			value callback = value_type_copy(invoke_state->callback);

			/* Move finalizers */
			value_move(callback, invoke_state->callback);

			Py_DECREF(invoke_state_capsule);

			return callback;
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
	else if (id == TYPE_CLASS)
	{
		loader_impl_py_class py_cls = malloc(sizeof(struct loader_impl_py_class_type));

		Py_INCREF(obj);

		klass c = class_create(PyUnicode_AsUTF8(PyObject_Repr(obj)), py_cls, &py_class_interface_singleton);

		py_cls->impl = impl;
		py_cls->class = obj;

		/*
		if (py_loader_impl_discover_class(impl, obj, c) != 0)
		{
			class_destroy(c);
			return NULL;
		}
		*/
		v = value_create_class(c);
	}
	else if (id == TYPE_OBJECT)
	{
		loader_impl_py_object py_obj = malloc(sizeof(struct loader_impl_py_object_type));

		Py_INCREF(obj);
		
		PyTypeObject * object_class = Py_TYPE(obj);
		Py_INCREF(object_class);

		/* TODO: Will capi_to_value recognize and be able to parse a PyTypeObject ? */
		value obj_cls = py_loader_impl_capi_to_value(impl, (PyObject*)object_class, py_loader_impl_capi_to_value_type((PyObject*)object_class));

		/* Not using class_new() here because the object is already instantiated in the runtime */
		/* So we must avoid calling it's constructor again */
		object o = object_create(PyUnicode_AsUTF8(PyObject_Repr(obj)), py_obj, &py_object_interface_singleton, value_to_class(obj_cls));

		py_obj->impl = impl;
		py_obj->object = obj;
		py_obj->object_class = object_class;

		/*
		if (py_loader_impl_validate_object(impl, obj, o) != 0)
		{
			object_destroy(o);
			return NULL;
		}
		*/
	
		v = value_create_object(o);
	}
	else
	{
		/* Return the value as opaque pointer */
		v = value_create_ptr(obj);

		/* Create reference to the value so it does not get garbage collected */
		Py_INCREF(obj);

		/* Set up finalizer in order to free the value */
		value_finalizer(v, &py_loader_impl_value_ptr_finalize, obj);

		log_write("metacall", LOG_LEVEL_WARNING, "Unrecognized Python Type: %s", Py_TYPE(obj)->tp_name);
	}

	return v;
}

PyObject *py_loader_impl_value_to_capi(loader_impl impl, type_id id, value v)
{
	if (id == TYPE_BOOL)
	{
		boolean b = value_to_bool(v);

		long l = (b == 0) ? 0L : 1L;

		return PyBool_FromLong(l);
	}
	else if (id == TYPE_CHAR)
	{
		char s = value_to_char(v);
		long l = (long)s;

		return PyLong_FromLong(l);
	}
	else if (id == TYPE_SHORT)
	{
		short s = value_to_short(v);
		long l = (long)s;

		return PyLong_FromLong(l);
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
		const char *str = value_to_string(v);

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

		const char *buffer = value_to_buffer(v);

#if PY_MAJOR_VERSION == 2

		/* TODO */

#elif PY_MAJOR_VERSION == 3
		return PyBytes_FromStringAndSize(buffer, (Py_ssize_t)size);
#endif
	}
	else if (id == TYPE_ARRAY)
	{
		value *array_value = value_to_array(v);

		Py_ssize_t iterator, array_size = (Py_ssize_t)value_type_count(v);

		PyObject *list = PyList_New(array_size);

		for (iterator = 0; iterator < array_size; ++iterator)
		{
			PyObject *item = py_loader_impl_value_to_capi(impl, value_type_id((value)array_value[iterator]), (value)array_value[iterator]);

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
		log_write("metacall", LOG_LEVEL_ERROR, "TODO: Python map not implemented yet for arguments");
	}
	else if (id == TYPE_PTR)
	{
		void *ptr = value_to_ptr(v);

#if PY_MAJOR_VERSION == 2

		/* TODO */

#elif PY_MAJOR_VERSION == 3
		return PyCapsule_New(ptr, NULL, NULL);
#endif
	}
	else if (id == TYPE_FUTURE)
	{
		/* TODO */
		log_write("metacall", LOG_LEVEL_ERROR, "TODO: Python future not implemented yet for arguments");
	}
	else if (id == TYPE_FUNCTION)
	{
		loader_impl_py_function_type_invoke_state invoke_state = malloc(sizeof(struct loader_impl_py_function_type_invoke_state_type));

		PyObject *invoke_state_capsule;

		if (invoke_state == NULL)
		{
			return NULL;
		}

		invoke_state->impl = impl;
		invoke_state->py_impl = loader_impl_get(impl);
		invoke_state->callback = value_type_copy(v);

		/* Set up finalizer in order to free the invoke state */
		value_finalizer(invoke_state->callback, &py_loader_impl_value_invoke_state_finalize, invoke_state);

		invoke_state_capsule = PyCapsule_New(invoke_state, NULL, NULL);

		Py_XINCREF(invoke_state_capsule);

		return PyCFunction_New(py_loader_impl_function_type_invoke_defs, invoke_state_capsule);
	}
	else if (id == TYPE_NULL)
	{
		Py_RETURN_NONE;
	}
	else if (id == TYPE_CLASS)
	{
		klass obj = value_to_class(v);

		/* TODO: The return value of class_impl_get may not be a loader_impl_py_class, it can be a loader_impl_node_class too */
		loader_impl_py_class obj_impl = class_impl_get(obj);

		if (obj_impl == NULL)
		{
			log_write("metacall", LOG_LEVEL_WARNING, "Cannot retrieve loader_impl_py_class when converting value to python capi");
			return NULL;
		}

		return obj_impl->class;
	}
	else if (id == TYPE_OBJECT)
	{
		object obj = value_to_object(v);
		
		/* TODO: The return value of object_impl_get may not be a loader_impl_py_object, it can be a loader_impl_node_node too */
		loader_impl_py_object obj_impl = object_impl_get(obj);

		if (obj_impl == NULL)
		{
			log_write("metacall", LOG_LEVEL_WARNING, "Cannot retrieve loader_impl_py_object when converting value to python capi");
			return NULL;
		}

		return obj_impl->object;
	}
	else
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Unrecognized value type: %d", id);
	}

	return NULL;
}

function_return function_py_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	loader_impl_py_function py_func = (loader_impl_py_function)impl;
	signature s = function_signature(func);
	const size_t args_size = size;
	const size_t signature_args_size = signature_count(s);
	type ret_type = signature_get_return(s);
	PyObject *result = NULL;
	size_t args_count;
	loader_impl_py py_impl = loader_impl_get(py_func->impl);
	PyGILState_STATE gstate = PyGILState_Ensure();
	PyObject *tuple_args;
	/* Allocate dynamically more space for values in case of variable arguments */
	void ** values = args_size > signature_args_size ? malloc(sizeof(void *) * args_size) : py_func->values;

	/* Possibly a recursive call */
	if (Py_EnterRecursiveCall(" while executing a function in Python Loader") != 0)
	{
		PyGILState_Release(gstate);

		return NULL;
	}

	tuple_args = PyTuple_New(args_size);

	for (args_count = 0; args_count < args_size; ++args_count)
	{
		type t = args_count < signature_args_size ? signature_get_type(s, args_count) : NULL;

		type_id id = TYPE_INVALID;

		if (t == NULL)
		{
			id = value_type_id((value)args[args_count]);
		}
		else
		{
			id = type_index(t);
		}

		values[args_count] = py_loader_impl_value_to_capi(py_func->impl, id, args[args_count]);

		if (values[args_count] != NULL)
		{
			PyTuple_SetItem(tuple_args, args_count, values[args_count]);
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

	/* Variable arguments */
	if (args_size > signature_args_size)
	{
		free(values);
	}

	if (result != NULL)
	{
		value v = NULL;

		type_id id = TYPE_INVALID;

		if (ret_type == NULL)
		{
			id = py_loader_impl_capi_to_value_type(result);
		}
		else
		{
			id = type_index(ret_type);
		}

		v = py_loader_impl_capi_to_value(py_func->impl, result, id);

		Py_DECREF(result);

		PyGILState_Release(gstate);

		return v;
	}

	PyGILState_Release(gstate);

	return NULL;
}

function_return function_py_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;
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

function_interface function_py_singleton(void)
{
	static struct function_interface_type py_function_interface =
		{
			&function_py_interface_create,
			&function_py_interface_invoke,
			&function_py_interface_await,
			&function_py_interface_destroy};

	return &py_function_interface;
}

PyObject *py_loader_impl_function_type_invoke(PyObject *self, PyObject *args)
{
	static void *null_args[1] = {NULL};

	size_t args_size, args_count;

	Py_ssize_t callee_args_size;

	void **value_args;

	value ret;

	loader_impl_py_function_type_invoke_state invoke_state = PyCapsule_GetPointer(self, NULL);

	if (invoke_state == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Fatal error when invoking a function, state cannot be recovered, avoiding the function call");

		Py_RETURN_NONE;
	}

	callee_args_size = PyTuple_Size(args);

	args_size = callee_args_size < 0 ? 0 : (size_t)callee_args_size;

	value_args = args_size == 0 ? null_args : malloc(sizeof(void *) * args_size);

	if (value_args == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid allocation of arguments for callback");

		Py_RETURN_NONE;
	}

	/* Generate metacall values from python values */
	for (args_count = 0; args_count < args_size; ++args_count)
	{
		PyObject *arg = PyTuple_GetItem(args, (Py_ssize_t)args_count);

		type_id id = py_loader_impl_capi_to_value_type(arg);

		value_args[args_count] = py_loader_impl_capi_to_value(invoke_state->impl, arg, id);
	}

	/* Execute the callback */
	ret = (value)function_call(value_to_function(invoke_state->callback), value_args, args_size);

	/* Destroy argument values */
	for (args_count = 0; args_count < args_size; ++args_count)
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
		PyObject *py_ret = py_loader_impl_value_to_capi(invoke_state->impl, value_type_id(ret), ret);

		value_type_destroy(ret);

		return py_ret;
	}

	Py_RETURN_NONE;
}

PyObject *py_loader_impl_get_builtin(loader_impl_py py_impl, const char *builtin_name)
{
	PyObject *builtin = PyObject_GetAttrString(py_impl->builtins_module, builtin_name);

	Py_XINCREF(builtin);

	if (builtin != NULL && PyType_Check(builtin))
	{
		return builtin;
	}

	Py_XDECREF(builtin);

	return NULL;
}

int py_loader_impl_get_builtin_type(loader_impl impl, loader_impl_py py_impl, type_id id, const char *name)
{
	PyObject *builtin = py_loader_impl_get_builtin(py_impl, name);

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
	PyObject *module_name = PyUnicode_DecodeFSDefault("builtins");

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
			const char *name;
		} type_id_name_pair[] =
		{
			{TYPE_BOOL, "bool"},
			{TYPE_LONG, "int"},

#if PY_MAJOR_VERSION == 2
			{TYPE_LONG, "long"},
#endif

			{TYPE_DOUBLE, "float"},

			{TYPE_STRING, "str"},
			{TYPE_BUFFER, "bytes"},
			{TYPE_ARRAY, "list"},
			{TYPE_MAP, "dict"}
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
	PyObject *module_name = PyUnicode_DecodeFSDefault("inspect");

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
	PyObject *module_name = PyUnicode_DecodeFSDefault("traceback");

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
		PyObject *module_name = PyUnicode_DecodeFSDefault("gc");

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

	PyGILState_STATE gstate;

	(void)impl;
	(void)config;

	loader_copy(host);

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

		PyObject *system_path, *current_path;

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

	PyObject *system_modules;

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

loader_handle py_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char *buffer, size_t size)
{
	PyGILState_STATE gstate;

	PyObject *compiled;

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

		log_write("metacall", LOG_LEVEL_DEBUG, "Python loader (%p) importing %s from memory module at (%p)", (void *)impl, name, (void *)py_handle->modules[0].instance);

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

type py_loader_impl_discover_type(loader_impl impl, PyObject *annotation)
{
	type t = NULL;

	if (annotation != NULL)
	{
		PyObject *annotation_qualname = PyObject_GetAttrString(annotation, "__qualname__");

		const char *annotation_name = PyUnicode_AsUTF8(annotation_qualname);

		if (strcmp(annotation_name, "_empty") != 0)
		{
			t = loader_impl_type(impl, annotation_name);

			log_write("metacall", LOG_LEVEL_DEBUG, "Discover type (%p) (%p): %s", (void *)annotation, (void *)type_derived(t), annotation_name);

			Py_DECREF(annotation_qualname);
		}
	}

	return t;
}

int py_loader_impl_discover_func_args_count(PyObject *func)
{
	int args_count = -1;

	if (PyObject_HasAttrString(func, "__call__"))
	{
		PyObject *func_code = NULL;

		if (PyObject_HasAttrString(func, "__code__"))
		{
			func_code = PyObject_GetAttrString(func, "__code__");
		}
		else
		{
			PyObject *func_call = PyObject_GetAttrString(func, "__call__");

			if (func_call != NULL && PyObject_HasAttrString(func_call, "__code__"))
			{
				func_code = PyObject_GetAttrString(func_call, "__code__");
			}

			Py_XDECREF(func_call);
		}

		if (func_code != NULL)
		{
			PyObject *func_code_args_count = PyObject_GetAttrString(func_code, "co_argcount");

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

int py_loader_impl_discover_func(loader_impl impl, PyObject *func, function f)
{
	loader_impl_py py_impl = loader_impl_get(impl);

	PyObject *args = PyTuple_New(1);
	PyObject *result = NULL;

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

		PyObject *parameters = PyObject_GetAttrString(result, "parameters");

		PyObject *return_annotation = PyObject_GetAttrString(result, "return_annotation");

		if (parameters != NULL && PyMapping_Check(parameters))
		{
			PyObject *parameter_list = PyMapping_Values(parameters);

			if (parameter_list != NULL && PyList_Check(parameter_list))
			{
				Py_ssize_t iterator;

				Py_ssize_t parameter_list_size = PyMapping_Size(parameters);

				size_t args_count = signature_count(s);

				if ((size_t)parameter_list_size != args_count)
				{
					/* TODO: Implement properly variable arguments with inspection of the names */
					/* co_argcount in py_loader_impl_discover_func_args_count returns the number */
					/* of arguments (not including keyword only arguments, * or ** args), so they */
					/* won't be inspected but the variable call can be done with metacall*_s API */
					parameter_list_size = (Py_ssize_t)args_count;
				}

				for (iterator = 0; iterator < parameter_list_size; ++iterator)
				{
					PyObject *parameter = PyList_GetItem(parameter_list, iterator);

					if (parameter != NULL)
					{
						PyObject *name = PyObject_GetAttrString(parameter, "name");

						const char *parameter_name = PyUnicode_AsUTF8(name);

						PyObject *annotation = PyObject_GetAttrString(parameter, "annotation");

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

static int py_loader_impl_discover_class(loader_impl impl, PyObject *obj, klass c)
{
	(void)impl;
	(void)c;

	if (PyObject_HasAttrString(obj, "__dict__"))
	{
		PyObject *nameobj = PyUnicode_FromString("__dict__");
		PyObject *read_only_dict = PyObject_GenericGetAttr((PyObject *)obj, nameobj);
		Py_DECREF(nameobj);

		/* Turns out __dict__ is not a PyDict but PyMapping */
		if (!PyObject_TypeCheck(read_only_dict, &PyDictProxy_Type))
		{
			return 1;
		}

		PyObject *dict_items = PyMapping_Items(read_only_dict);

		Py_ssize_t dict_items_size = PyList_Size(dict_items);

		for (Py_ssize_t iterator = 0; iterator < dict_items_size; ++iterator)
		{
			PyObject *tuple = PyList_GetItem(dict_items, iterator);
			PyObject *tuple_key = PyTuple_GetItem(tuple, 0);
			PyObject *tuple_val = PyTuple_GetItem(tuple, 1);

			/* Skip weak references and dict, perhaps we shouldn't in the future? */
			if (!PyUnicode_CompareWithASCIIString(tuple_key, "__dict__") || !PyUnicode_CompareWithASCIIString(tuple_key, "__weakref__"))
				continue;

			// value key = py_loader_impl_capi_to_value(impl, tuple_key, py_loader_impl_capi_to_value_type(tuple_key));
			// value val = py_loader_impl_capi_to_value(impl, tuple_val, py_loader_impl_capi_to_value_type(tuple_val));

			log_write("metacall", LOG_LEVEL_DEBUG, "Introspection: class member %s, type %s",
					  PyUnicode_AsUTF8(tuple_key),
					  type_id_name(py_loader_impl_capi_to_value_type(tuple_val)));
		}
	}

	return 0;
}

/*
static int py_loader_impl_validate_object(loader_impl impl, PyObject *obj, object o)
{
	if (PyObject_HasAttrString(obj, "__dict__"))
	{
		PyObject *dict_key, *dict_val;
		Py_ssize_t pos = 0;

		while (PyDict_Next(PyObject_GetAttrString(obj, "__dict__"), &pos, &dict_key, &dict_val))
		{
			value attribute_key = py_loader_impl_capi_to_value(impl, dict_key, py_loader_impl_capi_to_value_type(dict_key));
			value attribute_val = py_loader_impl_capi_to_value(impl, dict_val, py_loader_impl_capi_to_value_type(dict_val));

			log_write("metacall", LOG_LEVEL_DEBUG, "Discover object member %s, type %s",
					  PyUnicode_AsUTF8(dict_key),
					  type_id_name(py_loader_impl_capi_to_value_type(dict_val)));
			
		}
	}

	return 0;
}
*/

int py_loader_impl_discover_module(loader_impl impl, PyObject *module, context ctx)
{
	PyGILState_STATE gstate;

	gstate = PyGILState_Ensure();

	if (module != NULL && PyModule_Check(module))
	{
		PyObject *module_dict = PyModule_GetDict(module);

		if (module_dict != NULL)
		{
			Py_ssize_t position = 0;

			PyObject *module_dict_key, *module_dict_val;

			while (PyDict_Next(module_dict, &position, &module_dict_key, &module_dict_val))
			{
				if (PyCallable_Check(module_dict_val))
				{
					const char *func_name = PyUnicode_AsUTF8(module_dict_key);

					int discover_args_count = py_loader_impl_discover_func_args_count(module_dict_val);

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

						Py_INCREF(module_dict_val);

						Py_INCREF(module_dict_val);

						py_func->func = module_dict_val;

						py_func->impl = impl;

						f = function_create(func_name, args_count, py_func, &function_py_singleton);

						log_write("metacall", LOG_LEVEL_DEBUG, "Introspection: function %s, args count %ld", func_name, args_count);

						if (py_loader_impl_discover_func(impl, module_dict_val, f) == 0)
						{
							scope sp = context_scope(ctx);

							scope_define(sp, func_name, value_create_function(f));
						}
						else
						{
							function_destroy(f);
						}
					}
				}

				// class is also PyCallable
				// PyObject_IsSubclass(module_dict_val, (PyObject *)&PyType_Type) == 0
				if (PyObject_TypeCheck(module_dict_val, &PyType_Type))
				{
					const char *cls_name = PyUnicode_AsUTF8(module_dict_key);

					log_write("metacall", LOG_LEVEL_DEBUG, "Introspection: class name %s", cls_name);

					loader_impl_py_class py_cls = malloc(sizeof(struct loader_impl_py_class_type));			

					Py_INCREF(module_dict_val);
					
					klass c = class_create(cls_name, py_cls, &py_class_interface_singleton);
					
					py_cls->impl = impl;
					py_cls->class = module_dict_val;
					
					if (py_loader_impl_discover_class(impl, module_dict_val, c) == 0)
					{
						scope sp = context_scope(ctx);

						scope_define(sp, cls_name, value_create_class(c));
					}
					else
					{
						class_destroy(c);
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
			log_write("metacall", LOG_LEVEL_ERROR, "Introspection module discovering error #%" PRIuS " <%p>", iterator, (void *)py_handle->modules[iterator].instance);

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

	PyObject *type, *value, *traceback;

	PyObject *type_str_obj, *value_str_obj, *traceback_str_obj;

	PyObject *traceback_list, *separator;

	const char *type_str, *value_str, *traceback_str;

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

	PyObject *garbage_list, *separator, *garbage_str_obj;

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

void py_loader_impl_sys_path_print(PyObject *sys_path_list)
{
	static const char sys_path_format_str[] = "Python System Paths:\n%s";
	static const char separator_str[] = "\n";

	PyObject *separator, *sys_path_str_obj;

	const char *sys_path_str = NULL;

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
