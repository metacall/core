#ifndef LOADER_IMPL_PY_DISCOVERING_H
#define LOADER_IMPL_PY_DISCOVERING_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <Python.h>

PyObject ** loader_impl_py_inspect(void);

PyObject ** loader_impl_py_inspect_signature(void);

int loader_impl_py_discovering_initialize(void);

int loader_impl_py_discovering_destroy(void);

int loader_impl_py_function_args_count(PyObject * func);

int loader_impl_py_function_inspect(PyObject * func, int args_count);

#ifdef __cplusplus
}
#endif

#endif // LOADER_IMPL_PY_DISCOVERING_H
