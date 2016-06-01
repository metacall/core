/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading python code at run-time into a process.
 *
 */

#include <py_loader/py_loader_impl.h>

#include <loader/loader_impl.h>

#include <reflect/type.h>
#include <reflect/function.h>
#include <reflect/scope.h>
#include <reflect/context.h>

#include <adt/hash_map.h>
#include <adt/hash_map_str.h>

#include <stdlib.h>
#include <stdio.h>

#include <Python.h>

typedef struct loader_impl_py_type
{
	PyObject * inspect_module;
	PyObject * inspect_signature;
	PyObject * builtins_module;

	hash_map type_info_map;

} * loader_impl_py;

int function_py_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

void function_py_interface_invoke(function func, function_impl impl, function_args args)
{
	PyObject * py_func = (PyObject *)impl;

	signature s = function_signature(func);

	const size_t args_size = signature_count(s);

	printf("Inner call: args_size(%ld)\n", args_size);

	//if (args_size > 0)
	{
		PyObject * tuple_args = PyTuple_New(args_size);

		PyObject * result = NULL;

		size_t args_count;

		for (args_count = 0; args_count < args_size; ++args_count)
		{
			type t = signature_get_type(s, args_count);

			type_id id = type_index(t);

			PyObject * value = NULL;

			printf("Type (%p): %d\n", (void *)t, id);

			/* TODO: value must be an array, each value *_FromType must be dereferenced */

			if (id == TYPE_INT)
			{
				int * value_ptr = (int *)(args[args_count]);

				printf("Type INT: %d\n", *value_ptr);

				value = PyLong_FromLong(*value_ptr);
			}
			else if (id == TYPE_DOUBLE)
			{
				double * value_ptr = (double *)(args[args_count]);

				printf("Type DOUBLE: %f\n", *value_ptr);

				value = PyFloat_FromDouble(*value_ptr);
			}

			if (value != NULL)
			{
				PyTuple_SetItem(tuple_args, args_count, value);
			}
		}

		printf("Calling function! py_func(%p): ", (void *)py_func);

		PyObject_Print(py_func, stdout, 0);

		printf("\nTuple args (%p): ", (void *)tuple_args);

		PyObject_Print(tuple_args, stdout, 0);

		result = PyObject_CallObject(py_func, tuple_args);

		printf("\nFunction called!\n");

		Py_DECREF(tuple_args);

		if (result != NULL)
		{
			printf("Function call result [%p]: ", (void *)result);

			PyObject_Print(result, stdout, 0);

			printf("\n");

			Py_DECREF(result);
		}
	}
}

void function_py_interface_destroy(function func, function_impl impl)
{
	PyObject * py_func = (PyObject *)impl;

	(void)func;

	Py_DECREF(py_func);
}

function_interface function_py_singleton()
{
	static struct function_interface_type py_interface =
	{
		&function_py_interface_create,
		&function_py_interface_invoke,
		&function_py_interface_destroy
	};

	return &py_interface;
}

PyObject * py_loader_impl_get_builtin(loader_impl_py py_impl, const char * builtin_name)
{
	PyObject * builtin = PyObject_GetAttrString(py_impl->builtins_module, builtin_name);

	if (builtin != NULL && PyType_Check(builtin))
	{
		return builtin;
	}

	Py_XDECREF(builtin);

	return NULL;
}

int py_loader_impl_get_builtin_type(loader_impl_py py_impl, type_id id, const char * name)
{
	PyObject * builtin = py_loader_impl_get_builtin(py_impl, name);

	if (builtin != NULL)
	{
		type builtin_type = type_create(id, name, builtin);

		if (builtin_type != NULL)
		{
			printf("Builtin [%p]: ", (void *)builtin);

			PyObject_Print(builtin, stdout, 0);

			printf("\n");

			if (hash_map_insert(py_impl->type_info_map, (hash_map_key)type_name(builtin_type), (hash_map_value)builtin_type) == 0)
			{
				return 0;
			}

			type_destroy(builtin_type);
		}

		Py_DECREF(builtin);
	}

	return 1;
}

int py_loader_impl_initialize_inspect_types(loader_impl_py py_impl)
{
	PyObject * module_name = PyUnicode_DecodeFSDefault("builtins");

	py_impl->builtins_module = PyImport_Import(module_name);

	Py_DECREF(module_name);

	if (py_impl->builtins_module != NULL)
	{
		py_impl->type_info_map = hash_map_create(&hash_map_cb_hash_str, &hash_map_cb_compare_str);

		if (py_impl->type_info_map != NULL)
		{
			if (py_loader_impl_get_builtin_type(py_impl, TYPE_INT, "int") == 0 &&

				#if PY_MAJOR_VERSION == 2
				py_loader_impl_get_builtin_type(py_impl, TYPE_LONG, "long") == 0 &&
				#endif

				py_loader_impl_get_builtin_type(py_impl, TYPE_DOUBLE, "float") == 0)
			{
				return 0;
			}
		}

		Py_DECREF(py_impl->builtins_module);
	}

	return 1;
}

int py_loader_impl_initialize_inspect(loader_impl_py py_impl)
{
	PyObject * module_name = PyUnicode_DecodeFSDefault("inspect");

	py_impl->inspect_module = PyImport_Import(module_name);

	Py_DECREF(module_name);

	if (py_impl->inspect_module != NULL)
	{
		py_impl->inspect_signature = PyObject_GetAttrString(py_impl->inspect_module, "signature");

		if (py_impl->inspect_signature != NULL && PyCallable_Check(py_impl->inspect_signature))
		{
			if (py_loader_impl_initialize_inspect_types(py_impl) == 0)
			{
				return 0;
			}
		}

		Py_XDECREF(py_impl->inspect_signature);
	}

	Py_DECREF(py_impl->inspect_module);

	return 1;
}

loader_impl_data py_loader_impl_initialize(loader_impl impl)
{
	loader_impl_py py_impl = malloc(sizeof(struct loader_impl_py_type));

	(void)impl;

	if (py_impl != NULL)
	{
		Py_Initialize();

		if (py_loader_impl_initialize_inspect(py_impl) == 0)
		{
			return py_impl;
		}

		free(py_impl);
	}

	return NULL;
}

int py_loader_impl_execution_path(loader_impl impl, loader_naming_path path)
{
	loader_impl_py py_impl = loader_impl_get(impl);

	if (py_impl != NULL)
	{
		PyObject * system_path = PySys_GetObject("path");
		PyObject * current_path = PyUnicode_DecodeFSDefault(path);

		PyList_Append(system_path, current_path);

		Py_DECREF(current_path);

		return 0;
	}

	return 1;
}

loader_handle py_loader_impl_load(loader_impl impl, loader_naming_path path, loader_naming_name name)
{
	PyObject * module_name = PyUnicode_DecodeFSDefault(name);

	PyObject * module = PyImport_Import(module_name);

	Py_DECREF(module_name);

	printf("Python loader (%p) importing %s from (%s) module at (%p)\n", (void *)impl, path, name, (void *)module);

	return (loader_handle)module;
}

int py_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	(void)impl;

	if (handle != NULL)
	{
		PyObject * module = (PyObject *)handle;

		Py_DECREF(module);

		return 0;
	}

	return 1;
}

type py_loader_impl_discover_type(loader_impl_py py_impl, PyObject * annotation)
{
	PyObject * annotation_qualname = PyObject_GetAttrString(annotation, "__qualname__");

	const char * annotation_name = PyUnicode_AsUTF8(annotation_qualname);

	type t = hash_map_get(py_impl->type_info_map, (hash_map_key)annotation_name);

	printf("Discover type (%p) (%p): %s\n", (void *)annotation, (void *)type_derived(t), annotation_name);

	Py_DECREF(annotation_qualname);

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

			if (func_call != NULL)
			{
				func_code = PyObject_GetAttrString(func_call, "__code__");

				Py_DECREF(func_call);
			}
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

		result = PyObject_CallObject(py_impl->inspect_signature, args);
	}

	if (result != NULL)
	{
		PyObject * parameters = PyObject_GetAttrString(result, "parameters");

		if (parameters != NULL && PyMapping_Check(parameters))
		{
			PyObject * parameter_list = PyMapping_Values(parameters);

			if (parameter_list != NULL && PyList_Check(parameter_list))
			{
				Py_ssize_t iterator;

				Py_ssize_t parameter_list_size = PyMapping_Size(parameters);

				signature s = function_signature(f);

				if ((size_t)parameter_list_size != signature_count(s))
				{
					/* error */
				}

				for (iterator = 0; iterator < parameter_list_size; ++iterator)
				{
					PyObject * parameter = PyList_GetItem(parameter_list, iterator);

					if (parameter != NULL)
					{
						PyObject * name = PyObject_GetAttrString(parameter, "name");

						const char * parameter_name = PyUnicode_AsUTF8(name);

						PyObject * annotation = PyObject_GetAttrString(parameter, "annotation");

						printf("Parameter %ld (name: ", iterator);

						PyObject_Print(name, stdout, 0);

						printf(", annotation [%p]: ", (void *)annotation);

						PyObject_Print(annotation, stdout, 0);

						printf(")\n");

						signature_set(s, iterator, parameter_name, py_loader_impl_discover_type(py_impl, annotation));
					}
				}
			}
		}

		return 0;
	}

	return 1;
}

int py_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	PyObject * module = (PyObject *)handle;

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

						function f = function_create(func_name, args_count, value, &function_py_singleton);

						printf("Introspection: function %s, args count %ld\n", func_name, args_count);

						if (py_loader_impl_discover_func(impl, value, f) == 0)
						{
							scope sp = context_scope(ctx);

							scope_define(sp, func_name, f);

							/*
							{
								int a = 15, b = 10;

								function_args args = { &a, &b };

								printf("CALL: \n");

								function_call(f, args);
							}
							*/
						}
					}
				}
			}
		}

		return 0;
	}

	return 1;
}

int py_loader_destroy_type_info_map_cb(hash_map map, hash_map_key key, hash_map_value value, hash_map_cb_iterate_args args)
{
	type t = (type)value;

	PyObject * builtin = (PyObject *)type_derived(t);

	(void)map;
	(void)key;
	(void)args;

	Py_DECREF(builtin);

	type_destroy(t);

	return 0;
}

int py_loader_impl_destroy_inspect_types(loader_impl_py py_impl)
{
	hash_map_iterate(py_impl->type_info_map, &py_loader_destroy_type_info_map_cb, NULL);

	hash_map_destroy(py_impl->type_info_map);

	Py_DECREF(py_impl->builtins_module);

	return 0;
}

int py_loader_impl_destroy_inspect(loader_impl_py py_impl)
{
	Py_DECREF(py_impl->inspect_signature);

	Py_DECREF(py_impl->inspect_module);

	return py_loader_impl_destroy_inspect_types(py_impl);
}

int py_loader_impl_destroy(loader_impl impl)
{
	loader_impl_py py_impl = loader_impl_get(impl);

	if (py_impl != NULL)
	{
		if (py_loader_impl_destroy_inspect(py_impl) != 0)
		{
			/* error */
		}

		Py_Finalize();

		free(py_impl);

		return 0;
	}

	return 1;
}
