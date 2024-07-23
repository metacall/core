/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
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

#include <py_loader/py_loader_impl.h>
#include <py_loader/py_loader_port.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>

#include <portability/portability_executable_path.h>
#include <portability/portability_path.h>

#include <reflect/reflect_class.h>
#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <log/log.h>

#include <metacall/metacall.h>

#include <stdbool.h>
#include <stdlib.h>

#include <Python.h>

#define PY_LOADER_IMPL_FUNCTION_TYPE_INVOKE_FUNC "__py_loader_impl_function_type_invoke__"
#define PY_LOADER_IMPL_FINALIZER_FUNC			 "__py_loader_impl_finalizer__"

#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	#define DEBUG_ENABLED 1
#else
	#define DEBUG_ENABLED 0
#endif

typedef struct loader_impl_py_function_type
{
	PyObject *func;
	PyObject **values; // Cache and re-use the values array
	loader_impl impl;
} * loader_impl_py_function;

typedef struct loader_impl_py_future_type
{
	loader_impl impl;
	loader_impl_py py_impl;
	PyObject *future;

} * loader_impl_py_future;

typedef struct loader_impl_py_class_type
{
	PyObject *cls;
	loader_impl impl;

} * loader_impl_py_class;

typedef struct loader_impl_py_object_type
{
	PyObject *obj;
	loader_impl impl;
	value obj_class;

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
	PyObject *inspect_getattr_static;
	PyObject *inspect_getfullargspec;
	PyObject *inspect_ismethod;
	PyObject *inspect_isclass;
	PyObject *builtins_module;
	PyObject *traceback_module;
	PyObject *traceback_format_exception;
	PyObject *import_module;
	PyObject *import_function;

	/* Start asyncio required modules */
	PyObject *asyncio_module;
	PyObject *asyncio_iscoroutinefunction;
	PyObject *asyncio_loop;
	PyObject *thread_background_future_check;
	PyObject *thread_background_module;
	PyObject *thread_background_start;
	PyObject *thread_background_send;
	PyObject *thread_background_stop;
	PyObject *py_task_callback_handler;
	/* End asyncio required modules */

#if DEBUG_ENABLED
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

typedef struct loader_impl_py_await_invoke_callback_state_type
{
	loader_impl impl;
	value func_val;
	function_resolve_callback resolve_callback;
	function_reject_callback reject_callback;
	void *context;
	PyObject *coroutine;

} * loader_impl_py_await_invoke_callback_state;

static int py_loader_impl_check_class(loader_impl_py py_impl, PyObject *obj);

static void py_loader_impl_error_print(loader_impl_py py_impl);

static value py_loader_impl_error_value(loader_impl_py py_impl);

static value py_loader_impl_error_value_from_exception(loader_impl_py py_impl, PyObject *type_obj, PyObject *value_obj, PyObject *traceback_obj);

#if DEBUG_ENABLED
static void py_loader_impl_gc_print(loader_impl_py py_impl);

static void py_loader_impl_sys_path_print(PyObject *sys_path_list);
#endif

static PyObject *py_loader_impl_function_type_invoke(PyObject *self, PyObject *args);

static PyObject *py_loader_impl_finalizer_object_impl(PyObject *self, PyObject *args);

static function_interface function_py_singleton(void);

static type_interface type_py_singleton(void);

static object_interface py_object_interface_singleton(void);

static class_interface py_class_interface_singleton(void);

static size_t py_loader_impl_discover_callable_args_count(loader_impl_py py_impl, PyObject *callable);

static int py_loader_impl_discover_func(loader_impl impl, PyObject *func, function f);

static int py_loader_impl_discover_method(loader_impl impl, PyObject *callable, method m, bool is_static);

static type py_loader_impl_get_type(loader_impl impl, PyObject *obj);

static int py_loader_impl_discover_constructor(loader_impl impl, PyObject *py_class, klass c);

static int py_loader_impl_discover_class(loader_impl impl, PyObject *read_only_dict, klass c);

static void py_loader_impl_value_invoke_state_finalize(value v, void *data);

static void py_loader_impl_value_ptr_finalize(value v, void *data);

static int py_loader_impl_finalize(loader_impl_py py_impl);

static PyObject *py_loader_impl_load_from_memory_compile(loader_impl_py py_impl, const loader_name name, const char *buffer);

static PyMethodDef py_loader_impl_function_type_invoke_defs[] = {
	{ PY_LOADER_IMPL_FUNCTION_TYPE_INVOKE_FUNC,
		py_loader_impl_function_type_invoke,
		METH_VARARGS,
		PyDoc_STR("Implements a trampoline for functions as values in the type system.") },
	{ NULL, NULL, 0, NULL }
};

static PyMethodDef py_loader_impl_finalizer_defs[] = {
	{ PY_LOADER_IMPL_FINALIZER_FUNC,
		py_loader_impl_finalizer_object_impl,
		METH_NOARGS,
		PyDoc_STR("Implements custom destructor for values.") },
	{ NULL, NULL, 0, NULL }
};

struct py_loader_impl_dict_obj
{
	PyDictObject dict;
	value v;
	PyObject *parent;
};

static void py_loader_impl_dict_dealloc(struct py_loader_impl_dict_obj *self);

static PyObject *py_loader_impl_dict_sizeof(struct py_loader_impl_dict_obj *self, void *unused);

static int py_loader_impl_dict_init(struct py_loader_impl_dict_obj *self, PyObject *args, PyObject *kwds);

static struct PyMethodDef py_loader_impl_dict_methods[] = {
	{ "__sizeof__", (PyCFunction)py_loader_impl_dict_sizeof, METH_NOARGS, PyDoc_STR("Get size of dictionary.") },
	{ NULL, NULL, 0, NULL }
};

union py_loader_impl_dict_cast
{
	PyTypeObject *type_object;
	PyObject *object;
};

static PyTypeObject py_loader_impl_dict_type = {
	PyVarObject_HEAD_INIT(NULL, 0) "DictWrapper",
	sizeof(struct py_loader_impl_dict_obj),
	0,
	(destructor)py_loader_impl_dict_dealloc,   /* tp_dealloc */
	0,										   /* tp_vectorcall_offset */
	0,										   /* tp_getattr */
	0,										   /* tp_setattr */
	0,										   /* tp_as_async */
	0,										   /* tp_repr */
	0,										   /* tp_as_number */
	0,										   /* tp_as_sequence */
	0,										   /* tp_as_mapping */
	0,										   /* tp_hash */
	0,										   /* tp_call */
	0,										   /* tp_str */
	0,										   /* tp_getattro */
	0,										   /* tp_setattro */
	0,										   /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  /* tp_flags */
	PyDoc_STR("Dict wrapper destructor hook"), /* tp_doc */
	0,										   /* tp_traverse */
	0,										   /* tp_clear */
	0,										   /* tp_richcompare */
	0,										   /* tp_weaklistoffset */
	0,										   /* tp_iter */
	0,										   /* tp_iternext */
	py_loader_impl_dict_methods,			   /* tp_methods */
	0,										   /* tp_members */
	0,										   /* tp_getset */
	0,										   /* tp_base */
	0,										   /* tp_dict */
	0,										   /* tp_descr_get */
	0,										   /* tp_descr_set */
	0,										   /* tp_dictoffset */
	(initproc)py_loader_impl_dict_init,		   /* tp_init */
	0,										   /* tp_alloc */
	0,										   /* tp_new */
	0,										   /* tp_free */
	0,										   /* tp_is_gc */
	0,										   /* tp_bases */
	0,										   /* tp_mro */
	0,										   /* tp_cache */
	0,										   /* tp_subclasses */
	0,										   /* tp_weaklist */
	0,										   /* tp_del */
	0,										   /* tp_version_tag */
	0,										   /* tp_finalize */
	0,										   /* tp_vectorcall */
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 12
	0 /* tp_watched */
#endif
};

/* Implements: if __name__ == "__main__": */
static int py_loader_impl_run_main = 1;
static char *py_loader_impl_main_module = NULL;

/* Holds reference to the original PyCFunction.tp_dealloc method */
static void (*py_loader_impl_pycfunction_dealloc)(PyObject *) = NULL;

PyObject *py_loader_impl_dict_sizeof(struct py_loader_impl_dict_obj *self, void *Py_UNUSED(unused))
{
	Py_ssize_t res;

	res = _PyDict_SizeOf((PyDictObject *)self);
	res += sizeof(struct py_loader_impl_dict_obj) - sizeof(PyDictObject);
	return PyLong_FromSsize_t(res);
}

int py_loader_impl_dict_init(struct py_loader_impl_dict_obj *self, PyObject *args, PyObject *kwds)
{
	if (PyDict_Type.tp_init((PyObject *)self, args, kwds) < 0)
		return -1;
	self->v = NULL;
	return 0;
}

PyObject *py_loader_impl_finalizer_object_impl(PyObject *self, PyObject *Py_UNUSED(args))
{
	value v = PyCapsule_GetPointer(self, NULL);

	if (v == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Fatal error destroying a value, the metacall value attached to the python value is null");
		Py_RETURN_NONE;
	}

	value_type_destroy(v);

	Py_RETURN_NONE;
}

void py_loader_impl_dict_dealloc(struct py_loader_impl_dict_obj *self)
{
	value_type_destroy(self->v);
	Py_DECREF(self->parent); /* TODO: Review if this is correct or this line is unnecessary */

	PyDict_Type.tp_dealloc((PyObject *)self);
}

PyObject *py_loader_impl_finalizer_wrap_map(PyObject *obj, value v)
{
	PyObject *args = PyTuple_New(1);
	union py_loader_impl_dict_cast dict_cast = { &py_loader_impl_dict_type };

	PyTuple_SetItem(args, 0, obj);
	Py_INCREF(obj);
	PyObject *wrapper = PyObject_CallObject(dict_cast.object, args);
	Py_DECREF(args);

	if (wrapper == NULL)
	{
		return NULL;
	}

	struct py_loader_impl_dict_obj *wrapper_obj = (struct py_loader_impl_dict_obj *)wrapper;

	wrapper_obj->v = v;
	wrapper_obj->parent = obj;

	return wrapper;
}

int py_loader_impl_finalizer_object(loader_impl impl, PyObject *obj, value v)
{
	/* This can be only used for non-builtin modules, any builtin will fail to overwrite the destructor */
	PyObject *v_capsule = PyCapsule_New(v, NULL, NULL);
	PyObject *destructor = PyCFunction_New(py_loader_impl_finalizer_defs, v_capsule);

	if (PyObject_SetAttrString(obj, "__del__", destructor) != 0)
	{
		loader_impl_py py_impl = loader_impl_get(impl);
		py_loader_impl_error_print(py_impl);
		PyErr_Clear();

		log_write("metacall", LOG_LEVEL_ERROR, "Trying to attach a destructor to (probably) a built-in type, "
											   "implement this type for the py_loader_impl_finalizer in order to solve this error");

		if (destructor != NULL)
		{
			/* This will destroy the capsule too */
			Py_DECREF(destructor);
		}
		else
		{
			Py_XDECREF(v_capsule);
		}

		return 1;
	}

	return 0;
}

void py_loader_impl_value_invoke_state_finalize(value v, void *data)
{
	PyObject *capsule = (PyObject *)data;
	loader_impl_py_function_type_invoke_state invoke_state = (loader_impl_py_function_type_invoke_state)PyCapsule_GetPointer(capsule, NULL);

	(void)v;

	if (loader_is_destroyed(invoke_state->impl) != 0 && capsule != NULL)
	{
		PyThreadState *tstate = PyEval_SaveThread();
		PyGILState_STATE gstate = PyGILState_Ensure();
		Py_DECREF(capsule);
		PyGILState_Release(gstate);
		PyEval_RestoreThread(tstate);
	}

	free(invoke_state);
}

void py_loader_impl_value_ptr_finalize(value v, void *data)
{
	type_id id = value_type_id(v);

	if (id == TYPE_PTR)
	{
		if (data != NULL)
		{
			loader_impl impl = (loader_impl)data;

			if (loader_is_destroyed(impl) != 0)
			{
				PyObject *obj = (PyObject *)value_to_ptr(v);
				Py_XDECREF(obj);
			}
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

	if (Py_IsInitialized() != 0)
	{
		PyThreadState *tstate = PyEval_SaveThread();
		PyGILState_STATE gstate = PyGILState_Ensure();
		Py_DECREF(builtin);
		PyGILState_Release(gstate);
		PyEval_RestoreThread(tstate);
	}
}

type_interface type_py_singleton(void)
{
	static struct type_interface_type py_type_interface = {
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

		if (py_func->values == NULL)
		{
			return 1;
		}

		for (size_t iterator = 0; iterator < args_size; ++iterator)
		{
			py_func->values[iterator] = NULL;
		}
	}
	else
	{
		py_func->values = NULL;
	}

	return 0;
}

int future_py_interface_create(future f, future_impl impl)
{
	(void)f;
	(void)impl;

	return 0;
}

void future_py_interface_destroy(future f, future_impl impl)
{
	loader_impl_py_future py_future = (loader_impl_py_future)impl;

	(void)f;

	if (py_future != NULL)
	{
		if (loader_is_destroyed(py_future->impl) != 0)
		{
			PyThreadState *tstate = PyEval_SaveThread();
			PyGILState_STATE gstate = PyGILState_Ensure();
			Py_DECREF(py_future->future);
			PyGILState_Release(gstate);
			PyEval_RestoreThread(tstate);
		}

		free(py_future);
	}
}

future_interface future_py_singleton(void)
{
	static struct future_interface_type py_future_interface = {
		&future_py_interface_create,
		NULL,
		&future_py_interface_destroy
	};

	return &py_future_interface;
}

int py_object_interface_create(object obj, object_impl impl)
{
	(void)obj;

	loader_impl_py_object py_obj = impl;

	py_obj->impl = NULL;
	py_obj->obj = NULL;
	py_obj->obj_class = NULL;

	return 0;
}

/* TODO: get and set is actually the same as static_get and static_set but applied to an object instead of a class */
value py_object_interface_get(object obj, object_impl impl, struct accessor_type *accessor)
{
	(void)obj;

	loader_impl_py_object py_object = (loader_impl_py_object)impl;
	PyObject *pyobject_object = py_object->obj;
	PyObject *key_py_str = PyUnicode_FromString(attribute_name(accessor->data.attr));
	PyObject *generic_attr = PyObject_GenericGetAttr(pyobject_object, key_py_str);
	Py_XDECREF(key_py_str);

	value v = py_loader_impl_capi_to_value(impl, generic_attr, py_loader_impl_capi_to_value_type(py_object->impl, generic_attr));
	Py_XDECREF(generic_attr);

	return v;
}

int py_object_interface_set(object obj, object_impl impl, struct accessor_type *accessor, value v)
{
	(void)obj;

	loader_impl_py_object py_object = (loader_impl_py_object)impl;
	PyObject *pyobject_object = py_object->obj;
	PyObject *key_py_str = PyUnicode_FromString(attribute_name(accessor->data.attr));
	PyObject *pyvalue = py_loader_impl_value_to_capi(py_object->impl, value_type_id(v), v);
	int retval = PyObject_GenericSetAttr(pyobject_object, key_py_str, pyvalue);

	Py_DECREF(key_py_str);

	return retval;
}

value py_object_interface_method_invoke(object obj, object_impl impl, method m, object_args args, size_t argc)
{
	(void)obj;

	loader_impl_py_object obj_impl = (loader_impl_py_object)impl;

	if (obj_impl == NULL || obj_impl->obj == NULL)
	{
		return NULL;
	}

	PyObject *args_tuple = PyTuple_New(argc);

	if (args_tuple == NULL)
	{
		return NULL;
	}

	for (size_t i = 0; i < argc; i++)
	{
		PyTuple_SET_ITEM(args_tuple, i, py_loader_impl_value_to_capi(obj_impl->impl, value_type_id(args[i]), args[i]));
	}

	PyObject *python_object = PyObject_CallMethod(obj_impl->obj, method_name(m), "O", args_tuple, NULL);

	Py_DECREF(args_tuple);

	if (python_object == NULL)
	{
		return NULL;
	}

	value ret = py_loader_impl_capi_to_value(impl, python_object, py_loader_impl_capi_to_value_type(obj_impl->impl, python_object));

	Py_XDECREF(python_object);

	return ret;
}

value py_object_interface_method_await(object obj, object_impl impl, method m, object_args args, size_t size, object_resolve_callback resolve, object_reject_callback reject, void *ctx)
{
	// TODO
	(void)obj;
	(void)impl;
	(void)m;
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
		if (loader_is_destroyed(py_object->impl) != 0)
		{
			PyThreadState *tstate = PyEval_SaveThread();
			PyGILState_STATE gstate = PyGILState_Ensure();
			Py_XDECREF(py_object->obj);

			if (py_object->obj_class != NULL)
			{
				value_type_destroy(py_object->obj_class);
			}
			PyGILState_Release(gstate);
			PyEval_RestoreThread(tstate);
		}

		free(py_object);
	}
}

object_interface py_object_interface_singleton(void)
{
	static struct object_interface_type py_object_interface = {
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

	py_cls->cls = NULL;
	py_cls->impl = NULL;

	return 0;
}

object py_class_interface_constructor(klass cls, class_impl impl, const char *name, constructor ctor, class_args args, size_t argc)
{
	(void)cls;
	(void)ctor;

	loader_impl_py_class py_cls = impl;

	loader_impl_py_object py_obj = malloc(sizeof(struct loader_impl_py_object_type));

	object obj = object_create(name, ACCESSOR_TYPE_STATIC, py_obj, &py_object_interface_singleton, cls);

	if (obj == NULL)
	{
		return NULL;
	}

	/* Get loader implementation from class */
	py_obj->impl = py_cls->impl;
	py_obj->obj_class = NULL;

	PyObject *args_tuple = PyTuple_New(argc);

	if (args_tuple == NULL)
		return NULL;

	for (size_t i = 0; i < argc; i++)
	{
		PyTuple_SET_ITEM(args_tuple, i, py_loader_impl_value_to_capi(py_cls->impl, value_type_id(args[i]), args[i]));
	}

	/* Calling the class will create an instance (object) */
	PyObject *python_object = PyObject_CallObject(py_cls->cls, args_tuple);

	Py_DECREF(args_tuple);

	if (python_object == NULL)
	{
		object_destroy(obj);
		return NULL;
	}

	Py_INCREF(py_cls->cls);
	py_obj->obj = python_object;

	return obj;
}

value py_class_interface_static_get(klass cls, class_impl impl, struct accessor_type *accessor)
{
	(void)cls;

	loader_impl_py_class py_class = (loader_impl_py_class)impl;
	PyObject *pyobject_class = py_class->cls;
	char *attr_name = attribute_name(accessor->data.attr);

	if (attr_name == NULL)
	{
		return NULL;
	}

	PyObject *key_py_str = PyUnicode_FromString(attr_name);
	PyObject *generic_attr = PyObject_GenericGetAttr(pyobject_class, key_py_str);
	Py_XDECREF(key_py_str);

	value v = py_loader_impl_capi_to_value(impl, generic_attr, py_loader_impl_capi_to_value_type(py_class->impl, generic_attr));
	Py_XDECREF(generic_attr);

	return v;
}

int py_class_interface_static_set(klass cls, class_impl impl, struct accessor_type *accessor, value v)
{
	(void)cls;

	loader_impl_py_class py_class = (loader_impl_py_class)impl;
	PyObject *pyobject_class = py_class->cls;
	PyObject *pyvalue = py_loader_impl_value_to_capi(py_class->impl, value_type_id(v), v);
	char *attr_name = attribute_name(accessor->data.attr);

	if (attr_name == NULL)
	{
		return 1;
	}

	PyObject *key_py_str = PyUnicode_FromString(attr_name);
	int retval = PyObject_GenericSetAttr(pyobject_class, key_py_str, pyvalue);

	Py_DECREF(key_py_str);

	return retval;
}

value py_class_interface_static_invoke(klass cls, class_impl impl, method m, class_args args, size_t argc)
{
	(void)cls;

	loader_impl_py_class cls_impl = (loader_impl_py_class)impl;

	if (cls_impl == NULL || cls_impl->cls == NULL)
	{
		return NULL;
	}

	char *static_method_name = method_name(m);

	PyObject *method = PyObject_GetAttrString(cls_impl->cls, static_method_name);

	if (method == NULL)
	{
		return NULL;
	}

	PyObject *args_tuple = PyTuple_New(argc);

	if (args_tuple == NULL)
	{
		return NULL;
	}

	for (size_t i = 0; i < argc; i++)
	{
		PyTuple_SET_ITEM(args_tuple, i, py_loader_impl_value_to_capi(cls_impl->impl, value_type_id(args[i]), args[i]));
	}

	PyObject *python_object = PyObject_Call(method, args_tuple, NULL);

	Py_DECREF(args_tuple);
	Py_DECREF(method);

	if (python_object == NULL)
	{
		return NULL;
	}

	return py_loader_impl_capi_to_value(impl, python_object, py_loader_impl_capi_to_value_type(cls_impl->impl, python_object));
}

value py_class_interface_static_await(klass cls, class_impl impl, method m, class_args args, size_t size, class_resolve_callback resolve, class_reject_callback reject, void *ctx)
{
	// TODO
	(void)cls;
	(void)impl;
	(void)m;
	(void)args;
	(void)size;
	(void)resolve;
	(void)reject;
	(void)ctx;

	return NULL;
}

void py_class_interface_destroy(klass cls, class_impl impl)
{
	loader_impl_py_class py_class = (loader_impl_py_class)impl;

	(void)cls;

	if (py_class != NULL)
	{
		if (loader_is_destroyed(py_class->impl) != 0)
		{
			PyThreadState *tstate = PyEval_SaveThread();
			PyGILState_STATE gstate = PyGILState_Ensure();
			Py_XDECREF(py_class->cls);
			PyGILState_Release(gstate);
			PyEval_RestoreThread(tstate);
		}

		free(py_class);
	}
}

class_interface py_class_interface_singleton(void)
{
	static struct class_interface_type py_class_interface = {
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

int py_loader_impl_check_future(loader_impl_py py_impl, PyObject *obj)
{
	PyObject *args_tuple = PyTuple_New(1);

	Py_INCREF(obj);

	PyTuple_SetItem(args_tuple, 0, obj);

	PyObject *result = PyObject_Call(py_impl->thread_background_future_check, args_tuple, NULL);

	Py_XDECREF(args_tuple);

	if (result == NULL)
	{
		if (PyErr_Occurred() != NULL)
		{
			py_loader_impl_error_print(py_impl);
		}

		return 0;
	}

	int ret = PyObject_IsTrue(result);

	Py_DECREF(result);

	return ret;
}

int py_loader_impl_check_async(loader_impl_py py_impl, PyObject *func)
{
	PyObject *result = PyObject_CallFunctionObjArgs(py_impl->asyncio_iscoroutinefunction, func, NULL);

	if (result == NULL)
	{
		if (PyErr_Occurred() != NULL)
		{
			py_loader_impl_error_print(py_impl);
		}

		return -1;
	}

	int ret = PyObject_IsTrue(result);

	Py_DECREF(result);

	return ret;
}

int py_loader_impl_check_class(loader_impl_py py_impl, PyObject *obj)
{
	PyObject *is_class = PyObject_CallFunction(py_impl->inspect_isclass, "O", obj);

	if (is_class == NULL)
	{
		PyErr_Clear();
		return -1;
	}

	int result = !(PyObject_IsTrue(is_class) == 1);

	Py_DECREF(is_class);

	return result;
}

type_id py_loader_impl_capi_to_value_type(loader_impl impl, PyObject *obj)
{
	loader_impl_py py_impl = loader_impl_get(impl);

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
	else if (PyList_Check(obj) || PyTuple_Check(obj))
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
	else if (PyExceptionInstance_Check(obj))
	{
		return TYPE_EXCEPTION;
	}
	else if (py_loader_impl_check_future(py_impl, obj) == 1)
	{
		return TYPE_FUTURE;
	}

	int result = py_loader_impl_check_class(py_impl, obj);

	if (result == 0)
	{
		return TYPE_CLASS;
	}
	else if (result == 1)
	{
		return TYPE_OBJECT;
	}
	else
	{
		return TYPE_INVALID;
	}
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
			v = value_create_buffer((const void *)str, (size_t)length);
		}
#endif
	}
	else if (id == TYPE_ARRAY)
	{
		Py_ssize_t iterator, length = 0;
		value *array_value;
		Py_ssize_t (*get_size)(PyObject *);
		PyObject *(*get_item)(PyObject *, Py_ssize_t);

		/* Array can be either a tuple or a list, select between them */
		if (PyList_Check(obj))
		{
			get_size = &PyList_Size;
			get_item = &PyList_GetItem;
		}
		else
		{
			get_size = &PyTuple_Size;
			get_item = &PyTuple_GetItem;
		}

		length = get_size(obj);

		v = value_create_array(NULL, (size_t)length);

		array_value = value_to_array(v);

		for (iterator = 0; iterator < length; ++iterator)
		{
			PyObject *element = get_item(obj, iterator);

			/* TODO: Review recursion overflow */
			array_value[iterator] = py_loader_impl_capi_to_value(impl, element, py_loader_impl_capi_to_value_type(impl, element));
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
				array_value[1] = py_loader_impl_capi_to_value(impl, element, py_loader_impl_capi_to_value_type(impl, element));

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
/* Check if we are passing our own hook to the callback */
#if 0 /* TODO: This optimization does not work properly (check metacall-node-port-test for implementing it) */
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
#endif

		loader_impl_py py_impl = loader_impl_get(impl);
		size_t args_count = py_loader_impl_discover_callable_args_count(py_impl, obj);
		loader_impl_py_function py_func = malloc(sizeof(struct loader_impl_py_function_type));
		function f = NULL;

		if (py_func == NULL)
		{
			return NULL;
		}

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
	else if (id == TYPE_NULL)
	{
		v = value_create_null();
	}
	else if (id == TYPE_FUTURE)
	{
		loader_impl_py_future py_future = malloc(sizeof(struct loader_impl_py_future_type));

		future f;

		if (py_future == NULL)
		{
			return NULL;
		}

		f = future_create(py_future, &future_py_singleton);

		if (f == NULL)
		{
			free(py_future);

			return NULL;
		}

		loader_impl_py py_impl = loader_impl_get(impl);

		py_future->impl = impl;
		py_future->py_impl = py_impl;
		py_future->future = obj;

		Py_INCREF(obj);

		v = value_create_future(f);
	}
	else if (id == TYPE_CLASS)
	{
		loader_impl_py_class py_cls = malloc(sizeof(struct loader_impl_py_class_type));

		Py_INCREF(obj);

		PyObject *qualname = PyObject_GetAttrString(obj, "__qualname__");
		klass c = class_create(PyUnicode_AsUTF8(qualname), ACCESSOR_TYPE_STATIC, py_cls, &py_class_interface_singleton);
		Py_XDECREF(qualname);

		py_cls->impl = impl;
		py_cls->cls = obj;

		// TODO: We should register the class during the discover as a type, so here we would
		// be able to retrieve the the class instance by using loader_impl_type

		if (py_loader_impl_discover_class(impl, obj, c) != 0)
		{
			class_destroy(c);
			return NULL;
		}

		v = value_create_class(c);
	}
	else if (id == TYPE_OBJECT)
	{
		loader_impl_py_object py_obj = malloc(sizeof(struct loader_impl_py_object_type));

		Py_INCREF(obj);

		PyObject *object_class = PyObject_Type(obj); /* Increments the class reference count */

		value obj_cls = py_loader_impl_capi_to_value(impl, object_class, py_loader_impl_capi_to_value_type(impl, object_class));

		/* Not using class_new() here because the object is already instantiated in the runtime */
		/* So we must avoid calling it's constructor again */
		PyObject *repr = PyObject_Repr(obj);
		object o = object_create(PyUnicode_AsUTF8(repr), ACCESSOR_TYPE_STATIC, py_obj, &py_object_interface_singleton, value_to_class(obj_cls));
		Py_XDECREF(repr);

		py_obj->impl = impl;
		py_obj->obj = obj;
		py_obj->obj_class = obj_cls;

		/*
		if (py_loader_impl_validate_object(impl, obj, o) != 0)
		{
			object_destroy(o);
			return NULL;
		}
		*/

		v = value_create_object(o);
	}
	else if (id == TYPE_EXCEPTION)
	{
		PyObject *tb = PyException_GetTraceback(obj);

		v = py_loader_impl_error_value_from_exception(loader_impl_get(impl), (PyObject *)Py_TYPE(obj), obj, tb ? tb : Py_None);

		Py_XDECREF(tb);
	}
	else
	{
		/* Return the value as opaque pointer */
		v = value_create_ptr(obj);

		/* Create reference to the value so it does not get garbage collected */
		Py_INCREF(obj);

		/* Set up finalizer in order to free the value */
		value_finalizer(v, &py_loader_impl_value_ptr_finalize, impl);

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
		value *map_value = value_to_map(v);

		Py_ssize_t iterator, map_size = (Py_ssize_t)value_type_count(v);

		PyObject *dict = PyDict_New();

		for (iterator = 0; iterator < map_size; ++iterator)
		{
			value *pair_value = value_to_array(map_value[iterator]);

			PyObject *key = py_loader_impl_value_to_capi(impl, value_type_id((value)pair_value[0]), (value)pair_value[0]);
			PyObject *item = py_loader_impl_value_to_capi(impl, value_type_id((value)pair_value[1]), (value)pair_value[1]);

			if (PyDict_SetItem(dict, key, item) != 0)
			{
				/* TODO: Report error */
			}
		}

		return dict;
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
		Py_RETURN_NONE;
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

		invoke_state_capsule = PyCapsule_New(invoke_state, NULL, NULL);

		Py_XINCREF(invoke_state_capsule);

		/* Set up finalizer in order to free the invoke state */
		value_finalizer(invoke_state->callback, &py_loader_impl_value_invoke_state_finalize, invoke_state_capsule);

		return PyCFunction_New(py_loader_impl_function_type_invoke_defs, invoke_state_capsule);
	}
	else if (id == TYPE_NULL)
	{
		Py_RETURN_NONE;
	}
	else if (id == TYPE_CLASS)
	{
		klass obj = value_to_class(v);

		/* TODO: This is completely wrong and it needs a refactor */
		/* TODO: The return value of class_impl_get may not be a loader_impl_py_class, it can be a loader_impl_node_class too */
		/* TODO: We must detect if it comes from python and use this method, otherwise we must create the class dynamically */
		loader_impl_py_class obj_impl = class_impl_get(obj);

		if (obj_impl == NULL)
		{
			log_write("metacall", LOG_LEVEL_WARNING, "Cannot retrieve loader_impl_py_class when converting value to python capi");
			return NULL;
		}

		return obj_impl->cls;
	}
	else if (id == TYPE_OBJECT)
	{
		object obj = value_to_object(v);

		/* TODO: This is completely wrong and it needs a refactor */
		/* TODO: The return value of object_impl_get may not be a loader_impl_py_object, it can be a loader_impl_node_node too */
		/* TODO: We must detect if it comes from python and use this method, otherwise we must create the object dynamically */
		loader_impl_py_object obj_impl = object_impl_get(obj);
		Py_INCREF(obj_impl->obj);

		if (obj_impl == NULL)
		{
			log_write("metacall", LOG_LEVEL_WARNING, "Cannot retrieve loader_impl_py_object when converting value to python capi");
			return NULL;
		}

		return obj_impl->obj;
	}
	else
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Unrecognized value type: %d", id);
	}

	return NULL;
}

PyObject *py_task_callback_handler_impl_unsafe(PyThreadState *tstate, PyGILState_STATE gstate, PyObject *pyfuture)
{
	PyObject *capsule = PyObject_GetAttrString(pyfuture, "__metacall_capsule");
	if (capsule == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid python capsule in task_callback_handler");
		Py_RETURN_NONE;
	}

	loader_impl_py_await_invoke_callback_state callback_state = PyCapsule_GetPointer(capsule, NULL);
	Py_DECREF(capsule);

	/* pyfuture should never raise InvalidStateError exception here */
	/* because this is a callback set in Future.add_done_callback */

	PyObject *result_str = PyUnicode_FromString("result");
	PyObject *result = PyObject_CallMethodObjArgs(pyfuture, result_str, NULL);
	Py_DECREF(result_str);

	value v = NULL, ret = NULL;

	if (result != NULL)
	{
		type_id id = py_loader_impl_capi_to_value_type(callback_state->impl, result);
		v = py_loader_impl_capi_to_value(callback_state->impl, result, id);

		Py_DECREF(result);

		PyGILState_Release(gstate);
		PyEval_RestoreThread(tstate);
		ret = callback_state->resolve_callback(v, callback_state->context);
		tstate = PyEval_SaveThread();
		gstate = PyGILState_Ensure();
	}
	else
	{
		if (PyErr_Occurred() != NULL)
		{
			PyObject *error_type, *error_value, *error_traceback;

			PyErr_Fetch(&error_type, &error_value, &error_traceback);

			PyObject *args = PyObject_GetAttrString(error_value, "args");

			/* Retrieve the contents of the exception (TODO: Pass the exception directly as value when we support the exception type?) */
			if (args != NULL && PyTuple_Check(args))
			{
				PyObject *val = PyTuple_GetItem(args, 0);
				type_id id = py_loader_impl_capi_to_value_type(callback_state->impl, val);
				v = py_loader_impl_capi_to_value(callback_state->impl, val, id);
			}

			Py_XDECREF(args);

			PyErr_Clear();
		}

		if (v == NULL)
		{
			v = value_create_null();
		}

		PyGILState_Release(gstate);
		PyEval_RestoreThread(tstate);
		ret = callback_state->reject_callback(v, callback_state->context);
		tstate = PyEval_SaveThread();
		gstate = PyGILState_Ensure();
	}

	loader_impl impl = callback_state->impl;

	value_type_destroy(v);
	Py_DECREF(callback_state->coroutine);
	value_type_destroy(callback_state->func_val);
	free(callback_state);

	if (ret == NULL)
	{
		Py_RETURN_NONE;
	}
	else
	{
		return py_loader_impl_value_to_capi(impl, value_type_id(ret), ret);
	}
}

PyObject *py_task_callback_handler_impl(PyObject *self, PyObject *pyfuture)
{
	PyThreadState *tstate = PyEval_SaveThread();
	PyGILState_STATE gstate = PyGILState_Ensure();

	/* self will always be NULL */
	(void)self;

	PyObject *result = py_task_callback_handler_impl_unsafe(tstate, gstate, pyfuture);

	PyGILState_Release(gstate);
	PyEval_RestoreThread(tstate);

	return result;
}

function_return function_py_interface_invoke(function func, function_impl impl, function_args args, size_t args_size)
{
	loader_impl_py_function py_func = (loader_impl_py_function)impl;
	signature s = function_signature(func);
	const size_t signature_args_size = signature_count(s);
	type ret_type = signature_get_return(s);
	loader_impl_py py_impl = loader_impl_get(py_func->impl);
	PyThreadState *tstate = PyEval_SaveThread();
	PyGILState_STATE gstate = PyGILState_Ensure();
	value v = NULL;

	/* Possibly a recursive call */
	if (Py_EnterRecursiveCall(" while executing a function in Python Loader") != 0)
	{
		goto finalize;
	}

	PyObject *tuple_args = PyTuple_New(args_size);

	/* Allocate dynamically more space for values in case of variable arguments */
	bool is_var_args = args_size > signature_args_size || py_func->values == NULL;
	PyObject **values = is_var_args ? malloc(sizeof(PyObject *) * args_size) : py_func->values;

	for (size_t args_count = 0; args_count < args_size; ++args_count)
	{
		type t = args_count < signature_args_size ? signature_get_type(s, args_count) : NULL;
		type_id id = t == NULL ? value_type_id((value)args[args_count]) : type_index(t);
		values[args_count] = py_loader_impl_value_to_capi(py_func->impl, id, args[args_count]);

		if (values[args_count] != NULL)
		{
			PyTuple_SET_ITEM(tuple_args, args_count, values[args_count]);
		}
	}

	PyObject *result = PyObject_CallObject(py_func->func, tuple_args);

	/* End of recursive call */
	Py_LeaveRecursiveCall();

	if (PyErr_Occurred() != NULL)
	{
		v = py_loader_impl_error_value(py_impl);

		if (v == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Fatal error when trying to fetch an exeption");
		}
	}

	Py_XDECREF(tuple_args);

	if (is_var_args)
	{
		free(values);
	}

	if (result == NULL || v != NULL)
	{
		goto finalize;
	}

	type_id id = ret_type == NULL ? py_loader_impl_capi_to_value_type(py_func->impl, result) : type_index(ret_type);
	v = py_loader_impl_capi_to_value(py_func->impl, result, id);

	Py_DECREF(result);
finalize:
	PyGILState_Release(gstate);
	PyEval_RestoreThread(tstate);
	return v;
}

function_return function_py_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	loader_impl_py_function py_func = (loader_impl_py_function)impl;
	signature s = function_signature(func);
	const size_t args_size = size;
	const size_t signature_args_size = signature_count(s);
	type ret_type = signature_get_return(s);
	PyObject *pyfuture = NULL;
	size_t args_count;
	loader_impl_py py_impl = loader_impl_get(py_func->impl);
	PyThreadState *tstate = PyEval_SaveThread();
	PyGILState_STATE gstate = PyGILState_Ensure();
	PyObject *tuple_args;

	/* Allocate dynamically more space for values in case of variable arguments */
	void **values = args_size > signature_args_size ? malloc(sizeof(void *) * args_size) : py_func->values;

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

	PyObject *coroutine = PyObject_CallObject(py_func->func, tuple_args);

	if (coroutine == NULL || PyErr_Occurred() != NULL)
	{
		Py_XDECREF(coroutine);
		goto error;
	}

	/* Send coroutine to the asyncio thread */
	PyObject *args_tuple = PyTuple_New(4); /* loop, coro, callback, capsule */

	if (args_tuple == NULL)
	{
		Py_DECREF(coroutine);
		goto error;
	}

	/* Use this capsule to handle the callback pointers to the python task callback */
	loader_impl_py_await_invoke_callback_state callback_state = malloc(sizeof(struct loader_impl_py_await_invoke_callback_state_type));

	if (callback_state == NULL)
	{
		Py_DECREF(coroutine);
		Py_DECREF(args_tuple);
		goto error;
	}

	callback_state->resolve_callback = resolve_callback;
	callback_state->reject_callback = reject_callback;
	callback_state->impl = py_func->impl;
	callback_state->context = context;
	callback_state->coroutine = coroutine;

	PyObject *callback_status = PyCapsule_New(callback_state, NULL, NULL);

	if (callback_status == NULL)
	{
		Py_DECREF(coroutine);
		Py_DECREF(args_tuple);
		free(callback_state);
		goto error;
	}

	/* Create a reference to the function so we avoid to delete it when destroying the event loop */
	callback_state->func_val = value_create_function(func);

	Py_INCREF(py_impl->asyncio_loop);
	Py_INCREF(coroutine);
	Py_INCREF(py_impl->py_task_callback_handler);
	Py_INCREF(callback_status);

	PyTuple_SetItem(args_tuple, 0, py_impl->asyncio_loop);
	PyTuple_SetItem(args_tuple, 1, coroutine);
	PyTuple_SetItem(args_tuple, 2, py_impl->py_task_callback_handler);
	PyTuple_SetItem(args_tuple, 3, callback_status);

	pyfuture = PyObject_Call(py_impl->thread_background_send, args_tuple, NULL);
	Py_DECREF(args_tuple);

	/* Variable arguments */
	if (args_size > signature_args_size)
	{
		free(values);
	}

	if (pyfuture != NULL)
	{
		value v = NULL;

		type_id id = TYPE_INVALID;

		if (ret_type == NULL)
		{
			id = py_loader_impl_capi_to_value_type(py_func->impl, pyfuture);
		}
		else
		{
			id = type_index(ret_type);
		}

		v = py_loader_impl_capi_to_value(py_func->impl, pyfuture, id);

		Py_DECREF(pyfuture);
		Py_DECREF(tuple_args);

		PyGILState_Release(gstate);
		PyEval_RestoreThread(tstate);

		return v;
	}

error:
	if (PyErr_Occurred() != NULL)
	{
		py_loader_impl_error_print(py_impl);
	}

	Py_XDECREF(pyfuture);
	Py_DECREF(tuple_args);

	PyGILState_Release(gstate);
	PyEval_RestoreThread(tstate);

	return NULL;
}

void function_py_interface_destroy(function func, function_impl impl)
{
	loader_impl_py_function py_func = (loader_impl_py_function)impl;

	if (py_func != NULL)
	{
		if (py_func->values != NULL)
		{
			(void)func;
			free(py_func->values);
		}

		if (loader_is_destroyed(py_func->impl) != 0)
		{
			PyThreadState *tstate = PyEval_SaveThread();
			PyGILState_STATE gstate = PyGILState_Ensure();
			Py_DECREF(py_func->func);
			PyGILState_Release(gstate);
			PyEval_RestoreThread(tstate);
		}

		free(py_func);
	}
}

function_interface function_py_singleton(void)
{
	static struct function_interface_type py_function_interface = {
		&function_py_interface_create,
		&function_py_interface_invoke,
		&function_py_interface_await,
		&function_py_interface_destroy
	};

	return &py_function_interface;
}

PyObject *py_loader_impl_function_type_invoke(PyObject *self, PyObject *args)
{
	static void *null_args[1] = { NULL };

	loader_impl_py_function_type_invoke_state invoke_state = PyCapsule_GetPointer(self, NULL);

	if (invoke_state == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Fatal error when invoking a function, state cannot be recovered, avoiding the function call");
		Py_RETURN_NONE;
	}

	Py_ssize_t callee_args_size = PyTuple_Size(args);
	size_t args_size = callee_args_size < 0 ? 0 : (size_t)callee_args_size;
	void **value_args = args_size == 0 ? null_args : malloc(sizeof(void *) * args_size);

	if (value_args == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid allocation of arguments for callback");
		Py_RETURN_NONE;
	}

	/* Generate metacall values from python values */
	for (size_t args_count = 0; args_count < args_size; ++args_count)
	{
		PyObject *arg = PyTuple_GetItem(args, (Py_ssize_t)args_count);
		type_id id = py_loader_impl_capi_to_value_type(invoke_state->impl, arg);
		value_args[args_count] = py_loader_impl_capi_to_value(invoke_state->impl, arg, id);
	}

	/* Execute the callback */
	value ret = (value)function_call(value_to_function(invoke_state->callback), value_args, args_size);

	/* Destroy argument values */
	for (size_t args_count = 0; args_count < args_size; ++args_count)
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

	if (builtin == NULL)
	{
		goto error_get_builtin;
	}

	type builtin_type = type_create(id, name, builtin, &type_py_singleton);

	if (builtin_type == NULL)
	{
		goto error_create_type;
	}

	if (loader_impl_type_define(impl, type_name(builtin_type), builtin_type) == 0)
	{
		return 0;
	}

	type_destroy(builtin_type);
error_create_type:
	Py_DECREF(builtin);
error_get_builtin:
	return 1;
}

int py_loader_impl_import_module(loader_impl_py py_impl, PyObject **loc, const char *name)
{
	PyObject *module_name = PyUnicode_DecodeFSDefault(name);
	*loc = PyImport_Import(module_name);

	Py_DECREF(module_name);

	if (*loc == NULL)
	{
		py_loader_impl_error_print(py_impl);
		return 1;
	}

	return 0;
}

int py_loader_impl_initialize_inspect_types(loader_impl impl, loader_impl_py py_impl)
{
	if (py_loader_impl_import_module(py_impl, &py_impl->builtins_module, "builtins") != 0)
	{
		goto error_import_module;
	}

	/* TODO: move this to loader_impl */

	static struct
	{
		type_id id;
		const char *name;
	} type_id_name_pair[] = {
		{ TYPE_BOOL, "bool" },
		{ TYPE_LONG, "int" },

#if PY_MAJOR_VERSION == 2
		{ TYPE_LONG, "long" },
#endif

		{ TYPE_DOUBLE, "float" },

		{ TYPE_STRING, "str" },
		{ TYPE_BUFFER, "bytes" },
		{ TYPE_ARRAY, "list" },
		{ TYPE_ARRAY, "tuple" },
		{ TYPE_MAP, "dict" }
	};

	size_t size = sizeof(type_id_name_pair) / sizeof(type_id_name_pair[0]);

	for (size_t index = 0; index < size; ++index)
	{
		if (py_loader_impl_get_builtin_type(impl, py_impl,
				type_id_name_pair[index].id,
				type_id_name_pair[index].name) != 0)
		{
			if (PyErr_Occurred() != NULL)
			{
				py_loader_impl_error_print(py_impl);
			}

			goto error_get_builtin_type;
		}
	}

	return 0;

error_get_builtin_type:
	Py_DECREF(py_impl->builtins_module);
error_import_module:
	return 1;
}

int py_loader_impl_initialize_inspect(loader_impl impl, loader_impl_py py_impl)
{
	if (py_loader_impl_import_module(py_impl, &py_impl->inspect_module, "inspect") != 0)
	{
		goto error_import_module;
	}

	py_impl->inspect_signature = PyObject_GetAttrString(py_impl->inspect_module, "signature");

	if (py_impl->inspect_signature == NULL)
	{
		goto error_inspect_signature;
	}

	py_impl->inspect_getattr_static = PyObject_GetAttrString(py_impl->inspect_module, "getattr_static");

	if (py_impl->inspect_getattr_static == NULL)
	{
		goto error_inspect_getattr_static;
	}

	py_impl->inspect_getfullargspec = PyObject_GetAttrString(py_impl->inspect_module, "getfullargspec");

	if (py_impl->inspect_getfullargspec == NULL)
	{
		goto error_inspect_getfullargspec;
	}

	py_impl->inspect_ismethod = PyObject_GetAttrString(py_impl->inspect_module, "ismethod");

	if (py_impl->inspect_ismethod == NULL)
	{
		goto error_inspect_ismethod;
	}

	py_impl->inspect_isclass = PyObject_GetAttrString(py_impl->inspect_module, "isclass");

	if (py_impl->inspect_isclass == NULL)
	{
		goto error_inspect_isclass;
	}

	if (PyCallable_Check(py_impl->inspect_signature) && py_loader_impl_initialize_inspect_types(impl, py_impl) == 0)
	{
		return 0;
	}

	Py_DECREF(py_impl->inspect_isclass);
error_inspect_isclass:
	Py_DECREF(py_impl->inspect_ismethod);
error_inspect_ismethod:
	Py_DECREF(py_impl->inspect_getfullargspec);
error_inspect_getfullargspec:
	Py_DECREF(py_impl->inspect_getattr_static);
error_inspect_getattr_static:
	Py_DECREF(py_impl->inspect_signature);
error_inspect_signature:
	Py_DECREF(py_impl->inspect_module);
error_import_module:
	return 1;
}

int py_loader_impl_initialize_asyncio_module(loader_impl_py py_impl)
{
	PyObject *module_name = PyUnicode_DecodeFSDefault("asyncio");
	py_impl->asyncio_module = PyImport_Import(module_name);

	Py_DECREF(module_name);

	if (PyErr_Occurred() != NULL)
	{
		py_loader_impl_error_print(py_impl);
		return 1;
	}

	if (py_impl->asyncio_module == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error getting asyncio module or threading module not loaded");
		return 2;
	}

	py_impl->asyncio_iscoroutinefunction = PyObject_GetAttrString(py_impl->asyncio_module, "iscoroutinefunction");

	if (py_impl->asyncio_iscoroutinefunction == NULL || !PyCallable_Check(py_impl->asyncio_iscoroutinefunction))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error getting asyncio.iscoroutinefunction(func)");
		goto error_after_asyncio_module;
	}

	/* Wrap the C callback around a PyObject so that the interpreter knows how to call back as if it was python code */
	static PyMethodDef py_task_callback_handler_def = {
		"metacall_task_callback_handler",
		py_task_callback_handler_impl,
		METH_O,
		"MetaCall async invoke callback"
	};

	py_impl->py_task_callback_handler = PyCFunction_NewEx(&py_task_callback_handler_def, NULL, NULL);

	if (py_impl->py_task_callback_handler == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error produced while creating the task callback handler for asyncio");
		goto error_after_asyncio_iscoroutinefunction;
	}

	/* Start the asyncio thread */
	PyObject *args_tuple = PyTuple_New(0);
	py_impl->asyncio_loop = PyObject_Call(py_impl->thread_background_start, args_tuple, NULL);
	Py_XDECREF(args_tuple);

	if (py_impl->asyncio_loop == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error produced while starting the asyncio thread");
		goto error_after_py_task_callback_handler;
	}

	return 0;

error_after_py_task_callback_handler:
	Py_DECREF(py_impl->py_task_callback_handler);
error_after_asyncio_iscoroutinefunction:
	Py_DECREF(py_impl->asyncio_iscoroutinefunction);
error_after_asyncio_module:
	Py_DECREF(py_impl->asyncio_module);

	return 1;
}

int py_loader_impl_initialize_traceback(loader_impl impl, loader_impl_py py_impl)
{
	(void)impl;

	if (py_loader_impl_import_module(py_impl, &py_impl->traceback_module, "traceback") != 0)
	{
		goto error_import_module;
	}

	py_impl->traceback_format_exception = PyObject_GetAttrString(py_impl->traceback_module, "format_exception");

	if (py_impl->traceback_format_exception == NULL)
	{
		goto error_format_exception;
	}

	if (PyCallable_Check(py_impl->traceback_format_exception))
	{
		return 0;
	}

	Py_XDECREF(py_impl->traceback_format_exception);
error_format_exception:
	Py_DECREF(py_impl->traceback_module);
error_import_module:
	return 1;
}

int py_loader_impl_initialize_gc(loader_impl_py py_impl)
{
#if DEBUG_ENABLED
	if (py_loader_impl_import_module(py_impl, &py_impl->gc_module, "gc") != 0)
	{
		goto error_import_module;
	}

	py_impl->gc_set_debug = PyObject_GetAttrString(py_impl->gc_module, "set_debug");

	if (py_impl->gc_set_debug == NULL)
	{
		goto error_set_debug;
	}

	if (!PyCallable_Check(py_impl->gc_set_debug))
	{
		goto error_callable_check;
	}

	py_impl->gc_debug_leak = PyDict_GetItemString(PyModule_GetDict(py_impl->gc_module), "DEBUG_LEAK");
	py_impl->gc_debug_stats = PyDict_GetItemString(PyModule_GetDict(py_impl->gc_module), "DEBUG_STATS");

	if (py_impl->gc_debug_leak != NULL && py_impl->gc_debug_stats != NULL)
	{
		Py_INCREF(py_impl->gc_debug_leak);
		Py_INCREF(py_impl->gc_debug_stats);

		return 0;
	}

	Py_XDECREF(py_impl->gc_debug_leak);
	Py_XDECREF(py_impl->gc_debug_stats);

error_callable_check:
	Py_XDECREF(py_impl->gc_set_debug);
error_set_debug:
	Py_DECREF(py_impl->gc_module);
error_import_module:
	return 1;
#else
	{
		(void)py_impl;

		return 1;
	}
#endif
}

int py_loader_impl_initialize_import(loader_impl_py py_impl)
{
	static const char import_module_str[] =
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 5
		"import sys\n"
		"import importlib.util\n"
		"import importlib\n"
		"from pathlib import Path\n"
		"\n"
		"def load_from_spec(module_name, spec):\n"
		"	m = importlib.util.module_from_spec(spec)\n"
		"	spec.loader.exec_module(m)\n"
		"	sys.modules[module_name] = m\n"
		"	return m\n"
		"\n"
		"def load_from_path(module_name, path = None):\n"
		"	search_location = None\n"
		"	try:\n"
		"		if path == None:\n"
		"			if module_name in sys.modules:\n"
		"				return sys.modules[module_name]\n"
		"			else:\n"
		"				spec = importlib.util.find_spec(module_name)\n"
		"				if spec is None:\n"
		"					return FileNotFoundError('Module ' + module_name + ' could not be found')\n"
		"				return load_from_spec(module_name, spec)\n"
		"		else:\n"
		"			current_search_location = str(Path(path).parent.absolute())\n"
		"			if not current_search_location in sys.path:\n"
		"				search_location = current_search_location\n"
		"				sys.path.insert(0, current_search_location)\n"
		"			spec = importlib.util.spec_from_file_location(module_name, path, submodule_search_locations=[current_search_location])\n"
		"			if spec is None:\n"
		"				if search_location is not None:\n"
		"					sys.path.pop(0)\n"
		"				return FileNotFoundError('File ' + path + ' could not be found')\n"
		"			module = load_from_spec(module_name, spec)\n"
		"			if search_location is not None:\n"
		"				sys.path.pop(0)\n"
		"			return module\n"
		"	except FileNotFoundError as e:\n"
		"		if search_location is not None:\n"
		"			sys.path.pop(0)\n"
		"		return e\n"
		"	except Exception as e:\n"
		"		import traceback\n"
		"		print(traceback.format_exc())\n"
		"		if search_location is not None:\n"
		"			sys.path.pop(0)\n"
		"		return e\n"
#elif PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 3
		// TODO: Not tested
		// TODO: Implement load from module (optional path)
		// TODO: Catch exceptions and implement better error handling
		"from importlib.machinery import SourceFileLoader\n"
		"def load_from_path(module_name, path):\n"
		"	return SourceFileLoader(module_name, path).load_module()\n"
#elif PY_MAJOR_VERSION == 2
		// TODO: Not tested
		// TODO: Implement load from module (optional path)
		// TODO: Catch exceptions and implement better error handling
		"import imp\n"
		"def load_from_path(module_name, path):\n"
		"	return imp.load_source(module_name, path)\n"
#else
	#error "Import module not supported in this Python version"
#endif
		;

	static const loader_name name = "py_loader_impl_load_from_file_path";

	py_impl->import_module = py_loader_impl_load_from_memory_compile(py_impl, name, import_module_str);

	if (py_impl->import_module == NULL)
	{
		goto error_import_compile;
	}

	py_impl->import_function = PyObject_GetAttrString(py_impl->import_module, "load_from_path");

	if (py_impl->import_function == NULL || !PyCallable_Check(py_impl->import_function))
	{
		if (PyErr_Occurred() != NULL)
		{
			goto error_import_function;
		}
	}

	return 0;
error_import_function:
	Py_DECREF(py_impl->import_module);
error_import_compile:
	py_loader_impl_error_print(py_impl);
	return 1;
}

int py_loader_impl_initialize_thread_background_module(loader_impl_py py_impl)
{
	static const char thread_background_module_str[] =
		"import asyncio\n"
		"from threading import Thread\n"
		"from threading import Event\n"
		"class ThreadLoop:\n"
		"	def __init__(self, loop, t):\n"
		"		self.loop = loop\n"
		"		self.t = t\n"
		"def future_check(f):\n"
		"	return asyncio.isfuture(f)\n"
		"def future_create(tl):\n"
		"	return tl.loop.create_future()\n"
		"def future_resolve(tl, f, value):\n"
		"	tl.loop.call_soon_threadsafe(f.set_result, value)\n"
		"def future_reject(tl, f, exception):\n"
		"	tl.loop.call_soon_threadsafe(f.set_exception, exception)\n"
		"def background_loop(loop):\n"
		"	asyncio.set_event_loop(loop)\n"
		"	loop.run_forever()\n"
		"	loop.run_until_complete(loop.shutdown_asyncgens())\n"
		"	loop.stop()\n"
		"	loop.close()\n"
		"def start_background_loop():\n"
		"	loop = asyncio.new_event_loop()\n"
		"	t = Thread(target=background_loop, name='MetaCall asyncio event loop', args=(loop,), daemon=False)\n"
		"	t.start()\n"
		"	return ThreadLoop(loop, t)\n"
		"def send_background_loop(tl, coro, callback, capsule):\n"
		"	task = asyncio.run_coroutine_threadsafe(coro, tl.loop)\n"
		"	task.__metacall_capsule = capsule\n"
		"	task.add_done_callback(callback)\n"
		"	return asyncio.wrap_future(task, loop=tl.loop)\n"
		"def stop_background_loop(tl):\n"
		"	tl.loop.call_soon_threadsafe(tl.loop.stop)\n"
		"	tl.t.join()\n";

	/* How to use the module: */
	/*
	import threading
	import time

	async def async_func(n):
		print('inside async_func', threading.current_thread().ident, ':', n)
		return 54

	def loop_handler(task):
		print('inside handler', threading.current_thread().ident)
		print('task capsule', task.__metacall_capsule)
		print(task.result())

	def main():
		print('main thread', threading.current_thread().ident)
		loop = start_background_loop()
		f = send_background_loop(loop, async_func(6), loop_handler, 12)
		print('future:', f)
		time.sleep(5)
		stop_background_loop(loop)

	if __name__ == "__main__":
		main()
	*/

	static const loader_name name = "py_loader_impl_thread_background";

	py_impl->thread_background_module = py_loader_impl_load_from_memory_compile(py_impl, name, thread_background_module_str);

	if (py_impl->thread_background_module == NULL)
	{
		goto error_thread_background_compile;
	}

	py_impl->thread_background_future_check = PyObject_GetAttrString(py_impl->thread_background_module, "future_check");

	if (py_impl->thread_background_future_check == NULL || !PyCallable_Check(py_impl->thread_background_future_check))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error getting future_check function");
		goto error_thread_background_future_check;
	}

	py_impl->thread_background_start = PyObject_GetAttrString(py_impl->thread_background_module, "start_background_loop");

	if (py_impl->thread_background_start == NULL || !PyCallable_Check(py_impl->thread_background_start))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error getting start_background_loop function");
		goto error_thread_background_start;
	}

	py_impl->thread_background_send = PyObject_GetAttrString(py_impl->thread_background_module, "send_background_loop");

	if (py_impl->thread_background_send == NULL || !PyCallable_Check(py_impl->thread_background_send))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error getting send_background_loop function");
		goto error_thread_background_send;
	}

	py_impl->thread_background_stop = PyObject_GetAttrString(py_impl->thread_background_module, "stop_background_loop");

	if (py_impl->thread_background_stop == NULL || !PyCallable_Check(py_impl->thread_background_stop))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Error getting stop_background_loop function");
		goto error_thread_background_stop;
	}

	return 0;

error_thread_background_stop:
	Py_XDECREF(py_impl->thread_background_stop);
error_thread_background_send:
	Py_XDECREF(py_impl->thread_background_send);
error_thread_background_start:
	Py_XDECREF(py_impl->thread_background_start);
error_thread_background_future_check:
	Py_XDECREF(py_impl->thread_background_future_check);
error_thread_background_compile:
	Py_XDECREF(py_impl->thread_background_module);

	if (PyErr_Occurred() != NULL)
	{
		py_loader_impl_error_print(py_impl);
	}

	return 1;
}

int py_loader_impl_initialize_sys_executable(loader_impl_py py_impl)
{
	portability_executable_path_str exe_path_str = { 0 };
	portability_executable_path_length length = 0;

	if (portability_executable_path(exe_path_str, &length) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Python loader failed to retrieve the executable path");
		return 1;
	}

	PyObject *exe_path_obj = PyUnicode_DecodeFSDefaultAndSize(exe_path_str, (Py_ssize_t)length);

	if (exe_path_obj == NULL)
	{
		return 1;
	}

	int result = PySys_SetObject("executable", exe_path_obj);

	Py_DECREF(exe_path_obj);

	if (result == -1)
	{
		if (PyErr_Occurred() != NULL)
		{
			py_loader_impl_error_print(py_impl);
			PyErr_Clear();
		}

		return 1;
	}

	return 0;
}

int py_loader_impl_initialize_argv(loader_impl_py py_impl, int argc, char **argv)
{
	Py_ssize_t iterator, array_size = (Py_ssize_t)(argc - 1);
	PyObject *list = PyList_New(array_size);

	for (iterator = 0; iterator < array_size; ++iterator)
	{
		const char *arg = argv[iterator + 1];
		PyObject *item = PyUnicode_DecodeFSDefaultAndSize(arg, (Py_ssize_t)strlen(arg));

		if (!(item != NULL && PyList_SetItem(list, iterator, item) == 0))
		{
			goto error_set_item;
		}
	}

	int result = PySys_SetObject("argv", list);

	Py_DECREF(list);

	if (result == -1)
	{
		if (PyErr_Occurred() != NULL)
		{
			py_loader_impl_error_print(py_impl);
			PyErr_Clear();
		}

		return 1;
	}

	return 0;

error_set_item:
	Py_DECREF(list);
	return 1;
}

static void PyCFunction_dealloc(PyObject *obj)
{
	/* Check if we are passing our own hook to the callback */
	if (PyCFunction_Check(obj) && PyCFunction_GET_FUNCTION(obj) == py_loader_impl_function_type_invoke)
	{
		PyObject *error_type, *error_value, *error_traceback;

		/* Save the current exception if any */
		PyErr_Fetch(&error_type, &error_value, &error_traceback);

		PyObject *invoke_state_capsule = PyCFunction_GET_SELF(obj);

		loader_impl_py_function_type_invoke_state invoke_state = PyCapsule_GetPointer(invoke_state_capsule, NULL);

		value_type_destroy(invoke_state->callback);

		Py_DECREF(invoke_state_capsule);

		PyErr_Restore(error_type, error_value, error_traceback);
	}

	/* Call to the original meth_dealloc function */
	py_loader_impl_pycfunction_dealloc(obj);
}

loader_impl_data py_loader_impl_initialize(loader_impl impl, configuration config)
{
	(void)impl;
	(void)config;

	loader_impl_py py_impl = malloc(sizeof(struct loader_impl_py_type));
	int traceback_initialized = 1;
#if DEBUG_ENABLED
	int gc_initialized = 1;
#endif

	if (py_impl == NULL)
	{
		goto error_alloc_py_impl;
	}

	Py_InitializeEx(0);

	if (Py_IsInitialized() == 0)
	{
		goto error_init_py;
	}

#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION <= 6
	if (PyEval_ThreadsInitialized() == 0)
	{
		PyEval_InitThreads();
	}
#endif

	PyThreadState *tstate = PyEval_SaveThread();
	PyGILState_STATE gstate = PyGILState_Ensure();

	/* Hook the deallocation of PyCFunction */
	py_loader_impl_pycfunction_dealloc = PyCFunction_Type.tp_dealloc;
	PyCFunction_Type.tp_dealloc = PyCFunction_dealloc;
	PyType_Modified(&PyCFunction_Type);

	if (py_loader_impl_initialize_sys_executable(py_impl) != 0)
	{
		goto error_after_sys_executable;
	}

	char **argv = metacall_argv();
	int argc = metacall_argc();

	if (argv != NULL && argc > 1)
	{
		if (py_loader_impl_initialize_argv(py_impl, argc, argv) != 0)
		{
			goto error_after_argv;
		}

		py_loader_impl_main_module = argv[1];
		py_loader_impl_run_main = 0;
	}

	if (py_loader_impl_initialize_traceback(impl, py_impl) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid traceback module creation");
	}
	else
	{
		traceback_initialized = 0;
	}

#if DEBUG_ENABLED
	{
		if (py_loader_impl_initialize_gc(py_impl) != 0)
		{
			PyObject_CallMethodObjArgs(py_impl->gc_module, py_impl->gc_set_debug, py_impl->gc_debug_leak /* py_impl->gc_debug_stats */);
			gc_initialized = 0;
		}
		else
		{
			log_write("metacall", LOG_LEVEL_WARNING, "Invalid garbage collector module creation");
		}
	}
#endif

	if (py_loader_impl_initialize_inspect(impl, py_impl) != 0)
	{
		goto error_after_traceback_and_gc;
	}

	if (py_loader_impl_initialize_import(py_impl) != 0)
	{
		goto error_after_inspect;
	}

	if (PY_LOADER_PORT_NAME_FUNC() == NULL)
	{
		goto error_after_import;
	}

	if (py_loader_impl_initialize_thread_background_module(py_impl) != 0)
	{
		goto error_after_import;
	}

	if (py_loader_impl_initialize_asyncio_module(py_impl) != 0)
	{
		goto error_after_thread_background_module;
	}

	/* py_loader_impl_dict_type is derived from PyDict_Type */
	py_loader_impl_dict_type.tp_base = &PyDict_Type;

	if (PyType_Ready(&py_loader_impl_dict_type) < 0)
	{
		goto error_after_asyncio_module;
	}

	PyGILState_Release(gstate);
	PyEval_RestoreThread(tstate);

	/* Register initialization */
	loader_initialization_register(impl);

	log_write("metacall", LOG_LEVEL_DEBUG, "Python loader initialized correctly");

	return py_impl;

error_after_asyncio_module:
	Py_DECREF(py_impl->asyncio_iscoroutinefunction);
	Py_DECREF(py_impl->asyncio_loop);
	Py_DECREF(py_impl->asyncio_module);
	Py_XDECREF(py_impl->py_task_callback_handler);
error_after_thread_background_module:
	Py_DECREF(py_impl->thread_background_module);
	Py_DECREF(py_impl->thread_background_start);
	Py_DECREF(py_impl->thread_background_send);
	Py_DECREF(py_impl->thread_background_stop);
	Py_DECREF(py_impl->thread_background_future_check);
error_after_import:
	Py_DECREF(py_impl->import_module);
	Py_DECREF(py_impl->import_function);
error_after_inspect:
	Py_DECREF(py_impl->inspect_signature);
	Py_DECREF(py_impl->inspect_module);
	Py_DECREF(py_impl->builtins_module);
error_after_traceback_and_gc:
	if (traceback_initialized == 0)
	{
		Py_DECREF(py_impl->traceback_format_exception);
		Py_DECREF(py_impl->traceback_module);
	}
#if DEBUG_ENABLED
	if (gc_initialized == 0)
	{
		Py_DECREF(py_impl->gc_set_debug);
		Py_DECREF(py_impl->gc_debug_leak);
		Py_DECREF(py_impl->gc_debug_stats);
		Py_DECREF(py_impl->gc_module);
	}
#endif
error_after_argv:
error_after_sys_executable:
	PyGILState_Release(gstate);
	PyEval_RestoreThread(tstate);
	(void)py_loader_impl_finalize(py_impl);
error_init_py:
	free(py_impl);
error_alloc_py_impl:
	return NULL;
}

int py_loader_impl_execution_path(loader_impl impl, const loader_path path)
{
	loader_impl_py py_impl = loader_impl_get(impl);

	if (py_impl == NULL)
	{
		return 1;
	}

	int result = 0;
	PyThreadState *tstate = PyEval_SaveThread();
	PyGILState_STATE gstate = PyGILState_Ensure();
	PyObject *system_paths = PySys_GetObject("path");
	PyObject *current_path = PyUnicode_DecodeFSDefault(path);

	for (Py_ssize_t index = 0; index < PyList_Size(system_paths); ++index)
	{
		PyObject *elem = PyList_GetItem(system_paths, index);

		if (PyObject_RichCompareBool(elem, current_path, Py_EQ) == 1)
		{
			goto clear_current_path;
		}
	}

	/* Put the local paths in front of global paths */
	if (PyList_Insert(system_paths, 0, current_path) != 0)
	{
		result = 1;
		py_loader_impl_error_print(py_impl);
		goto clear_current_path;
	}

#if DEBUG_ENABLED
	py_loader_impl_sys_path_print(system_paths);
#endif

clear_current_path:
	Py_DECREF(current_path);
	PyGILState_Release(gstate);
	PyEval_RestoreThread(tstate);
	return result;
}

loader_impl_py_handle py_loader_impl_handle_create(size_t size)
{
	loader_impl_py_handle py_handle = malloc(sizeof(struct loader_impl_py_handle_type));

	if (py_handle == NULL)
	{
		goto error_alloc_handle;
	}

	py_handle->size = size;
	py_handle->modules = malloc(sizeof(struct loader_impl_py_handle_module_type) * size);

	if (py_handle->modules == NULL)
	{
		goto error_alloc_modules;
	}

	for (size_t iterator = 0; iterator < size; ++iterator)
	{
		py_handle->modules[iterator].instance = NULL;
		py_handle->modules[iterator].name = NULL;
	}

	return py_handle;

error_alloc_modules:
	free(py_handle);
error_alloc_handle:
	return NULL;
}

void py_loader_impl_module_destroy(loader_impl_py_handle_module module)
{
	if (module->name != NULL)
	{
		PyObject *system_modules = PySys_GetObject("modules");

		// TODO: Sometimes this fails, seems that sys.modules does not contain the item.
		// Probably is because of the new import system which is using importlib, but
		// it does not seem something problematic although it will be interesting
		// to check it out so we are sure there's no leaked memory
		if (PyObject_HasAttr(system_modules, module->name) == 1)
		{
			PyObject *item = PyObject_GetItem(system_modules, module->name);

			if (item != NULL)
			{
				Py_DECREF(item);
				PyObject_DelItem(system_modules, module->name);
			}
		}

		Py_DECREF(module->name);
		module->name = NULL;
	}

	if (module->instance != NULL)
	{
		Py_DECREF(module->instance);
		module->instance = NULL;
	}
}

void py_loader_impl_handle_destroy(loader_impl_py_handle py_handle)
{
	PyThreadState *tstate = PyEval_SaveThread();
	PyGILState_STATE gstate = PyGILState_Ensure();

	for (size_t iterator = 0; iterator < py_handle->size; ++iterator)
	{
		py_loader_impl_module_destroy(&py_handle->modules[iterator]);
	}

	PyGILState_Release(gstate);
	PyEval_RestoreThread(tstate);
	free(py_handle->modules);
	free(py_handle);
}

int py_loader_impl_load_from_file_path(loader_impl_py py_impl, loader_impl_py_handle_module module, const loader_path path, PyObject **exception, int run_main)
{
	if (run_main == 0)
	{
		static const char name[] = "__main__";
		module->name = PyUnicode_DecodeFSDefaultAndSize(name, (Py_ssize_t)(sizeof(name) - 1));
	}
	else
	{
		loader_name name;
		size_t size = portability_path_get_fullname(path, strnlen(path, LOADER_PATH_SIZE) + 1, name, LOADER_NAME_SIZE);

		*exception = NULL;

		if (size <= 1)
		{
			goto error_name_create;
		}

		module->name = PyUnicode_DecodeFSDefaultAndSize(name, (Py_ssize_t)size - 1);
	}

	if (module->name == NULL)
	{
		goto error_name_create;
	}

	PyObject *py_path = PyUnicode_DecodeFSDefault(path);

	if (py_path == NULL)
	{
		goto error_path_create;
	}

	PyObject *args_tuple = PyTuple_New(2);

	if (args_tuple == NULL)
	{
		goto error_tuple_create;
	}

	PyTuple_SetItem(args_tuple, 0, module->name);
	PyTuple_SetItem(args_tuple, 1, py_path);
	Py_INCREF(module->name);
	Py_INCREF(py_path);

	module->instance = PyObject_Call(py_impl->import_function, args_tuple, NULL);

	if (!(module->instance != NULL && PyModule_Check(module->instance)))
	{
		if (module->instance != NULL && PyErr_GivenExceptionMatches(module->instance, PyExc_Exception))
		{
			*exception = module->instance;
			module->instance = NULL;
		}

		goto error_module_instance;
	}

	Py_DECREF(args_tuple);
	Py_DECREF(py_path);

	return 0;

error_module_instance:
	Py_DECREF(args_tuple);
error_tuple_create:
	Py_DECREF(py_path);
error_path_create:
	Py_XDECREF(module->name);
	module->name = NULL;
error_name_create:
	return 1;
}

int py_loader_impl_load_from_module(loader_impl_py py_impl, loader_impl_py_handle_module module, const loader_path path, PyObject **exception)
{
	size_t length = strlen(path);

	*exception = NULL;

	if (length == 0)
	{
		goto error_name_create;
	}

	module->name = PyUnicode_DecodeFSDefaultAndSize(path, (Py_ssize_t)length);

	if (module->name == NULL)
	{
		goto error_name_create;
	}

	PyObject *args_tuple = PyTuple_New(1);

	if (args_tuple == NULL)
	{
		goto error_tuple_create;
	}

	PyTuple_SetItem(args_tuple, 0, module->name);
	Py_INCREF(module->name);

	module->instance = PyObject_Call(py_impl->import_function, args_tuple, NULL);

	if (!(module->instance != NULL && PyModule_Check(module->instance)))
	{
		if (module->instance != NULL && PyErr_GivenExceptionMatches(module->instance, PyExc_Exception))
		{
			*exception = module->instance;
			module->instance = NULL;
		}

		goto error_module_instance;
	}

	Py_INCREF(module->instance);
	Py_DECREF(args_tuple);

	return 0;

error_module_instance:
	Py_DECREF(args_tuple);
error_tuple_create:
	Py_XDECREF(module->name);
	module->name = NULL;
error_name_create:
	return 1;
}

int py_loader_impl_import_exception(PyObject *exception)
{
	return /*PyErr_GivenExceptionMatches(exception, PyExc_ImportError) ||*/ PyErr_GivenExceptionMatches(exception, PyExc_FileNotFoundError);
}

int py_loader_impl_load_from_file_relative(loader_impl_py py_impl, loader_impl_py_handle_module module, const loader_path path, PyObject **exception, int run_main)
{
	PyObject *system_paths = PySys_GetObject("path");

	for (Py_ssize_t index = 0; index < PyList_Size(system_paths); ++index)
	{
		PyObject *elem = PyList_GetItem(system_paths, index);
		Py_ssize_t length = 0;
		const char *system_path_str = PyUnicode_AsUTF8AndSize(elem, &length);
		loader_path join_path, canonical_path;
		size_t join_path_size = portability_path_join(system_path_str, length + 1, path, strnlen(path, LOADER_PATH_SIZE) + 1, join_path, LOADER_PATH_SIZE);
		portability_path_canonical(join_path, join_path_size, canonical_path, LOADER_PATH_SIZE);

		if (py_loader_impl_load_from_file_path(py_impl, module, canonical_path, exception, run_main) == 0)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "Python Loader relative module loaded at %s", canonical_path);

			return 0;
		}
		else
		{
			/* Stop loading if we found an exception like SyntaxError, continue if the file is not found */
			if (*exception != NULL && !py_loader_impl_import_exception(*exception))
			{
				return 1;
			}
		}

		if (PyErr_Occurred() != NULL)
		{
			PyErr_Clear();
		}
	}

	return 1;
}

static void py_loader_impl_load_from_file_exception(loader_impl_py py_impl, const loader_path path, PyObject *exception)
{
	log_write("metacall", LOG_LEVEL_ERROR, "Python Error: Exception raised while loading the module '%s'", path);

	if (PyErr_Occurred() != NULL)
	{
		PyErr_Clear();
	}

	PyObject *exception_type = PyObject_Type(exception);

	PyErr_SetObject(exception_type, exception);

	Py_DECREF(exception_type);

	py_loader_impl_error_print(py_impl);

	if (PyErr_Occurred() != NULL)
	{
		PyErr_Clear();
	}
}

loader_handle py_loader_impl_load_from_file(loader_impl impl, const loader_path paths[], size_t size)
{
	loader_impl_py py_impl = loader_impl_get(impl);
	loader_impl_py_handle py_handle = py_loader_impl_handle_create(size);
	int run_main = 1;
	size_t iterator;
	PyObject *exception = NULL;

	if (py_handle == NULL)
	{
		goto error_create_handle;
	}

	PyThreadState *tstate = PyEval_SaveThread();
	PyGILState_STATE gstate = PyGILState_Ensure();

	/* Possibly a recursive call */
	if (Py_EnterRecursiveCall(" while loading a module from file in Python Loader") != 0)
	{
		goto error_recursive_call;
	}

	/* If we loaded one script and this script is the same as the one we passed to argv,
	then we should set up this as a main script, only if only we set up the argv in MetaCall.
	This should run only once, the first time after the initialization */
	if (py_loader_impl_run_main == 0 && size == 1 && strcmp(paths[0], py_loader_impl_main_module) == 0)
	{
		run_main = 0;
		py_loader_impl_run_main = 1;
	}

	for (iterator = 0; iterator < size; ++iterator)
	{
		int result = 1;

		/* We assume it is a path so we load from path */
		if (portability_path_is_absolute(paths[iterator], strnlen(paths[iterator], LOADER_PATH_SIZE) + 1) == 0)
		{
			/* Load as absolute path */
			result = py_loader_impl_load_from_file_path(py_impl, &py_handle->modules[iterator], paths[iterator], &exception, run_main);
		}
		else
		{
			/* Try to load it as a path relative to all execution paths */
			result = py_loader_impl_load_from_file_relative(py_impl, &py_handle->modules[iterator], paths[iterator], &exception, run_main);
		}

		/* Stop loading if we found an exception like SyntaxError, continue if the file is not found */
		if (result == 1)
		{
			if (exception != NULL && !py_loader_impl_import_exception(exception))
			{
				py_loader_impl_load_from_file_exception(py_impl, paths[iterator], exception);

				goto error_import_module;
			}
		}

		if (PyErr_Occurred() != NULL)
		{
			PyErr_Clear();
		}

		/* Try to load the module as it is */
		if (result != 0 && py_loader_impl_load_from_module(py_impl, &py_handle->modules[iterator], paths[iterator], &exception) != 0)
		{
			/* Show error message if the module was not found */
			if (exception != NULL)
			{
				if (py_loader_impl_import_exception(exception))
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Python Error: Module '%s' not found", paths[iterator]);
				}
				else
				{
					py_loader_impl_load_from_file_exception(py_impl, paths[iterator], exception);
				}
			}
			else
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Python Error: Module '%s' failed to load without any exception thrown", paths[iterator]);
			}

			goto error_import_module;
		}
	}

	if (PyErr_Occurred() != NULL)
	{
		PyErr_Clear();
	}

	/* End of recursive call */
	Py_LeaveRecursiveCall();

	PyGILState_Release(gstate);
	PyEval_RestoreThread(tstate);

	return (loader_handle)py_handle;

error_import_module:
	if (PyErr_Occurred() != NULL)
	{
		py_loader_impl_error_print(py_impl);
		PyErr_Clear();
	}
	Py_XDECREF(exception);
error_recursive_call:
	PyGILState_Release(gstate);
	PyEval_RestoreThread(tstate);
	py_loader_impl_handle_destroy(py_handle);
error_create_handle:
	return NULL;
}

PyObject *py_loader_impl_load_from_memory_compile(loader_impl_py py_impl, const loader_name name, const char *buffer)
{
	PyObject *compiled = Py_CompileString(buffer, name, Py_file_input);

	if (compiled == NULL)
	{
		py_loader_impl_error_print(py_impl);
		return NULL;
	}

	PyObject *instance = PyImport_ExecCodeModule(name, compiled);

	Py_DECREF(compiled);

	return instance;
}

loader_handle py_loader_impl_load_from_memory(loader_impl impl, const loader_name name, const char *buffer, size_t size)
{
	(void)size;

	loader_impl_py_handle py_handle = py_loader_impl_handle_create(1);

	if (py_handle == NULL)
	{
		goto error_create_handle;
	}

	PyThreadState *tstate = PyEval_SaveThread();
	PyGILState_STATE gstate = PyGILState_Ensure();

	/* Possibly a recursive call */
	if (Py_EnterRecursiveCall(" while loading a module from memory in Python Loader") != 0)
	{
		goto error_import_module;
	}

	loader_impl_py py_impl = loader_impl_get(impl);

	py_handle->modules[0].instance = py_loader_impl_load_from_memory_compile(py_impl, name, buffer);

	if (py_handle->modules[0].instance == NULL)
	{
		py_loader_impl_error_print(py_impl);
		goto error_import_module;
	}

	py_handle->modules[0].name = PyUnicode_DecodeFSDefault(name);

	/* End of recursive call */
	Py_LeaveRecursiveCall();

	PyGILState_Release(gstate);
	PyEval_RestoreThread(tstate);

	log_write("metacall", LOG_LEVEL_DEBUG, "Python loader (%p) importing %s from memory module at (%p)", (void *)impl, name, (void *)py_handle->modules[0].instance);

	return (loader_handle)py_handle;

error_import_module:
	if (PyErr_Occurred() != NULL)
	{
		PyErr_Clear();
	}
	PyGILState_Release(gstate);
	PyEval_RestoreThread(tstate);
	py_loader_impl_handle_destroy(py_handle);
error_create_handle:
	return NULL;
}

loader_handle py_loader_impl_load_from_package(loader_impl impl, const loader_path path)
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

type py_loader_impl_discover_type(loader_impl impl, PyObject *annotation, const char *func_name, const char *parameter_name)
{
	type t = NULL;

	if (annotation == NULL)
	{
		return NULL;
	}

	static const char qualname[] = "__qualname__";

	if (PyObject_HasAttrString(annotation, qualname) == 0)
	{
		if (parameter_name != NULL)
		{
			log_write("metacall", LOG_LEVEL_WARNING, "Invalid annotation type in the parameter '%s' of the function %s", parameter_name, func_name);
		}
		else
		{
			log_write("metacall", LOG_LEVEL_WARNING, "Invalid annotation type in the return type of the function %s", func_name);
		}

		return NULL;
	}

	PyObject *annotation_qualname = PyObject_GetAttrString(annotation, qualname);
	const char *annotation_name = PyUnicode_AsUTF8(annotation_qualname);

	if (annotation_qualname != NULL)
	{
		if (strcmp(annotation_name, "_empty") != 0)
		{
			t = loader_impl_type(impl, annotation_name);

			log_write("metacall", LOG_LEVEL_DEBUG, "Discover type (%p) (%p): %s", (void *)annotation, (void *)type_derived(t), annotation_name);
		}

		Py_DECREF(annotation_qualname);
	}

	return t;
}

size_t py_loader_impl_discover_callable_args_count(loader_impl_py py_impl, PyObject *callable)
{
	/* TODO: Improve this in the future, adding positional arguments, variable arguments and others */
	size_t args_count = 0;
	PyObject *spec = PyObject_CallFunction(py_impl->inspect_getfullargspec, "O", callable);

	if (spec == NULL)
	{
		/* This means we have a PyCFunction which cannot be introspected, we let varargs to do the calls */
		/* TODO: In the future we should check if this is a callback from metacall because then we can obtain the function signature */
		PyErr_Clear();
		goto unsupported_callable;
	}

	PyObject *args = PyTuple_GetItem(spec, 0);

	if (args == NULL)
	{
		/* If there's no arguments, let's Python deal with this as variable arguments */
		goto clear_spec;
	}

	/* Get the number of arguments */
	args_count = (size_t)PyObject_Size(args);

	PyObject *is_method = PyObject_CallFunction(py_impl->inspect_ismethod, "O", callable);

	if (is_method == NULL)
	{
		goto clear_spec;
	}

	/* Do not count self parameter */
	if (PyObject_IsTrue(is_method) == 1)
	{
		args_count--;
	}

	Py_DECREF(is_method);
clear_spec:
	Py_DECREF(spec); /* The elements from the tuple (args) are cleaned here */
unsupported_callable:
	return args_count;
}

int py_loader_impl_discover_func(loader_impl impl, PyObject *func, function f)
{
	loader_impl_py py_impl = loader_impl_get(impl);
	PyObject *args = PyTuple_New(1);

	if (args == NULL)
	{
		py_loader_impl_error_print(py_impl);
		return 1;
	}

	PyTuple_SetItem(args, 0, func);
	Py_INCREF(func);
	PyObject *result = PyObject_CallObject(py_impl->inspect_signature, args);
	Py_DECREF(args);

	if (result != NULL)
	{
		signature s = function_signature(f);
		const char *func_name = function_name(f);
		PyObject *parameters = PyObject_GetAttrString(result, "parameters");
		PyObject *return_annotation = PyObject_GetAttrString(result, "return_annotation");

		if (parameters != NULL && PyMapping_Check(parameters))
		{
			PyObject *parameter_list = PyMapping_Values(parameters);

			if (parameter_list != NULL && PyList_Check(parameter_list))
			{
				Py_ssize_t parameter_list_size = PyMapping_Size(parameters);
				size_t args_count = signature_count(s);

				if ((size_t)parameter_list_size != args_count)
				{
					/* TODO: Implement properly variable arguments with inspection of the names */
					/* co_argcount in py_loader_impl_discover_callable_args_count returns the number */
					/* of arguments (not including keyword only arguments, * or ** args), so they */
					/* won't be inspected but the variable call can be done with metacall*_s API */
					parameter_list_size = (Py_ssize_t)args_count;
				}

				for (Py_ssize_t iterator = 0; iterator < parameter_list_size; ++iterator)
				{
					PyObject *parameter = PyList_GetItem(parameter_list, iterator);

					if (parameter == NULL)
					{
						continue;
					}

					PyObject *name = PyObject_GetAttrString(parameter, "name");
					const char *parameter_name = PyUnicode_AsUTF8(name);
					PyObject *annotation = PyObject_GetAttrString(parameter, "annotation");
					type t = py_loader_impl_discover_type(impl, annotation, func_name, parameter_name);
					signature_set(s, iterator, parameter_name, t);
					Py_XDECREF(name);
					Py_XDECREF(annotation);
				}
			}

			Py_XDECREF(parameter_list);
		}

		Py_XDECREF(parameters);

		function_async(f, py_loader_impl_check_async(py_impl, func) == 1 ? ASYNCHRONOUS : SYNCHRONOUS);

		signature_set_return(s, py_loader_impl_discover_type(impl, return_annotation, func_name, NULL));

		Py_DECREF(return_annotation);

		return 0;
	}
	else
	{
		/* We are trying to inspect a non callable object or inspect.signature does not support the type */
		PyErr_Clear();

		if (PyCFunction_Check(func))
		{
			signature s = function_signature(f);

			signature_set_return(s, NULL);

			return 0;
		}
	}

	return 1;
}

int py_loader_impl_discover_method(loader_impl impl, PyObject *callable, method m, bool is_static)
{
	loader_impl_py py_impl = loader_impl_get(impl);
	PyObject *args = PyTuple_New(1);

	if (args == NULL)
	{
		py_loader_impl_error_print(py_impl);
		return 1;
	}

	if (is_static)
	{
		PyObject *func = PyObject_GetAttrString(callable, "__func__");
		PyTuple_SetItem(args, 0, func);
	}
	else
	{
		PyTuple_SetItem(args, 0, callable);
		Py_INCREF(callable);
	}
	PyObject *result = PyObject_CallObject(py_impl->inspect_signature, args);
	Py_DECREF(args);

	if (result != NULL)
	{
		signature s = method_signature(m);
		const char *m_name = method_name(m);
		PyObject *parameters = PyObject_GetAttrString(result, "parameters");
		PyObject *return_annotation = PyObject_GetAttrString(result, "return_annotation");

		if (parameters != NULL && PyMapping_Check(parameters))
		{
			PyObject *parameter_list = PyMapping_Values(parameters);

			if (parameter_list != NULL && PyList_Check(parameter_list))
			{
				Py_ssize_t parameter_list_size = PyMapping_Size(parameters);
				size_t args_count = signature_count(s);

				if ((size_t)parameter_list_size != args_count)
				{
					/* TODO: Implement properly variable arguments with inspection of the names */
					/* co_argcount in py_loader_impl_discover_callable_args_count returns the number */
					/* of arguments (not including keyword only arguments, * or ** args), so they */
					/* won't be inspected but the variable call can be done with metacall*_s API */
					parameter_list_size = (Py_ssize_t)args_count;
				}

				for (Py_ssize_t iterator = 0; iterator < parameter_list_size; ++iterator)
				{
					PyObject *parameter = PyList_GetItem(parameter_list, iterator);

					if (parameter == NULL)
					{
						continue;
					}

					PyObject *name = PyObject_GetAttrString(parameter, "name");
					const char *parameter_name = PyUnicode_AsUTF8(name);
					PyObject *annotation = PyObject_GetAttrString(parameter, "annotation");
					type t = py_loader_impl_discover_type(impl, annotation, m_name, parameter_name);
					signature_set(s, iterator, parameter_name, t);
					Py_XDECREF(name);
					Py_XDECREF(annotation);
				}
			}

			Py_XDECREF(parameter_list);
		}

		Py_XDECREF(parameters);

		signature_set_return(s, py_loader_impl_discover_type(impl, return_annotation, m_name, NULL));

		Py_DECREF(return_annotation);

		return 0;
	}
	else
	{
		/* We are trying to inspect a non callable object or inspect.signature does not support the type */
		PyErr_Clear();

		if (PyCFunction_Check(callable))
		{
			signature s = method_signature(m);

			signature_set_return(s, NULL);

			return 0;
		}
	}

	return 1;
}

type py_loader_impl_get_type(loader_impl impl, PyObject *obj)
{
	type t = NULL;
	PyObject *builtin = PyObject_Type(obj); /* Increments reference count of the type */

	if (!(builtin != NULL && PyType_Check(builtin)))
	{
		goto builtin_error;
	}

	PyObject *t_name = PyObject_GetAttrString(builtin, "__name__");

	if (!(t_name != NULL && PyUnicode_Check(t_name)))
	{
		goto type_name_error;
	}

	const char *name = PyUnicode_AsUTF8(t_name);
	t = loader_impl_type(impl, name);

	if (t == NULL)
	{
		t = type_create(py_loader_impl_capi_to_value_type(impl, obj), name, builtin, &type_py_singleton);

		if (t == NULL)
		{
			goto type_name_error;
		}

		if (loader_impl_type_define(impl, type_name(t), t) != 0)
		{
			type_destroy(t); // This already clears the builtin reference in the destroy of the signleton
			t = NULL;
		}

		Py_DECREF(t_name);
		return t;
	}

type_name_error:
	Py_XDECREF(t_name);
builtin_error:
	Py_XDECREF(builtin);
	return t;
}

int py_loader_impl_discover_constructor(loader_impl impl, PyObject *py_class, klass c)
{
	int ret = 0;

	/* Inspect the constructor */
	if (!PyObject_HasAttrString(py_class, "__init__"))
	{
		return 1;
	}

	loader_impl_py py_impl = loader_impl_get(impl);
	PyObject *args = PyTuple_New(1);

	if (args == NULL)
	{
		py_loader_impl_error_print(py_impl);
		return 1;
	}

	PyObject *name_init = PyUnicode_FromString("__init__");
	PyObject *init_method = PyObject_GenericGetAttr(py_class, name_init);
	Py_DECREF(name_init);

	PyTuple_SetItem(args, 0, init_method);

	PyObject *result = PyObject_CallObject(py_impl->inspect_signature, args);
	Py_DECREF(args); /* Clears init_method reference */

	if (result == NULL)
	{
		py_loader_impl_error_print(py_impl);
		return 1;
	}

	PyObject *parameters = PyObject_GetAttrString(result, "parameters");

	if (parameters != NULL)
	{
		PyObject *parameter_list = PyMapping_Values(parameters);

		if (parameter_list != NULL && PyList_Check(parameter_list))
		{
			Py_ssize_t parameter_list_size = PyMapping_Size(parameters);
			constructor ctor = constructor_create(parameter_list_size > 0 ? (size_t)parameter_list_size - 1 : 0, VISIBILITY_PUBLIC);
			size_t parameter_count = 0;

			/* Start at 1 because we do not count the 'self' parameter */
			for (Py_ssize_t iterator = 1; iterator < parameter_list_size; ++iterator)
			{
				PyObject *parameter = PyList_GetItem(parameter_list, iterator);

				if (parameter == NULL)
				{
					continue;
				}

				PyObject *name = PyObject_GetAttrString(parameter, "name");
				const char *parameter_name = PyUnicode_AsUTF8(name);
				PyObject *annotation = PyObject_GetAttrString(parameter, "annotation");
				type t = py_loader_impl_discover_type(impl, annotation, "__init__", parameter_name);
				constructor_set(ctor, parameter_count++, parameter_name, t);
				Py_XDECREF(name);
				Py_XDECREF(annotation);
			}

			ret = class_register_constructor(c, ctor);

			if (ret != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Failed to register constructor in class %s", class_name(c));
			}
		}

		Py_XDECREF(parameter_list);
	}

	Py_XDECREF(parameters);

	return ret;
}

int py_loader_impl_discover_class(loader_impl impl, PyObject *py_class, klass c)
{
	loader_impl_py py_impl = loader_impl_get(impl);

	/* Inspect the constructor */
	if (py_loader_impl_discover_constructor(impl, py_class, c) != 0)
	{
		constructor ctor = constructor_create(0, VISIBILITY_PUBLIC);

		if (class_register_constructor(c, ctor) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Failed to register default constructor in class %s", class_name(c));
		}
	}

	if (PyObject_HasAttrString(py_class, "__dict__"))
	{
		PyObject *nameobj = PyUnicode_FromString("__dict__");
		PyObject *read_only_dict = PyObject_GenericGetAttr((PyObject *)py_class, nameobj);
		Py_DECREF(nameobj);

		/* Turns out __dict__ is not a PyDict but PyMapping */
		if (!PyObject_TypeCheck(read_only_dict, &PyDictProxy_Type))
		{
			Py_XDECREF(read_only_dict);
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

			type_id member_type = py_loader_impl_capi_to_value_type(impl, tuple_val);

			// Skip None types
			if (member_type == TYPE_NULL)
			{
				continue;
			}

			PyObject *args = PyTuple_New(2);

			if (args == NULL)
			{
				py_loader_impl_error_print(py_impl);
				continue;
			}

			PyTuple_SetItem(args, 0, py_class); // class
			Py_INCREF(py_class);
			PyTuple_SetItem(args, 1, tuple_key); // method
			Py_INCREF(tuple_key);
			PyObject *method_static = PyObject_CallObject(py_impl->inspect_getattr_static, args);
			Py_DECREF(args);
			bool is_static_method = PyObject_TypeCheck(method_static, &PyStaticMethod_Type);

			log_write("metacall", LOG_LEVEL_DEBUG, "Introspection: class member %s, type %s, static method: %d",
				PyUnicode_AsUTF8(tuple_key),
				type_id_name(py_loader_impl_capi_to_value_type(impl, tuple_val)),
				is_static_method);

			if (member_type == TYPE_FUNCTION || is_static_method)
			{
				enum async_id func_synchronicity = SYNCHRONOUS;
				size_t args_count;

				if (is_static_method)
				{
					PyObject *func = PyObject_GetAttrString(tuple_val, "__func__");
					args_count = py_loader_impl_discover_callable_args_count(py_impl, func);
					Py_DECREF(func);
				}
				else
				{
					args_count = py_loader_impl_discover_callable_args_count(py_impl, tuple_val);
				}

				if (py_loader_impl_check_async(py_impl, tuple_val) == 1)
				{
					func_synchronicity = ASYNCHRONOUS;
				}

				method m = method_create(c,
					PyUnicode_AsUTF8(tuple_key),
					args_count,
					NULL,			   /* There's no need to pass the method implementation (tuple_val) here,
					* it is used only for introspection, not for invoking it, that's done by name */
					VISIBILITY_PUBLIC, /* TODO: check @property decorator for protected access? */
					func_synchronicity,
					NULL);

				if (is_static_method)
				{
					class_register_static_method(c, m);
				}
				else
				{
					class_register_method(c, m);
				}

				if (py_loader_impl_discover_method(impl, tuple_val, m, is_static_method) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Failed to discover method %s from class %s",
						PyUnicode_AsUTF8(tuple_key),
						class_name(c));
				}

				/* Delete the reference of the method here instead of in py_method_interface_destroy */
				Py_XDECREF(tuple_val);
			}
			else
			{
				const char *name = PyUnicode_AsUTF8(tuple_key);
				type t = py_loader_impl_get_type(impl, tuple_val);

				if (t == NULL)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Invalid type discovering in class method: %s", name);
					continue;
				}

				attribute static_attr = attribute_create(c, name, t, NULL, VISIBILITY_PUBLIC, NULL);
				attribute attr = attribute_create(c, name, t, NULL, VISIBILITY_PUBLIC, NULL);

				/* In Python all attributes can work as static or non-static, so we register both */
				class_register_static_attribute(c, static_attr);
				class_register_attribute(c, attr);
			}
		}

		Py_XDECREF(dict_items);
		Py_XDECREF(read_only_dict);
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
	int ret = 1;
	PyThreadState *tstate = PyEval_SaveThread();
	PyGILState_STATE gstate = PyGILState_Ensure();

	if (module == NULL || !PyModule_Check(module))
	{
		goto cleanup;
	}

	// This should never fail since `module` is a valid module object
	PyObject *module_dict = PyModule_GetDict(module);
	Py_ssize_t position = 0;
	PyObject *module_dict_key, *module_dict_val;
	loader_impl_py py_impl = loader_impl_get(impl);

	while (PyDict_Next(module_dict, &position, &module_dict_key, &module_dict_val))
	{
		// Class is also PyCallable, so test for class first
		if (PyObject_TypeCheck(module_dict_val, &PyType_Type))
		// PyObject_IsSubclass(module_dict_val, (PyObject *)&PyType_Type) == 0
		{
			const char *cls_name = PyUnicode_AsUTF8(module_dict_key);

			log_write("metacall", LOG_LEVEL_DEBUG, "Introspection: class name %s", cls_name);

			loader_impl_py_class py_cls = malloc(sizeof(struct loader_impl_py_class_type));

			Py_INCREF(module_dict_val);

			klass c = class_create(cls_name, ACCESSOR_TYPE_STATIC, py_cls, &py_class_interface_singleton);

			py_cls->impl = impl;
			py_cls->cls = module_dict_val;

			if (py_loader_impl_discover_class(impl, module_dict_val, c) == 0)
			{
				scope sp = context_scope(ctx);
				value v = value_create_class(c);
				if (scope_define(sp, cls_name, v) != 0)
				{
					value_type_destroy(v);
					goto cleanup;
				}
			}
			else
			{
				class_destroy(c);
				goto cleanup;
			}
		}
		else if (PyCallable_Check(module_dict_val))
		{
			const char *func_name = PyUnicode_AsUTF8(module_dict_key);
			size_t discover_args_count = py_loader_impl_discover_callable_args_count(py_impl, module_dict_val);
			loader_impl_py_function py_func = malloc(sizeof(struct loader_impl_py_function_type));

			if (py_func == NULL)
			{
				goto cleanup;
			}

			Py_INCREF(module_dict_val);
			py_func->func = module_dict_val;
			py_func->impl = impl;

			function f = function_create(func_name, discover_args_count, py_func, &function_py_singleton);

			log_write("metacall", LOG_LEVEL_DEBUG, "Introspection: function %s, args count %" PRIuS, func_name, discover_args_count);

			if (py_loader_impl_discover_func(impl, module_dict_val, f) == 0)
			{
				scope sp = context_scope(ctx);
				value v = value_create_function(f);
				if (scope_define(sp, func_name, v) != 0)
				{
					value_type_destroy(v);
					goto cleanup;
				}
			}
			else
			{
				function_destroy(f);
				goto cleanup;
			}
		}
	}

	ret = 0;

cleanup:
	PyGILState_Release(gstate);
	PyEval_RestoreThread(tstate);
	return ret;
}

int py_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_py_handle py_handle = (loader_impl_py_handle)handle;

	for (size_t iterator = 0; iterator < py_handle->size; ++iterator)
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
	static const char error_format_str[] = "Python %s: %s\n%s";
	static const char separator_str[] = "";
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

value py_loader_impl_error_value(loader_impl_py py_impl)
{
	PyObject *type_obj, *value_obj, *traceback_obj;

	PyErr_Fetch(&type_obj, &value_obj, &traceback_obj);

	value v = py_loader_impl_error_value_from_exception(py_impl, type_obj, value_obj, traceback_obj);

	PyErr_Clear();

	return v;
}

value py_loader_impl_error_value_from_exception(loader_impl_py py_impl, PyObject *type_obj, PyObject *value_obj, PyObject *traceback_obj)
{
	static const char separator_str[] = "";
	static const char traceback_not_found[] = "Traceback not available";
	PyObject *value_str_obj, *traceback_str_obj;
	PyObject *traceback_list, *separator;
	const char *type_str, *value_str, *traceback_str;
	value ret = NULL;

	value_str_obj = PyObject_Str(value_obj);

	traceback_list = PyObject_CallFunctionObjArgs(py_impl->traceback_format_exception, type_obj, value_obj, traceback_obj, NULL);

#if PY_MAJOR_VERSION == 2
	separator = PyString_FromString(separator_str);

	traceback_str_obj = PyString_Join(separator, traceback_list);

	value_str = PyString_AsString(value_str_obj);
	traceback_str = traceback_str_obj ? PyString_AsString(traceback_str_obj) : NULL;
#elif PY_MAJOR_VERSION == 3
	separator = PyUnicode_FromString(separator_str);

	traceback_str_obj = PyUnicode_Join(separator, traceback_list);

	type_str = PyExceptionClass_Name(type_obj);
	value_str = PyUnicode_AsUTF8(value_str_obj);
	traceback_str = traceback_str_obj ? PyUnicode_AsUTF8(traceback_str_obj) : NULL;
#endif

	exception ex = exception_create_const(value_str, type_str, 0, traceback_str ? traceback_str : traceback_not_found);

	throwable th = throwable_create(value_create_exception(ex));

	ret = value_create_throwable(th);

	Py_XDECREF(traceback_list);
	Py_DECREF(separator);
	Py_XDECREF(traceback_str_obj);

	return ret;
}

#if DEBUG_ENABLED
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

	Py_XDECREF(sys_path_str_obj);
	Py_XDECREF(separator);
}
#endif

int py_loader_impl_finalize(loader_impl_py py_impl)
{
	if (Py_IsInitialized() != 0)
	{
		if (PyErr_Occurred() != NULL)
		{
			py_loader_impl_error_print(py_impl);
		}

#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 6
		{
			if (Py_FinalizeEx() != 0)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Error when executing Py_FinalizeEx");
				return 1;
			}
		}
#else
		{
			Py_Finalize();
		}
#endif
	}

	return 0;
}

int py_loader_impl_destroy(loader_impl impl)
{
	loader_impl_py py_impl = loader_impl_get(impl);

	if (py_impl == NULL)
	{
		return 1;
	}

	/* Destroy children loaders */
	loader_unload_children(impl);

	PyThreadState *tstate = PyEval_SaveThread();
	PyGILState_STATE gstate = PyGILState_Ensure();

	/* Stop event loop for async calls */
	PyObject *args_tuple = PyTuple_New(1);
	Py_INCREF(py_impl->asyncio_loop);
	PyTuple_SetItem(args_tuple, 0, py_impl->asyncio_loop);
	PyObject_Call(py_impl->thread_background_stop, args_tuple, NULL);
	Py_XDECREF(args_tuple);

	if (PyErr_Occurred() != NULL)
	{
		py_loader_impl_error_print(py_impl);
	}

	Py_DECREF(py_impl->inspect_signature);
	Py_DECREF(py_impl->inspect_getattr_static);
	Py_DECREF(py_impl->inspect_getfullargspec);
	Py_DECREF(py_impl->inspect_ismethod);
	Py_DECREF(py_impl->inspect_isclass);
	Py_DECREF(py_impl->inspect_module);
	Py_DECREF(py_impl->builtins_module);
	Py_DECREF(py_impl->traceback_format_exception);
	Py_DECREF(py_impl->traceback_module);
	Py_DECREF(py_impl->import_function);
	Py_DECREF(py_impl->import_module);

	Py_XDECREF(py_impl->asyncio_iscoroutinefunction);
	Py_XDECREF(py_impl->asyncio_loop);
	Py_XDECREF(py_impl->asyncio_module);
	Py_XDECREF(py_impl->py_task_callback_handler);
	Py_XDECREF(py_impl->thread_background_future_check);
	Py_XDECREF(py_impl->thread_background_module);
	Py_XDECREF(py_impl->thread_background_start);
	Py_XDECREF(py_impl->thread_background_send);
	Py_XDECREF(py_impl->thread_background_stop);

#if DEBUG_ENABLED
	{
		py_loader_impl_gc_print(py_impl);
		Py_DECREF(py_impl->gc_set_debug);
		Py_DECREF(py_impl->gc_debug_leak);
		Py_DECREF(py_impl->gc_debug_stats);
		Py_DECREF(py_impl->gc_module);
	}
#endif

	PyGILState_Release(gstate);
	PyEval_RestoreThread(tstate);

	int result = py_loader_impl_finalize(py_impl);

	/* Unhook the deallocation of PyCFunction */
	PyCFunction_Type.tp_dealloc = py_loader_impl_pycfunction_dealloc;

	free(py_impl);

	return result;
}
