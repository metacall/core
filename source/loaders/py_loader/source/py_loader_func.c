/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <py_loader/py_loader_func.h>
#include <py_loader/py_loader_symbol_fallback.h>
#include <py_loader/py_loader_threading.h>

#include <log/log.h>

#include <metacall/metacall.h>

struct py_loader_impl_func_obj
{
	PyObject_HEAD
		loader_impl impl;
	loader_impl_py py_impl;
	value callback;
};

static void py_loader_impl_func_dealloc(PyObject *self)
{
	struct py_loader_impl_func_obj *wrapped = (struct py_loader_impl_func_obj *)self;

	/* Clean the MetaCall function value */
	metacall_value_destroy(wrapped->callback);

	/* Free the memory for the wrapper object itself */
	Py_TYPE(self)->tp_free(self);
}

static PyObject *py_loader_impl_func_call(PyObject *self, PyObject *args, PyObject *kwds)
{
	struct py_loader_impl_func_obj *wrapped = (struct py_loader_impl_func_obj *)self;

	/* TODO: Support callback named arguments call? */
	(void)kwds;

	if (wrapped == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Fatal error when invoking a function callback, state cannot be recovered, avoiding the function call");
		return Py_ReturnNone();
	}

	py_loader_thread_acquire();

	Py_ssize_t callee_args_size = PyTuple_Size(args);
	size_t args_size = callee_args_size < 0 ? 0 : (size_t)callee_args_size;
	void **value_args = args_size == 0 ? metacall_null_args : malloc(sizeof(void *) * args_size);

	if (value_args == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid allocation of arguments for callback");
		py_loader_thread_release();
		return Py_ReturnNone();
	}

	/* Generate metacall values from python values */
	for (size_t args_count = 0; args_count < args_size; ++args_count)
	{
		PyObject *arg = PyTuple_GetItem(args, (Py_ssize_t)args_count);
		type_id id = py_loader_impl_capi_to_value_type(wrapped->impl, arg);
		value_args[args_count] = py_loader_impl_capi_to_value(wrapped->impl, arg, id);
	}

	py_loader_thread_release();

	int thread_state_saved = py_loader_thread_is_main() && PyGILState_Check();

	if (thread_state_saved)
	{
		py_loader_thread_release();
	}

	/* Execute the callback */
	value ret = (value)function_call(value_to_function(wrapped->callback), value_args, args_size);

	/* Destroy argument values */
	for (size_t args_count = 0; args_count < args_size; ++args_count)
	{
		value_type_destroy(value_args[args_count]);
	}

	if (value_args != metacall_null_args)
	{
		free(value_args);
	}

	if (thread_state_saved)
	{
		py_loader_thread_acquire();
	}

	/* Transform the return value into a python value */
	if (ret != NULL)
	{
		py_loader_thread_acquire();
		PyObject *py_ret = py_loader_impl_value_to_capi(wrapped->impl, value_type_id(ret), ret);
		py_loader_thread_release();
		value_type_destroy(ret);
		return py_ret;
	}

	return Py_ReturnNone();
}

static PyTypeObject py_loader_impl_func_type = {
	PyVarObject_HEAD_INIT(NULL, 0) "PyCFunctionWrapper",
	sizeof(struct py_loader_impl_func_obj),
	0,
	(destructor)py_loader_impl_func_dealloc,		  /* tp_dealloc */
	0,												  /* tp_vectorcall_offset */
	0,												  /* tp_getattr */
	0,												  /* tp_setattr */
	0,												  /* tp_as_async */
	0,												  /* tp_repr */
	0,												  /* tp_as_number */
	0,												  /* tp_as_sequence */
	0,												  /* tp_as_mapping */
	0,												  /* tp_hash */
	(ternaryfunc)py_loader_impl_func_call,			  /* tp_call */
	0,												  /* tp_str */
	0,												  /* tp_getattro */
	0,												  /* tp_setattro */
	0,												  /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,		  /* tp_flags */
	PyDoc_STR("PyCFunction wrapper destructor hook"), /* tp_doc */
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
	0,												  /* tp_descr_get */
	0,												  /* tp_descr_set */
	0,												  /* tp_dictoffset */
	0,												  /* tp_init */
	0,												  /* tp_alloc */
	(newfunc)PyType_GenericNew,						  /* tp_new */
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

int py_loader_impl_func_type_init(void)
{
	return PyType_Ready(&py_loader_impl_func_type);
}

int py_loader_impl_func_check(PyObject *obj)
{
	return PyObject_TypeCheck(obj, &py_loader_impl_func_type);
}

void *py_loader_impl_func_copy(PyObject *obj)
{
	struct py_loader_impl_func_obj *wrapped = (struct py_loader_impl_func_obj *)obj;

	if (wrapped == NULL)
	{
		return NULL;
	}

	return metacall_value_copy(wrapped->callback);
}

PyObject *py_loader_impl_func_new(loader_impl impl, loader_impl_py py_impl, value callback)
{
	struct py_loader_impl_func_obj *wrapped = PyObject_New(struct py_loader_impl_func_obj, &py_loader_impl_func_type);

	if (wrapped == NULL)
	{
		return NULL;
	}

	wrapped->impl = impl;
	wrapped->py_impl = py_impl;
	wrapped->callback = value_type_copy(callback);

	return (PyObject *)wrapped;
}
