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
#include <py_loader/py_loader_symbol_fallback.h>
#include <py_loader/py_loader_threading.h>

#include <log/log.h>

#include <metacall/metacall.h>

#if 0
	#define PY_SSIZE_T_CLEAN
	#include <Python.h>
	#include <stdlib.h>
	#include <structmember.h>

/*
    Demonstrates fully dynamic allocation of:
    - methods (malloc / free)
    - members (malloc / free)
    - cleanup on module teardown via type dealloc hook

    NOTE:
    Python may keep references to type objects, so freeing must be done
    only after ensuring type is no longer in use.
*/

/* =========================
   Dynamic storage pointers
   ========================= */

static PyMethodDef *g_methods = NULL;
static PyMemberDef *g_members = NULL;

/* =========================
   Generic methods
   ========================= */

static PyObject* hello(PyObject *self, PyObject *args) {
    printf("hello from dynamic method\n");
    Py_RETURN_NONE;
}

static PyObject* add(PyObject *self, PyObject *args) {
    long a, b;
    if (!PyArg_ParseTuple(args, "ll", &a, &b)) {
        return NULL;
    }
    return PyLong_FromLong(a + b);
}

/* =========================
   Dynamic init
   ========================= */

static int dynamic_init(PyObject *self, PyObject *args, PyObject *kwds) {
    long x = 0, y = 0;

    static char *kwlist[] = {"x", "y", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ll", kwlist, &x, &y)) {
        return -1;
    }

    PyObject_SetAttrString(self, "x", PyLong_FromLong(x));
    PyObject_SetAttrString(self, "y", PyLong_FromLong(y));

    return 0;
}

/* =========================
   Type dealloc hook
   ========================= */

static void dynamic_dealloc(PyObject *self) {
    PyTypeObject *type = Py_TYPE(self);

    Py_TYPE(self)->tp_free(self);

    /* free dynamic allocations if still present */
    if (g_methods) {
        free(g_methods);
        g_methods = NULL;
    }

    if (g_members) {
        free(g_members);
        g_members = NULL;
    }

    /* avoid dangling pointers inside type */
    type->tp_methods = NULL;
    type->tp_members = NULL;
}

/* =========================
   Build dynamic methods
   ========================= */

static PyMethodDef* build_methods() {
    PyMethodDef *methods = (PyMethodDef*)malloc(sizeof(PyMethodDef) * 3);

    methods[0].ml_name = "hello";
    methods[0].ml_meth = (PyCFunction)hello;
    methods[0].ml_flags = METH_VARARGS;
    methods[0].ml_doc = "prints hello";

    methods[1].ml_name = "add";
    methods[1].ml_meth = (PyCFunction)add;
    methods[1].ml_flags = METH_VARARGS;
    methods[1].ml_doc = "adds two numbers";

    memset(&methods[2], 0, sizeof(PyMethodDef));

    return methods;
}

/* =========================
   Build dynamic members
   ========================= */

static PyMemberDef* build_members() {
    PyMemberDef *members = (PyMemberDef*)malloc(sizeof(PyMemberDef) * 3);

    members[0].name = "x";
    members[0].type = T_LONG;
    members[0].offset = 0;
    members[0].flags = 0;
    members[0].doc = "dynamic x";

    members[1].name = "y";
    members[1].type = T_LONG;
    members[1].offset = 0;
    members[1].flags = 0;
    members[1].doc = "dynamic y";

    memset(&members[2], 0, sizeof(PyMemberDef));

    return members;
}

/* =========================
   Constructor
   ========================= */

static int dynamic_init(PyObject *self, PyObject *args, PyObject *kwds) {
    long x = 0, y = 0;
    static char *kwlist[] = {"x", "y", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ll", kwlist, &x, &y)) {
        return -1;
    }

    PyObject_SetAttrString(self, "x", PyLong_FromLong(x));
    PyObject_SetAttrString(self, "y", PyLong_FromLong(y));

    return 0;
}

/* =========================
   Slots + Type
   ========================= */

static PyType_Slot slots[] = {
    {Py_tp_init, (void*)dynamic_init},
    {Py_tp_dealloc, (void*)dynamic_dealloc},
    {0, 0}
};

static PyType_Spec spec = {
    "dynamic_module.DynamicClass",
    sizeof(PyObject),
    0,
    Py_TPFLAGS_DEFAULT,
    slots
};

/* =========================
   Module definition
   ========================= */

static PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "dynamic_module",
    "Dynamic class with malloc-based methods/members",
    -1,
    NULL
};

/* =========================
   Main
   ========================= */

int main(int argc, char *argv[]) {
    Py_Initialize();

    PyObject *module = PyModule_Create(&moduledef);
    if (!module) {
        PyErr_Print();
        return 1;
    }

    PyObject *cls = PyType_FromSpec(&spec);
    if (!cls) {
        PyErr_Print();
        return 1;
    }

    /* allocate dynamic methods + members */
    g_methods = build_methods();
    g_members = build_members();

    PyTypeObject *type = (PyTypeObject*)cls;
    type->tp_methods = g_methods;
    type->tp_members = g_members;

    PyType_Ready(type);

    PyModule_AddObject(module, "DynamicClass", cls);

    /* test instance */
    PyObject *obj = PyObject_CallFunction(cls, "ll", 5, 10);
    if (!obj) {
        PyErr_Print();
        return 1;
    }

    PyObject *res = PyObject_CallMethod(obj, "add", "(ll)", 7, 8);
    if (res) {
        printf("add = %ld\n", PyLong_AsLong(res));
        Py_DECREF(res);
    } else {
        PyErr_Print();
    }

    Py_DECREF(obj);

    Py_Finalize();
    return 0;
}

*/
#endif

// binding to the class
#if 0
	#include <Python.h>
	#include <structmember.h>

/* ==========================================================================
   1. Data Structures
   ========================================================================== */

// The raw C data structure you want to bind
typedef struct {
    int id;
    double scale;
} MyCData;

// The Python wrapper object structure
typedef struct {
    PyObject_HEAD
    MyCData* c_data;  // Pointer to the bound C data
} MyClassObject;

/* ==========================================================================
   2. Constructor and Destructor Implementation
   ========================================================================== */

// Memory Allocation (tp_new)
static PyObject *MyClass_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    MyClassObject *self;
    self = (MyClassObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        // Initialize the pointer safely to NULL
        self->c_data = NULL;
    }
    return (PyObject *)self;
}

// Data Initialization (tp_init)
static int MyClass_init(MyClassObject *self, PyObject *args, PyObject *kwds) {
    int initial_id;
    double initial_scale;

    // Parse arguments passed from Python code
    if (!PyArg_ParseTuple(args, "id", &initial_id, &initial_scale)) {
        return -1; 
    }

    // Allocate the custom C structure
    self->c_data = (MyCData*) malloc(sizeof(MyCData));
    if (self->c_data == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Unable to allocate C data.");
        return -1;
    }

    // Bind the values to your C data
    self->c_data->id = initial_id;
    self->c_data->scale = initial_scale;

    return 0; // Success
}

// Destructor / Memory Cleanup (tp_dealloc)
static void MyClass_dealloc(MyClassObject *self) {
    // Free your custom allocated C structure to prevent leaks
    if (self->c_data != NULL) {
        free(self->c_data);
    }
    
    // Free the Python object wrapper itself
    Py_TYPE(self)->tp_free((PyObject *)self);
}

/* ==========================================================================
   3. Custom Methods
   ========================================================================== */

// A method executing C logic using the bound data
static PyObject *MyClass_process(MyClassObject *self, PyObject *Py_UNUSED(ignored)) {
    if (self->c_data == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "C data structure uninitialized.");
        return NULL;
    }

    // Access and read from your bound C data structure
    double result = self->c_data->id * self->c_data->scale;

    // Convert the C primitive back to a Python object type
    return PyFloat_FromDouble(result);
}

// Method Table for the class
static PyMethodDef MyClass_methods[] = {
    {"process", (PyCFunction)MyClass_process, METH_NOARGS, "Computes a value from bound C data."},
    {NULL, NULL, 0, NULL}  // Sentinel
};

/* ==========================================================================
   4. Type Object and Module Definition
   ========================================================================== */

// Define the Python Type setup
static PyTypeObject MyClassType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mymodule.MyClass",
    .tp_doc = PyDoc_STR("A Python object wrapping an underlying C struct"),
    .tp_basicsize = sizeof(MyClassObject), // Size of the wrapper struct
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = MyClass_new,
    .tp_init = (initproc)MyClass_init,
    .tp_dealloc = (destructor)MyClass_dealloc,
    .tp_methods = MyClass_methods,
};

// Define the Module itself
static struct PyModuleDef mymodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "mymodule",
    .m_doc = "Module containing a custom C-backed Python class.",
    .m_size = -1,
};

// Module entry point / Initialization function called by Python
PyMODINIT_FUNC PyInit_mymodule(void) {
    PyObject *m;

    // Finalize the type object mapping layout details
    if (PyType_Ready(&MyClassType) < 0) {
        return NULL;
    }

    // Create the extension module object
    m = PyModule_Create(&mymodule);
    if (m == NULL) {
        return NULL;
    }

    // Add MyClass to the module dictionary
    Py_INCREF(&MyClassType);
    if (PyModule_AddObject(m, "MyClass", (PyObject *)&MyClassType) < 0) {
        Py_DECREF(&MyClassType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
#endif

static PyObject *hello(PyObject *self, PyObject *args)
{
	printf("hello from dynamic method\n");
	Py_RETURN_NONE;
}

PyMethodDef *py_loader_impl_class_methods_to_capi(map methods)
{
	struct map_iterator_type it;
	size_t iterator = 0, size = map_size(methods);

	/* The extra element is for the list null terminator */
	PyMethodDef *py_methods = (PyMethodDef *)malloc(sizeof(PyMethodDef) * (size + 1));

	if (py_methods == NULL)
	{
		return NULL;
	}

	/* Initialize null terminator */
	memset(&py_methods[size], 0, sizeof(PyMethodDef));

	for (map_iterator_begin(&it, methods); map_iterator_end(&it) != 0; map_iterator_next(&it), ++iterator)
	{
		const char *key = map_iterator_key(&it);
		method m = map_iterator_value(&it);

		py_methods[iterator].ml_name = key;
		py_methods[iterator].ml_meth = (PyCFunction)hello; // TODO: I need to bind here the method and other things
		py_methods[iterator].ml_flags = METH_VARARGS;
		py_methods[iterator].ml_doc = "";
	}
}

PyObject *py_loader_impl_class_value_to_capi(loader_impl impl, loader_impl_py py_impl, value v)
{
	klass cls = value_to_class(v);
	map methods = class_methods(cls);
	PyMethodDef *py_methods = py_loader_impl_class_methods_to_capi(methods);

	// TODO: Constructor first, so we can bind data in methods

	return NULL;
}
