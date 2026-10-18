/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <py_loader/py_loader_class.h>
#include <py_loader/py_loader_object.h>
#include <py_loader/py_loader_symbol_fallback.h>
#include <py_loader/py_loader_threading.h>

#include <reflect/reflect_class.h>
#include <reflect/reflect_object.h>

#include <adt/adt_set.h>

#include <log/log.h>

#include <metacall/metacall.h>

#include <stddef.h>

/* Arguments up to this size are converted into the stack, avoiding the allocation in each call */
#define PY_LOADER_IMPL_OBJECT_ARGS_STACK_SIZE 16

/* Base of all the generated class types, each instance wraps a MetaCall object */
struct py_loader_impl_object_type
{
	PyObject_HEAD
		loader_impl impl;
	value obj;
};

/* Data descriptor that forwards an attribute of the instance to the MetaCall object */
struct py_loader_impl_object_attribute_type
{
	PyObject_HEAD
		PyObject *name;
};

/* Method descriptor that forwards a method call of the instance to the MetaCall object */
struct py_loader_impl_object_method_type
{
	PyObject_HEAD
		PyObject *name;
#if PY_LOADER_IMPL_VECTORCALL
	vectorcallfunc vectorcall;
#endif
};

/* Cache of wrappers indexed by object, it holds borrowed references, each wrapper removes itself on dealloc */
static set py_loader_impl_object_cache = NULL;

PyObject *py_loader_impl_method_invoke(loader_impl impl, void *target, PyObject *name, py_loader_impl_reflect_invoke_ptr invoke, PyObject *const *args, size_t argc)
{
	const char *key = PyUnicode_AsUTF8(name);

	if (key == NULL)
	{
		return NULL;
	}

	void *stack_args[PY_LOADER_IMPL_OBJECT_ARGS_STACK_SIZE];
	void **value_args = argc <= PY_LOADER_IMPL_OBJECT_ARGS_STACK_SIZE ? stack_args : malloc(sizeof(void *) * argc);

	if (value_args == NULL)
	{
		PyErr_Format(PyExc_ValueErrorPtr(), "Invalid allocation of arguments for the method '%s'", key);
		return NULL;
	}

	/* Generate metacall values from python values */
	for (size_t args_count = 0; args_count < argc; ++args_count)
	{
		type_id id = py_loader_impl_capi_to_value_type(impl, args[args_count]);
		value_args[args_count] = py_loader_impl_capi_to_value(impl, args[args_count], id);
	}

	py_loader_thread_release();

	/* Execute the method */
	value ret = (value)invoke(target, key, value_args, argc);

	/* Destroy argument values */
	for (size_t args_count = 0; args_count < argc; ++args_count)
	{
		value_type_destroy(value_args[args_count]);
	}

	py_loader_thread_acquire();

	if (value_args != stack_args)
	{
		free(value_args);
	}

	if (ret == NULL)
	{
		return Py_ReturnNone();
	}

	/* Transform the return value into a python value */
	PyObject *py_ret = py_loader_impl_value_to_capi(impl, value_type_id(ret), ret);

	py_loader_thread_release();
	value_type_destroy(ret);
	py_loader_thread_acquire();

	return py_ret;
}

static void py_loader_impl_object_dealloc(PyObject *self)
{
	struct py_loader_impl_object_type *wrapped = (struct py_loader_impl_object_type *)self;

	if (wrapped->obj != NULL)
	{
		if (py_loader_impl_object_cache != NULL)
		{
			set_remove(py_loader_impl_object_cache, (set_key)value_to_object(wrapped->obj));
		}

		/* Clean the MetaCall object value */
		metacall_value_destroy(wrapped->obj);
		wrapped->obj = NULL;
	}

	/* Free the memory for the wrapper object itself, the reference to the heap type is released by the subtype */
	Py_TYPE(self)->tp_free(self);
}

static PyTypeObject py_loader_impl_object_type = {
	PyVarObject_HEAD_INIT(NULL, 0) "PyCObjectWrapper",
	sizeof(struct py_loader_impl_object_type),
	0,
	(destructor)py_loader_impl_object_dealloc, /* tp_dealloc */
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
	PyDoc_STR("MetaCall object wrapper"),	   /* tp_doc */
	0,										   /* tp_traverse */
	0,										   /* tp_clear */
	0,										   /* tp_richcompare */
	0,										   /* tp_weaklistoffset */
	0,										   /* tp_iter */
	0,										   /* tp_iternext */
	0,										   /* tp_methods */
	0,										   /* tp_members */
	0,										   /* tp_getset */
	0,										   /* tp_base */
	0,										   /* tp_dict */
	0,										   /* tp_descr_get */
	0,										   /* tp_descr_set */
	0,										   /* tp_dictoffset */
	0,										   /* tp_init */
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
	0, /* tp_watched */
#endif
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 13
	0, /* tp_versions_used */
#endif
};

static void py_loader_impl_object_attribute_dealloc(PyObject *self)
{
	struct py_loader_impl_object_attribute_type *descriptor = (struct py_loader_impl_object_attribute_type *)self;

	Py_DecRef(descriptor->name);

	Py_TYPE(self)->tp_free(self);
}

static PyObject *py_loader_impl_object_attribute_get(PyObject *self, PyObject *instance, PyObject *owner)
{
	struct py_loader_impl_object_attribute_type *descriptor = (struct py_loader_impl_object_attribute_type *)self;

	(void)owner;

	/* Accessed from the class, return the descriptor itself */
	if (instance == NULL)
	{
		Py_IncRef(self);
		return self;
	}

	if (!py_loader_impl_object_check(instance))
	{
		PyErr_SetString(PyExc_TypeErrorPtr(), "The attribute descriptor requires a MetaCall object");
		return NULL;
	}

	struct py_loader_impl_object_type *wrapped = (struct py_loader_impl_object_type *)instance;
	const char *key = PyUnicode_AsUTF8(descriptor->name);

	if (key == NULL)
	{
		return NULL;
	}

	py_loader_thread_release();

	value v = object_get(value_to_object(wrapped->obj), key);

	py_loader_thread_acquire();

	if (v == NULL)
	{
		PyErr_Format(PyExc_AttributeErrorPtr(), "Failed to get the attribute '%s' of the MetaCall object", key);
		return NULL;
	}

	PyObject *result = py_loader_impl_value_to_capi(wrapped->impl, value_type_id(v), v);

	py_loader_thread_release();
	value_type_destroy(v);
	py_loader_thread_acquire();

	return result;
}

static int py_loader_impl_object_attribute_set(PyObject *self, PyObject *instance, PyObject *v_obj)
{
	struct py_loader_impl_object_attribute_type *descriptor = (struct py_loader_impl_object_attribute_type *)self;

	if (!py_loader_impl_object_check(instance))
	{
		PyErr_SetString(PyExc_TypeErrorPtr(), "The attribute descriptor requires a MetaCall object");
		return -1;
	}

	struct py_loader_impl_object_type *wrapped = (struct py_loader_impl_object_type *)instance;
	const char *key = PyUnicode_AsUTF8(descriptor->name);

	if (key == NULL)
	{
		return -1;
	}

	if (v_obj == NULL)
	{
		PyErr_Format(PyExc_AttributeErrorPtr(), "Cannot delete the attribute '%s' of the MetaCall object", key);
		return -1;
	}

	type_id id = py_loader_impl_capi_to_value_type(wrapped->impl, v_obj);
	value v = py_loader_impl_capi_to_value(wrapped->impl, v_obj, id);

	if (v == NULL)
	{
		PyErr_Format(PyExc_TypeErrorPtr(), "Invalid value for the attribute '%s' of the MetaCall object", key);
		return -1;
	}

	py_loader_thread_release();

	int result = object_set(value_to_object(wrapped->obj), key, v);

	value_type_destroy(v);

	py_loader_thread_acquire();

	if (result != 0)
	{
		PyErr_Format(PyExc_AttributeErrorPtr(), "Failed to set the attribute '%s' of the MetaCall object", key);
		return -1;
	}

	return 0;
}

static PyTypeObject py_loader_impl_object_attribute_type = {
	PyVarObject_HEAD_INIT(NULL, 0) "PyCAttributeDescriptor",
	sizeof(struct py_loader_impl_object_attribute_type),
	0,
	(destructor)py_loader_impl_object_attribute_dealloc, /* tp_dealloc */
	0,													 /* tp_vectorcall_offset */
	0,													 /* tp_getattr */
	0,													 /* tp_setattr */
	0,													 /* tp_as_async */
	0,													 /* tp_repr */
	0,													 /* tp_as_number */
	0,													 /* tp_as_sequence */
	0,													 /* tp_as_mapping */
	0,													 /* tp_hash */
	0,													 /* tp_call */
	0,													 /* tp_str */
	0,													 /* tp_getattro */
	0,													 /* tp_setattro */
	0,													 /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,									 /* tp_flags */
	PyDoc_STR("MetaCall object attribute descriptor"),	 /* tp_doc */
	0,													 /* tp_traverse */
	0,													 /* tp_clear */
	0,													 /* tp_richcompare */
	0,													 /* tp_weaklistoffset */
	0,													 /* tp_iter */
	0,													 /* tp_iternext */
	0,													 /* tp_methods */
	0,													 /* tp_members */
	0,													 /* tp_getset */
	0,													 /* tp_base */
	0,													 /* tp_dict */
	(descrgetfunc)py_loader_impl_object_attribute_get,	 /* tp_descr_get */
	(descrsetfunc)py_loader_impl_object_attribute_set,	 /* tp_descr_set */
	0,													 /* tp_dictoffset */
	0,													 /* tp_init */
	0,													 /* tp_alloc */
	0,													 /* tp_new */
	0,													 /* tp_free */
	0,													 /* tp_is_gc */
	0,													 /* tp_bases */
	0,													 /* tp_mro */
	0,													 /* tp_cache */
	0,													 /* tp_subclasses */
	0,													 /* tp_weaklist */
	0,													 /* tp_del */
	0,													 /* tp_version_tag */
	0,													 /* tp_finalize */
	0,													 /* tp_vectorcall */
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 12
	0, /* tp_watched */
#endif
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 13
	0, /* tp_versions_used */
#endif
};

static void py_loader_impl_object_method_dealloc(PyObject *self)
{
	struct py_loader_impl_object_method_type *descriptor = (struct py_loader_impl_object_method_type *)self;

	Py_DecRef(descriptor->name);

	Py_TYPE(self)->tp_free(self);
}

static PyObject *py_loader_impl_object_method_invoke(PyObject *self, PyObject *const *args, size_t argc)
{
	struct py_loader_impl_object_method_type *descriptor = (struct py_loader_impl_object_method_type *)self;

	/* The first argument is the instance, it is passed by the method call optimization or the bound method */
	if (argc == 0 || !py_loader_impl_object_check(args[0]))
	{
		PyErr_SetString(PyExc_TypeErrorPtr(), "The method descriptor requires a MetaCall object as first argument");
		return NULL;
	}

	struct py_loader_impl_object_type *wrapped = (struct py_loader_impl_object_type *)args[0];

	return py_loader_impl_method_invoke(wrapped->impl, value_to_object(wrapped->obj), descriptor->name, (py_loader_impl_reflect_invoke_ptr)&metacallv_object, &args[1], argc - 1);
}

static PyObject *py_loader_impl_object_method_call(PyObject *self, PyObject *args, PyObject *kwds)
{
	if (kwds != NULL && PyDict_Size(kwds) > 0)
	{
		PyErr_SetString(PyExc_TypeErrorPtr(), "MetaCall methods do not support keyword arguments");
		return NULL;
	}

	return py_loader_impl_object_method_invoke(self, &PyTuple_GET_ITEM(args, 0), (size_t)PyTuple_GET_SIZE(args));
}

#if PY_LOADER_IMPL_VECTORCALL
static PyObject *py_loader_impl_object_method_vectorcall(PyObject *self, PyObject *const *args, size_t nargsf, PyObject *kwnames)
{
	if (kwnames != NULL && PyTuple_GET_SIZE(kwnames) > 0)
	{
		PyErr_SetString(PyExc_TypeErrorPtr(), "MetaCall methods do not support keyword arguments");
		return NULL;
	}

	return py_loader_impl_object_method_invoke(self, args, (size_t)PyVectorcall_NARGS(nargsf));
}
#endif

static PyObject *py_loader_impl_object_method_get(PyObject *self, PyObject *instance, PyObject *owner)
{
	(void)owner;

	/* Accessed from the class, return the descriptor itself */
	if (instance == NULL)
	{
		Py_IncRef(self);
		return self;
	}

	/* Slow path, used when the method is accessed without calling it (i.e: f = obj.method) */
	return PyMethod_New(self, instance);
}

static PyTypeObject py_loader_impl_object_method_type = {
	PyVarObject_HEAD_INIT(NULL, 0) "PyCMethodDescriptor",
	sizeof(struct py_loader_impl_object_method_type),
	0,
	(destructor)py_loader_impl_object_method_dealloc, /* tp_dealloc */
	0,												  /* tp_vectorcall_offset */
	0,												  /* tp_getattr */
	0,												  /* tp_setattr */
	0,												  /* tp_as_async */
	0,												  /* tp_repr */
	0,												  /* tp_as_number */
	0,												  /* tp_as_sequence */
	0,												  /* tp_as_mapping */
	0,												  /* tp_hash */
	(ternaryfunc)py_loader_impl_object_method_call,	  /* tp_call */
	0,												  /* tp_str */
	0,												  /* tp_getattro */
	0,												  /* tp_setattro */
	0,												  /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,								  /* tp_flags */
	PyDoc_STR("MetaCall object method descriptor"),	  /* tp_doc */
	0,												  /* tp_traverse */
	0,												  /* tp_clear */
	0,												  /* tp_richcompare */
	0,												  /* tp_weaklistoffset */
	0,												  /* tp_iter */
	0,												  /* tp_iternext */
	0,												  /* tp_methods */
	0,												  /* tp_members */
	0,												  /* tp_getset */
	0,												  /* tp_base */
	0,												  /* tp_dict */
	(descrgetfunc)py_loader_impl_object_method_get,	  /* tp_descr_get */
	0,												  /* tp_descr_set */
	0,												  /* tp_dictoffset */
	0,												  /* tp_init */
	0,												  /* tp_alloc */
	0,												  /* tp_new */
	0,												  /* tp_free */
	0,												  /* tp_is_gc */
	0,												  /* tp_bases */
	0,												  /* tp_mro */
	0,												  /* tp_cache */
	0,												  /* tp_subclasses */
	0,												  /* tp_weaklist */
	0,												  /* tp_del */
	0,												  /* tp_version_tag */
	0,												  /* tp_finalize */
	0,												  /* tp_vectorcall */
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 12
	0, /* tp_watched */
#endif
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 13
	0, /* tp_versions_used */
#endif
};

int py_loader_impl_object_type_init(void)
{
#if PY_LOADER_IMPL_VECTORCALL
	/* Allow the method call optimization (avoids the bound method allocation) and call it through vectorcall (avoids the tuple allocation) */
	py_loader_impl_object_method_type.tp_flags |= Py_TPFLAGS_METHOD_DESCRIPTOR | Py_TPFLAGS_HAVE_VECTORCALL;
	py_loader_impl_object_method_type.tp_vectorcall_offset = offsetof(struct py_loader_impl_object_method_type, vectorcall);
#endif

	if (PyType_Ready(&py_loader_impl_object_type) < 0)
	{
		return -1;
	}

	if (PyType_Ready(&py_loader_impl_object_attribute_type) < 0)
	{
		return -1;
	}

	if (PyType_Ready(&py_loader_impl_object_method_type) < 0)
	{
		return -1;
	}

	if (py_loader_impl_object_cache == NULL)
	{
		py_loader_impl_object_cache = set_create(&hash_callback_ptr, &comparable_callback_ptr);

		if (py_loader_impl_object_cache == NULL)
		{
			return -1;
		}
	}

	return 0;
}

void py_loader_impl_object_type_destroy(void)
{
	if (py_loader_impl_object_cache != NULL)
	{
		set_destroy(py_loader_impl_object_cache);
		py_loader_impl_object_cache = NULL;
	}
}

PyTypeObject *py_loader_impl_object_base_type(void)
{
	return &py_loader_impl_object_type;
}

int py_loader_impl_object_check(PyObject *obj)
{
	return PyObject_TypeCheck(obj, &py_loader_impl_object_type) && ((struct py_loader_impl_object_type *)obj)->obj != NULL;
}

void *py_loader_impl_object_copy(PyObject *obj)
{
	struct py_loader_impl_object_type *wrapped = (struct py_loader_impl_object_type *)obj;

	if (wrapped == NULL)
	{
		return NULL;
	}

	return metacall_value_copy(wrapped->obj);
}

PyObject *py_loader_impl_object_wrap(PyTypeObject *type, loader_impl impl, value obj)
{
	/* The object already exists in its runtime, so the instance is allocated without calling tp_new nor __init__ */
	PyObject *instance = type->tp_alloc(type, 0);

	if (instance == NULL)
	{
		metacall_value_destroy(obj);
		return NULL;
	}

	struct py_loader_impl_object_type *wrapped = (struct py_loader_impl_object_type *)instance;

	wrapped->impl = impl;
	wrapped->obj = obj;

	if (set_insert(py_loader_impl_object_cache, (set_key)value_to_object(obj), (set_value)instance) != 0)
	{
		log_write("metacall", LOG_LEVEL_WARNING, "Failed to cache the Python wrapper of the MetaCall object %s", object_name(value_to_object(obj)));
	}

	return instance;
}

PyObject *py_loader_impl_object_new(loader_impl impl, value obj)
{
	object o = value_to_object(obj);

	/* Reuse the wrapper, so the identity of the object is preserved in Python */
	PyObject *instance = (PyObject *)set_get(py_loader_impl_object_cache, (set_key)o);

	if (instance != NULL)
	{
		Py_IncRef(instance);
		return instance;
	}

	klass cls = object_class_get(o);

	if (cls == NULL)
	{
		/* Objects without class do not have descriptors, use the base type directly */
		return py_loader_impl_object_wrap(&py_loader_impl_object_type, impl, metacall_value_copy(obj));
	}

	PyObject *type = py_loader_impl_class_type_get(impl, cls);

	if (type == NULL)
	{
		return NULL;
	}

	instance = py_loader_impl_object_wrap((PyTypeObject *)type, impl, metacall_value_copy(obj));

	/* The instance holds its own reference to the heap type */
	Py_DecRef(type);

	return instance;
}

PyObject *py_loader_impl_object_attribute_new(PyObject *name)
{
	struct py_loader_impl_object_attribute_type *descriptor = PyObject_New(struct py_loader_impl_object_attribute_type, &py_loader_impl_object_attribute_type);

	if (descriptor == NULL)
	{
		return NULL;
	}

	Py_IncRef(name);
	descriptor->name = name;

	return (PyObject *)descriptor;
}

PyObject *py_loader_impl_object_method_new(PyObject *name)
{
	struct py_loader_impl_object_method_type *descriptor = PyObject_New(struct py_loader_impl_object_method_type, &py_loader_impl_object_method_type);

	if (descriptor == NULL)
	{
		return NULL;
	}

	Py_IncRef(name);
	descriptor->name = name;
#if PY_LOADER_IMPL_VECTORCALL
	descriptor->vectorcall = &py_loader_impl_object_method_vectorcall;
#endif

	return (PyObject *)descriptor;
}
