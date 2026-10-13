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

#include <adt/adt_map.h>
#include <adt/adt_set.h>

#include <log/log.h>

#include <metacall/metacall.h>

#include <stddef.h>

/* Arguments up to this size are converted into the stack, avoiding the allocation in each construction */
#define PY_LOADER_IMPL_CLASS_ARGS_STACK_SIZE 16

/* Metaclass instance layout, each instance is a Python type that wraps a MetaCall class */
struct py_loader_impl_class_type
{
	PyHeapTypeObject heap_type;
	loader_impl impl;
	value cls;
};

/* Callable that forwards to a static method of a MetaCall class */
struct py_loader_impl_class_method_type
{
	PyObject_HEAD
		loader_impl impl;
	value cls;
	PyObject *name;
#if PY_LOADER_IMPL_VECTORCALL
	vectorcallfunc vectorcall;
#endif
};

/* Cache of generated types indexed by klass, it holds borrowed references, each type removes itself on dealloc */
static set py_loader_impl_class_cache = NULL;

union py_loader_impl_class_cast
{
	PyTypeObject *type_object;
	PyObject *object;
};

static void py_loader_impl_class_method_dealloc(PyObject *self)
{
	struct py_loader_impl_class_method_type *wrapped = (struct py_loader_impl_class_method_type *)self;

	/* Clean the MetaCall class value and the method name */
	metacall_value_destroy(wrapped->cls);
	Py_DecRef(wrapped->name);

	/* Free the memory for the wrapper object itself */
	Py_TYPE(self)->tp_free(self);
}

static PyObject *py_loader_impl_class_method_call(PyObject *self, PyObject *args, PyObject *kwds)
{
	struct py_loader_impl_class_method_type *wrapped = (struct py_loader_impl_class_method_type *)self;

	if (kwds != NULL && PyDict_Size(kwds) > 0)
	{
		PyErr_SetString(PyExc_TypeErrorPtr(), "MetaCall static methods do not support keyword arguments");
		return NULL;
	}

	return py_loader_impl_method_invoke(wrapped->impl, value_to_class(wrapped->cls), wrapped->name, (py_loader_impl_reflect_invoke_ptr)&metacallv_class, &PyTuple_GET_ITEM(args, 0), (size_t)PyTuple_GET_SIZE(args));
}

#if PY_LOADER_IMPL_VECTORCALL
static PyObject *py_loader_impl_class_method_vectorcall(PyObject *self, PyObject *const *args, size_t nargsf, PyObject *kwnames)
{
	struct py_loader_impl_class_method_type *wrapped = (struct py_loader_impl_class_method_type *)self;

	if (kwnames != NULL && PyTuple_GET_SIZE(kwnames) > 0)
	{
		PyErr_SetString(PyExc_TypeErrorPtr(), "MetaCall static methods do not support keyword arguments");
		return NULL;
	}

	return py_loader_impl_method_invoke(wrapped->impl, value_to_class(wrapped->cls), wrapped->name, (py_loader_impl_reflect_invoke_ptr)&metacallv_class, args, (size_t)PyVectorcall_NARGS(nargsf));
}
#endif

static PyTypeObject py_loader_impl_class_method_type = {
	PyVarObject_HEAD_INIT(NULL, 0) "PyCClassMethodWrapper",
	sizeof(struct py_loader_impl_class_method_type),
	0,
	(destructor)py_loader_impl_class_method_dealloc,   /* tp_dealloc */
	0,												   /* tp_vectorcall_offset */
	0,												   /* tp_getattr */
	0,												   /* tp_setattr */
	0,												   /* tp_as_async */
	0,												   /* tp_repr */
	0,												   /* tp_as_number */
	0,												   /* tp_as_sequence */
	0,												   /* tp_as_mapping */
	0,												   /* tp_hash */
	(ternaryfunc)py_loader_impl_class_method_call,	   /* tp_call */
	0,												   /* tp_str */
	0,												   /* tp_getattro */
	0,												   /* tp_setattro */
	0,												   /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,								   /* tp_flags */
	PyDoc_STR("MetaCall class static method wrapper"), /* tp_doc */
	0,												   /* tp_traverse */
	0,												   /* tp_clear */
	0,												   /* tp_richcompare */
	0,												   /* tp_weaklistoffset */
	0,												   /* tp_iter */
	0,												   /* tp_iternext */
	0,												   /* tp_methods */
	0,												   /* tp_members */
	0,												   /* tp_getset */
	0,												   /* tp_base */
	0,												   /* tp_dict */
	0,												   /* tp_descr_get */
	0,												   /* tp_descr_set */
	0,												   /* tp_dictoffset */
	0,												   /* tp_init */
	0,												   /* tp_alloc */
	0,												   /* tp_new */
	0,												   /* tp_free */
	0,												   /* tp_is_gc */
	0,												   /* tp_bases */
	0,												   /* tp_mro */
	0,												   /* tp_cache */
	0,												   /* tp_subclasses */
	0,												   /* tp_weaklist */
	0,												   /* tp_del */
	0,												   /* tp_version_tag */
	0,												   /* tp_finalize */
	0,												   /* tp_vectorcall */
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 12
	0, /* tp_watched */
#endif
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 13
	0, /* tp_versions_used */
#endif
};

static PyObject *py_loader_impl_class_method_new(loader_impl impl, value cls, PyObject *name)
{
	struct py_loader_impl_class_method_type *wrapped = PyObject_New(struct py_loader_impl_class_method_type, &py_loader_impl_class_method_type);

	if (wrapped == NULL)
	{
		return NULL;
	}

	Py_IncRef(name);

	wrapped->impl = impl;
	wrapped->cls = metacall_value_copy(cls);
	wrapped->name = name;
#if PY_LOADER_IMPL_VECTORCALL
	wrapped->vectorcall = &py_loader_impl_class_method_vectorcall;
#endif

	return (PyObject *)wrapped;
}

static void py_loader_impl_class_dealloc(PyObject *self)
{
	struct py_loader_impl_class_type *wrapped = (struct py_loader_impl_class_type *)self;

	if (wrapped->cls != NULL)
	{
		if (py_loader_impl_class_cache != NULL)
		{
			set_remove(py_loader_impl_class_cache, (set_key)value_to_class(wrapped->cls));
		}

		/* Clean the MetaCall class value */
		metacall_value_destroy(wrapped->cls);
		wrapped->cls = NULL;
	}

	/* Free the type object itself */
	PyTypeTypePtr()->tp_dealloc(self);
}

static PyObject *py_loader_impl_class_call(PyObject *self, PyObject *args, PyObject *kwds)
{
	struct py_loader_impl_class_type *wrapped = (struct py_loader_impl_class_type *)self;

	/* Subclasses defined in Python share this metaclass but they do not wrap any MetaCall class */
	if (wrapped->cls == NULL)
	{
		return PyTypeTypePtr()->tp_call(self, args, kwds);
	}

	klass cls = value_to_class(wrapped->cls);

	if (kwds != NULL && PyDict_Size(kwds) > 0)
	{
		PyErr_Format(PyExc_TypeErrorPtr(), "The constructor of the MetaCall class '%s' does not support keyword arguments", class_name(cls));
		return NULL;
	}

	size_t argc = (size_t)PyTuple_GET_SIZE(args);
	void *stack_args[PY_LOADER_IMPL_CLASS_ARGS_STACK_SIZE];
	type_id stack_ids[PY_LOADER_IMPL_CLASS_ARGS_STACK_SIZE];
	void **value_args = stack_args;
	type_id *ids = stack_ids;

	if (argc > PY_LOADER_IMPL_CLASS_ARGS_STACK_SIZE)
	{
		value_args = malloc(sizeof(void *) * argc);
		ids = malloc(sizeof(type_id) * argc);

		if (value_args == NULL || ids == NULL)
		{
			free(value_args);
			free(ids);
			PyErr_Format(PyExc_ValueErrorPtr(), "Invalid allocation of arguments for the constructor of the MetaCall class '%s'", class_name(cls));
			return NULL;
		}
	}

	/* Generate metacall values from python values */
	for (size_t args_count = 0; args_count < argc; ++args_count)
	{
		PyObject *arg = PyTuple_GET_ITEM(args, args_count);
		type_id id = py_loader_impl_capi_to_value_type(wrapped->impl, arg);
		value_args[args_count] = py_loader_impl_capi_to_value(wrapped->impl, arg, id);
		ids[args_count] = value_type_id(value_args[args_count]);
	}

	py_loader_thread_release();

	/* Construct the object in its own runtime */
	constructor ctor = class_constructor(cls, ids, argc);
	object o = class_new(cls, class_name(cls), ctor, value_args, argc);
	value obj = NULL;

	if (o != NULL)
	{
		obj = value_create_object(o);

		if (obj == NULL)
		{
			object_destroy(o);
		}
	}

	/* Destroy argument values */
	for (size_t args_count = 0; args_count < argc; ++args_count)
	{
		value_type_destroy(value_args[args_count]);
	}

	py_loader_thread_acquire();

	if (value_args != stack_args)
	{
		free(value_args);
		free(ids);
	}

	if (obj == NULL)
	{
		PyErr_Format(PyExc_RuntimeErrorPtr(), "Failed to construct an instance of the MetaCall class '%s'", class_name(cls));
		return NULL;
	}

	return py_loader_impl_object_wrap((PyTypeObject *)self, wrapped->impl, obj);
}

static PyObject *py_loader_impl_class_getattro(PyObject *self, PyObject *name)
{
	struct py_loader_impl_class_type *wrapped = (struct py_loader_impl_class_type *)self;

	if (wrapped->cls == NULL)
	{
		return PyTypeTypePtr()->tp_getattro(self, name);
	}

	const char *key = PyUnicode_AsUTF8(name);

	if (key == NULL)
	{
		return NULL;
	}

	klass cls = value_to_class(wrapped->cls);

	/* Static attributes are forwarded first, so the value is always up to date and the type lookup miss is avoided */
	if (class_static_attribute(cls, key) != NULL)
	{
		py_loader_thread_release();

		value v = class_static_get(cls, key);

		py_loader_thread_acquire();

		if (v == NULL)
		{
			PyErr_Format(PyExc_AttributeErrorPtr(), "Failed to get the static attribute '%s' of the MetaCall class '%s'", key, class_name(cls));
			return NULL;
		}

		PyObject *result = py_loader_impl_value_to_capi(wrapped->impl, value_type_id(v), v);

		py_loader_thread_release();
		value_type_destroy(v);
		py_loader_thread_acquire();

		return result;
	}

	/* Static methods and descriptors are stored in the type dictionary */
	return PyTypeTypePtr()->tp_getattro(self, name);
}

static int py_loader_impl_class_setattro(PyObject *self, PyObject *name, PyObject *v_obj)
{
	struct py_loader_impl_class_type *wrapped = (struct py_loader_impl_class_type *)self;

	if (wrapped->cls != NULL)
	{
		const char *key = PyUnicode_AsUTF8(name);

		if (key == NULL)
		{
			return -1;
		}

		klass cls = value_to_class(wrapped->cls);

		if (class_static_attribute(cls, key) != NULL)
		{
			if (v_obj == NULL)
			{
				PyErr_Format(PyExc_AttributeErrorPtr(), "Cannot delete the static attribute '%s' of the MetaCall class '%s'", key, class_name(cls));
				return -1;
			}

			type_id id = py_loader_impl_capi_to_value_type(wrapped->impl, v_obj);
			value v = py_loader_impl_capi_to_value(wrapped->impl, v_obj, id);

			if (v == NULL)
			{
				PyErr_Format(PyExc_TypeErrorPtr(), "Invalid value for the static attribute '%s' of the MetaCall class '%s'", key, class_name(cls));
				return -1;
			}

			py_loader_thread_release();

			int result = class_static_set(cls, key, v);

			value_type_destroy(v);

			py_loader_thread_acquire();

			if (result != 0)
			{
				PyErr_Format(PyExc_AttributeErrorPtr(), "Failed to set the static attribute '%s' of the MetaCall class '%s'", key, class_name(cls));
				return -1;
			}

			return 0;
		}
	}

	return PyTypeTypePtr()->tp_setattro(self, name, v_obj);
}

static PyTypeObject py_loader_impl_class_type = {
	PyVarObject_HEAD_INIT(NULL, 0) "PyCClassWrapper",
	sizeof(struct py_loader_impl_class_type),
	0,
	(destructor)py_loader_impl_class_dealloc,	   /* tp_dealloc */
	0,											   /* tp_vectorcall_offset */
	0,											   /* tp_getattr */
	0,											   /* tp_setattr */
	0,											   /* tp_as_async */
	0,											   /* tp_repr */
	0,											   /* tp_as_number */
	0,											   /* tp_as_sequence */
	0,											   /* tp_as_mapping */
	0,											   /* tp_hash */
	(ternaryfunc)py_loader_impl_class_call,		   /* tp_call */
	0,											   /* tp_str */
	(getattrofunc)py_loader_impl_class_getattro,   /* tp_getattro */
	(setattrofunc)py_loader_impl_class_setattro,   /* tp_setattro */
	0,											   /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,							   /* tp_flags */
	PyDoc_STR("MetaCall class metaclass wrapper"), /* tp_doc */
	0,											   /* tp_traverse */
	0,											   /* tp_clear */
	0,											   /* tp_richcompare */
	0,											   /* tp_weaklistoffset */
	0,											   /* tp_iter */
	0,											   /* tp_iternext */
	0,											   /* tp_methods */
	0,											   /* tp_members */
	0,											   /* tp_getset */
	0,											   /* tp_base (PyType_Type, set on initialization) */
	0,											   /* tp_dict */
	0,											   /* tp_descr_get */
	0,											   /* tp_descr_set */
	0,											   /* tp_dictoffset */
	0,											   /* tp_init */
	0,											   /* tp_alloc */
	0,											   /* tp_new */
	0,											   /* tp_free */
	0,											   /* tp_is_gc */
	0,											   /* tp_bases */
	0,											   /* tp_mro */
	0,											   /* tp_cache */
	0,											   /* tp_subclasses */
	0,											   /* tp_weaklist */
	0,											   /* tp_del */
	0,											   /* tp_version_tag */
	0,											   /* tp_finalize */
	0,											   /* tp_vectorcall */
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 12
	0, /* tp_watched */
#endif
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 13
	0, /* tp_versions_used */
#endif
};

static int py_loader_impl_class_dict_set(PyObject *dict, const char *key, PyObject *(*create)(PyObject *, void *), void *data)
{
	PyObject *name = PyUnicode_FromString(key);

	if (name == NULL)
	{
		return 1;
	}

	/* Methods are stored in a multimap (one entry per overload), the first definition of a name wins */
	if (PyDict_Contains(dict, name) == 1)
	{
		Py_DecRef(name);
		return 0;
	}

	PyObject *descriptor = create(name, data);
	int result = descriptor == NULL || PyDict_SetItem(dict, name, descriptor) != 0;

	Py_DecRef(descriptor);
	Py_DecRef(name);

	return result;
}

static PyObject *py_loader_impl_class_dict_attribute(PyObject *name, void *data)
{
	(void)data;

	return py_loader_impl_object_attribute_new(name);
}

static PyObject *py_loader_impl_class_dict_method(PyObject *name, void *data)
{
	(void)data;

	return py_loader_impl_object_method_new(name);
}

struct py_loader_impl_class_dict_static_method_data
{
	loader_impl impl;
	value cls;
};

static PyObject *py_loader_impl_class_dict_static_method(PyObject *name, void *data)
{
	struct py_loader_impl_class_dict_static_method_data *static_method_data = (struct py_loader_impl_class_dict_static_method_data *)data;

	return py_loader_impl_class_method_new(static_method_data->impl, static_method_data->cls, name);
}

static PyObject *py_loader_impl_class_dict(loader_impl impl, klass c, value cls)
{
	PyObject *dict = PyDict_New();
	struct set_iterator_type attribute_it;
	struct map_iterator_type method_it;
	struct py_loader_impl_class_dict_static_method_data static_method_data = { impl, cls };

	if (dict == NULL)
	{
		return NULL;
	}

	/* The generic attribute lookup has no hook for a miss, so all the descriptors are created at once */
	for (class_attributes_begin(c, &attribute_it); set_iterator_end(&attribute_it) != 0; set_iterator_next(&attribute_it))
	{
		attribute attr = set_iterator_value(&attribute_it);

		if (py_loader_impl_class_dict_set(dict, attribute_name(attr), &py_loader_impl_class_dict_attribute, NULL) != 0)
		{
			goto error;
		}
	}

	for (class_methods_begin(c, &method_it); map_iterator_end(&method_it) != 0; map_iterator_next(&method_it))
	{
		method m = map_iterator_value(&method_it);

		if (py_loader_impl_class_dict_set(dict, method_name(m), &py_loader_impl_class_dict_method, NULL) != 0)
		{
			goto error;
		}
	}

	for (class_static_methods_begin(c, &method_it); map_iterator_end(&method_it) != 0; map_iterator_next(&method_it))
	{
		method m = map_iterator_value(&method_it);

		if (py_loader_impl_class_dict_set(dict, method_name(m), &py_loader_impl_class_dict_static_method, &static_method_data) != 0)
		{
			goto error;
		}
	}

	/* Instances do not need __dict__ nor __weakref__ because the state lives in the MetaCall object, set at the end so they cannot be overwritten */
	PyObject *slots = PyTuple_New(0);
	PyObject *module = PyUnicode_FromString("metacall");

	if (slots == NULL || module == NULL || PyDict_SetItemString(dict, "__slots__", slots) != 0 || PyDict_SetItemString(dict, "__module__", module) != 0)
	{
		Py_DecRef(slots);
		Py_DecRef(module);
		goto error;
	}

	Py_DecRef(slots);
	Py_DecRef(module);

	return dict;

error:
	Py_DecRef(dict);
	return NULL;
}

static PyObject *py_loader_impl_class_type_create(loader_impl impl, klass c, value cls)
{
	const char *name = class_name(c);
	PyObject *args = PyTuple_New(3);
	PyObject *py_name = PyUnicode_FromString(name != NULL ? name : "");
	PyObject *bases = PyTuple_New(1);
	PyObject *dict = py_loader_impl_class_dict(impl, c, cls);

	if (args == NULL || py_name == NULL || bases == NULL || dict == NULL)
	{
		Py_DecRef(args);
		Py_DecRef(py_name);
		Py_DecRef(bases);
		Py_DecRef(dict);
		metacall_value_destroy(cls);
		return NULL;
	}

	/* All the generated classes inherit from the object wrapper, so instances can hold the MetaCall object */
	union py_loader_impl_class_cast base_cast = { py_loader_impl_object_base_type() };

	Py_IncRef(base_cast.object);
	PyTuple_SET_ITEM(bases, 0, base_cast.object);

	/* The tuple steals the references of name, bases and dict */
	PyTuple_SET_ITEM(args, 0, py_name);
	PyTuple_SET_ITEM(args, 1, bases);
	PyTuple_SET_ITEM(args, 2, dict);

	/* Call the metaclass like type(name, bases, dict), this works with any Python 3 version */
	union py_loader_impl_class_cast class_cast = { &py_loader_impl_class_type };

	PyObject *type = PyObject_Call(class_cast.object, args, NULL);

	Py_DecRef(args);

	if (type == NULL)
	{
		metacall_value_destroy(cls);
		return NULL;
	}

	struct py_loader_impl_class_type *wrapped = (struct py_loader_impl_class_type *)type;

	wrapped->impl = impl;
	wrapped->cls = cls;

	if (set_insert(py_loader_impl_class_cache, (set_key)c, (set_value)type) != 0)
	{
		log_write("metacall", LOG_LEVEL_WARNING, "Failed to cache the Python type of the MetaCall class %s", name);
	}

	return type;
}

int py_loader_impl_class_type_init(void)
{
	/* The metaclass inherits from type, it must be set at run-time because of the symbol fallback */
	py_loader_impl_class_type.tp_base = PyTypeTypePtr();

#if PY_LOADER_IMPL_VECTORCALL
	/* Call static methods through vectorcall (avoids the tuple allocation) */
	py_loader_impl_class_method_type.tp_flags |= Py_TPFLAGS_HAVE_VECTORCALL;
	py_loader_impl_class_method_type.tp_vectorcall_offset = offsetof(struct py_loader_impl_class_method_type, vectorcall);
#endif

	if (PyType_Ready(&py_loader_impl_class_type) < 0)
	{
		return -1;
	}

	if (PyType_Ready(&py_loader_impl_class_method_type) < 0)
	{
		return -1;
	}

	if (py_loader_impl_class_cache == NULL)
	{
		py_loader_impl_class_cache = set_create(&hash_callback_ptr, &comparable_callback_ptr);

		if (py_loader_impl_class_cache == NULL)
		{
			return -1;
		}
	}

	return 0;
}

void py_loader_impl_class_type_destroy(void)
{
	if (py_loader_impl_class_cache != NULL)
	{
		set_destroy(py_loader_impl_class_cache);
		py_loader_impl_class_cache = NULL;
	}
}

int py_loader_impl_class_check(PyObject *obj)
{
	return PyObject_TypeCheck(obj, &py_loader_impl_class_type) && ((struct py_loader_impl_class_type *)obj)->cls != NULL;
}

void *py_loader_impl_class_copy(PyObject *obj)
{
	struct py_loader_impl_class_type *wrapped = (struct py_loader_impl_class_type *)obj;

	if (wrapped == NULL)
	{
		return NULL;
	}

	return metacall_value_copy(wrapped->cls);
}

PyObject *py_loader_impl_class_type_get(loader_impl impl, klass cls)
{
	/* Reuse the generated type, so the identity of the class is preserved in Python */
	PyObject *type = (PyObject *)set_get(py_loader_impl_class_cache, (set_key)cls);

	if (type != NULL)
	{
		Py_IncRef(type);
		return type;
	}

	value v = value_create_class(cls);

	if (v == NULL)
	{
		return NULL;
	}

	return py_loader_impl_class_type_create(impl, cls, v);
}

PyObject *py_loader_impl_class_new(loader_impl impl, value cls)
{
	klass c = value_to_class(cls);

	/* Reuse the generated type, so the identity of the class is preserved in Python */
	PyObject *type = (PyObject *)set_get(py_loader_impl_class_cache, (set_key)c);

	if (type != NULL)
	{
		Py_IncRef(type);
		return type;
	}

	return py_loader_impl_class_type_create(impl, c, metacall_value_copy(cls));
}
