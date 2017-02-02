/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading python code at run-time into a process.
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

typedef struct loader_impl_py_function_type
{
	PyObject * func;
	PyObject ** values;

} *loader_impl_py_function;

typedef struct loader_impl_py_type
{
	PyObject * inspect_module;
	PyObject * inspect_signature;
	PyObject * builtins_module;
	PyObject * main_module;

} *loader_impl_py;

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

function_return function_py_interface_invoke(function func, function_impl impl, function_args args)
{
	loader_impl_py_function py_func = (loader_impl_py_function)impl;

	signature s = function_signature(func);

	const size_t args_size = signature_count(s);

	type ret_type = signature_get_return(s);

	PyObject * tuple_args = PyTuple_New(args_size);

	PyObject * result = NULL;

	size_t args_count;

	for (args_count = 0; args_count < args_size; ++args_count)
	{
		type t = signature_get_type(s, args_count);

		type_id id = type_index(t);

		log_write("metacall", LOG_LEVEL_DEBUG, "Type (%p): %d", (void *)t, id);

		if (id == TYPE_BOOL)
		{
			boolean * value_ptr = (boolean *)(args[args_count]);

			long l = (*value_ptr == 0) ? 0L : 1L;

			py_func->values[args_count] = PyBool_FromLong(l);
		}
		else if (id == TYPE_INT)
		{
			int * value_ptr = (int *)(args[args_count]);

#if PY_MAJOR_VERSION == 2
			py_func->values[args_count] = PyInt_FromLong(*value_ptr);
#elif PY_MAJOR_VERSION == 3
			long l = (long)(*value_ptr);

			py_func->values[args_count] = PyLong_FromLong(l);
#endif
		}
		else if (id == TYPE_LONG)
		{
			long * value_ptr = (long *)(args[args_count]);

			py_func->values[args_count] = PyLong_FromLong(*value_ptr);
		}
		else if (id == TYPE_FLOAT)
		{
			float * value_ptr = (float *)(args[args_count]);

			py_func->values[args_count] = PyFloat_FromDouble((double)*value_ptr);
		}
		else if (id == TYPE_DOUBLE)
		{
			double * value_ptr = (double *)(args[args_count]);

			py_func->values[args_count] = PyFloat_FromDouble(*value_ptr);
		}
		else if (id == TYPE_STRING)
		{
			const char * value_ptr = (const char *)(args[args_count]);

#if PY_MAJOR_VERSION == 2
			py_func->values[args_count] = PyString_FromString(value_ptr);
#elif PY_MAJOR_VERSION == 3
			py_func->values[args_count] = PyUnicode_FromString(value_ptr);
#endif

		}
		else if (id == TYPE_PTR)
		{
			/* TODO */
		}

		if (py_func->values[args_count] != NULL)
		{
			PyTuple_SetItem(tuple_args, args_count, py_func->values[args_count]);
		}
	}

	result = PyObject_CallObject(py_func->func, tuple_args);

	Py_DECREF(tuple_args);

	if (result != NULL && ret_type != NULL)
	{
		value v = NULL;

		type_id id = type_index(ret_type);

		log_write("metacall", LOG_LEVEL_DEBUG, "Return type %p, %d", (void *)ret_type, id);

		if (id == TYPE_BOOL)
		{
			boolean b = (PyObject_IsTrue(result) == 1) ? 1 : 0;

			v = value_create_bool(b);
		}
		else if (id == TYPE_INT)
		{
#if PY_MAJOR_VERSION == 2
			long l = PyInt_AsLong(result);
#elif PY_MAJOR_VERSION == 3
			long l = PyLong_AsLong(result);
#endif

			/* TODO: Review overflow */
			int i = (int)l;

			v = value_create_int(i);
		}
		else if (id == TYPE_LONG)
		{
			long l = PyLong_AsLong(result);

			v = value_create_long(l);
		}
		else if (id == TYPE_FLOAT)
		{
			double d = PyFloat_AsDouble(result);

			v = value_create_float((float)d);
		}
		else if (id == TYPE_DOUBLE)
		{
			double d = PyFloat_AsDouble(result);

			v = value_create_double(d);
		}
		else if (id == TYPE_STRING)
		{
			char * str = NULL;
			Py_ssize_t length = 0;

#if PY_MAJOR_VERSION == 2
			if (PyString_AsStringAndSize(result, &str, &length) == -1)
			{
				/* error */
			}
#elif PY_MAJOR_VERSION == 3
			str = PyUnicode_AsUTF8AndSize(result, &length);
#endif

			v = value_create_string(str, (size_t)length);
		}
		else if (id == TYPE_PTR)
		{
			/* TODO */
		}
		else
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Unrecognized return type");
		}

		Py_DECREF(result);

		return v;
	}

	return NULL;
}

void function_py_interface_destroy(function func, function_impl impl)
{
	loader_impl_py_function py_func = (loader_impl_py_function)impl;

	if (py_func != NULL)
	{
		if (py_func->values != NULL)
		{
			signature s = function_signature(func);

			const size_t args_size = signature_count(s);

			size_t iterator;

			for (iterator = 0; iterator < args_size; ++iterator)
			{
				if (py_func->values[iterator] != NULL)
				{
					/* Py_DECREF(py_func->values[iterator]); */
				}
			}


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
		&function_py_interface_destroy
	};

	return &py_function_interface;
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

int py_loader_impl_get_builtin_type(loader_impl impl, loader_impl_py py_impl, type_id id, const char * name)
{
	PyObject * builtin = py_loader_impl_get_builtin(py_impl, name);

	if (builtin != NULL)
	{
		type builtin_type = type_create(id, name, builtin, &type_py_singleton);

		if (builtin_type != NULL)
		{
			/*
			log_write("metacall", LOG_LEVEL_DEBUG, "Builtin [%p]: ", (void *)builtin);

			PyObject_Print(builtin, stdout, 0);

			log_write("metacall", LOG_LEVEL_DEBUG, "");
			*/

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
			{ TYPE_PTR, "bytearray" }
		};

		size_t index, size = sizeof(type_id_name_pair) / sizeof(type_id_name_pair[0]);

		for (index = 0; index < size; ++index)
		{
			if (py_loader_impl_get_builtin_type(impl, py_impl,
				type_id_name_pair[index].id,
				type_id_name_pair[index].name) != 0)
			{
				/* error */

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

	Py_DECREF(module_name);

	if (py_impl->inspect_module != NULL)
	{
		py_impl->inspect_signature = PyObject_GetAttrString(py_impl->inspect_module, "signature");

		if (py_impl->inspect_signature != NULL && PyCallable_Check(py_impl->inspect_signature))
		{
			if (py_loader_impl_initialize_inspect_types(impl, py_impl) == 0)
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

		if (py_loader_impl_initialize_inspect(impl, py_impl) == 0)
		{
			PyModuleDef module_def = { 0 };

			module_def.m_name = "__metacall__";

			PyObject* py_module_def = PyModuleDef_Init(&module_def);

			py_impl->main_module = PyModule_Create(py_module_def);
			
			Py_IncRef(py_impl->main_module);

			if (py_impl->main_module != NULL) {
				return py_impl;
			}
		}

		free(py_impl);
	}

	return NULL;
}

int py_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
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

loader_handle py_loader_impl_load_from_files(loader_impl impl, const loader_naming_name name, const loader_naming_path path[], size_t size)
{
	loader_impl_py py_impl = loader_impl_get(impl);

	PyObject* main_dict = PyModule_GetDict(py_impl->main_module);

	loader_naming_name module_name;

	char location_path[512] = { 0 };

	for (size_t i = 0; i < size; i++)
	{
		loader_path_get_name(path[i], module_name);
		PyObject* py_module_name = PyUnicode_DecodeFSDefault(module_name);

		strncpy(location_path, path[i], strlen(path[i]) - (strlen(module_name) + 3));
		
		/* ugly wave*/
		
		PyObject * system_path = PySys_GetObject("path");
		PyObject * current_path = PyUnicode_DecodeFSDefault(location_path);

		PyList_Append(system_path, current_path);

		Py_DECREF(current_path);

		PyObject * module = PyImport_Import(py_module_name);
		PyObject * module_dict = PyModule_GetDict(module);
		PyDict_Merge(main_dict, module_dict, 0);
	}


	return (loader_handle)py_impl->main_module;
}

loader_handle py_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const loader_naming_extension extension, const char * buffer, size_t size)
{
	PyObject * compiled = Py_CompileString(buffer, name, Py_file_input);

	(void)size;

	if (compiled != NULL)
	{
		PyObject * module = PyImport_ExecCodeModule(name, compiled);

		log_write("metacall", LOG_LEVEL_DEBUG, "Python loader (%p) importing %s.%s from memory module at (%p)", (void *)impl, name, extension, (void *)module);

		return (loader_handle)module;
	}
	else
	{
		PyErr_Print();
	}

	return NULL;
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

type py_loader_impl_discover_type(loader_impl impl, PyObject * annotation)
{
	PyObject * annotation_qualname = PyObject_GetAttrString(annotation, "__qualname__");

	const char * annotation_name = PyUnicode_AsUTF8(annotation_qualname);

	type t = loader_impl_type(impl, annotation_name);

	log_write("metacall", LOG_LEVEL_DEBUG, "Discover type (%p) (%p): %s", (void *)annotation, (void *)type_derived(t), annotation_name);

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
		PyErr_Print();
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
					/* error */

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

						/*
						log_write("metacall", LOG_LEVEL_DEBUG, "Parameter %ld (name: ", iterator);

						PyObject_Print(name, stdout, 0);

						log_write("metacall", LOG_LEVEL_DEBUG, ", annotation [%p]: ", (void *)annotation);

						PyObject_Print(annotation, stdout, 0);

						log_write("metacall", LOG_LEVEL_DEBUG, ")");
						*/

						signature_set(s, iterator, parameter_name, py_loader_impl_discover_type(impl, annotation));

					}
				}
			}
		}

		signature_set_return(s, py_loader_impl_discover_type(impl, return_annotation));

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

			PyObject * key, *value;

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
							return 1;
						}

						py_func->func = value;

						f = function_create(func_name, args_count, py_func, &function_py_singleton);

						log_write("metacall", LOG_LEVEL_DEBUG, "Introspection: function %s, args count %ld", func_name, args_count);

						if (py_loader_impl_discover_func(impl, value, f) == 0)
						{
							scope sp = context_scope(ctx);

							scope_define(sp, func_name, f);
						}
					}
				}
			}
		}

		return 0;
	}

	return 1;
}

int py_loader_impl_destroy(loader_impl impl)
{
	loader_impl_py py_impl = loader_impl_get(impl);

	if (py_impl != NULL)
	{
		Py_DECREF(py_impl->inspect_signature);

		Py_DECREF(py_impl->inspect_module);

		Py_DECREF(py_impl->builtins_module);

		Py_DECREF(py_impl->main_module);

		Py_Finalize();

		free(py_impl);

		return 0;
	}

	return 1;
}
