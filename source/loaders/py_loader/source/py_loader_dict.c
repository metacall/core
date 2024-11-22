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

#include <py_loader/py_loader_dict.h>
#include <py_loader/py_loader_threading.h>

#include <metacall/metacall_value.h>

#include <Python.h>

#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 13
	#ifndef Py_BUILD_CORE
		#define Py_BUILD_CORE
	#endif
	#include <internal/pycore_dict.h>
#endif

struct py_loader_impl_dict_obj
{
	PyDictObject dict;
	void *v;
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
	0, /* tp_watched */
#endif
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 13
	0, /* tp_versions_used */
#endif
};

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

void py_loader_impl_dict_dealloc(struct py_loader_impl_dict_obj *self)
{
	metacall_value_destroy(self->v);
	Py_DECREF(self->parent); /* TODO: Review if this is correct or this line is unnecessary */

	PyDict_Type.tp_dealloc((PyObject *)self);
}

int py_loader_impl_dict_type_init(void)
{
	/* py_loader_impl_dict_type is derived from PyDict_Type */
	py_loader_impl_dict_type.tp_base = &PyDict_Type;

	return PyType_Ready(&py_loader_impl_dict_type);
}

PyObject *py_loader_impl_finalizer_wrap_map(PyObject *obj, void *v)
{
	py_loader_thread_acquire();

	PyObject *args = PyTuple_New(1);
	union py_loader_impl_dict_cast dict_cast = { &py_loader_impl_dict_type };

	PyTuple_SetItem(args, 0, obj);
	Py_INCREF(obj);
	PyObject *wrapper = PyObject_CallObject(dict_cast.object, args);
	Py_DECREF(args);

	py_loader_thread_release();

	if (wrapper == NULL)
	{
		return NULL;
	}

	struct py_loader_impl_dict_obj *wrapper_obj = (struct py_loader_impl_dict_obj *)wrapper;

	wrapper_obj->v = v;
	wrapper_obj->parent = obj;

	return wrapper;
}
