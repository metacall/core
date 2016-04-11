#include <python_backend.h>
#include <Python.h>

void simple_script(void)
{
	PyRun_SimpleString(
		"from time import time, ctime;\n"
		"print('Hello World! Today is', ctime(time()));\n"
	);
}

void example_script(void (*callback)(PyObject *))
{
	PyObject *module_name = PyUnicode_DecodeFSDefault("example");
	PyObject *example_module = PyImport_Import(module_name);

	Py_DECREF(module_name);

	if (example_module)
	{
		PyObject *func_multiply = PyObject_GetAttrString(example_module, "multiply");

		if (func_multiply && PyCallable_Check(func_multiply))
		{
			printf("Function loaded correctly\n");

			if (callback)
			{
				callback(func_multiply);
			}

			Py_DECREF(func_multiply);
		}
		else
		{
			printf("Function not found\n");

			Py_XDECREF(func_multiply);
		}

		Py_DECREF(example_module);
	}
	else
	{
		printf("Module not loaded\n");
	}
}

void multiply_callback(PyObject * func)
{
	if (func)
	{
		const int args_size = 2;
		int arg_count;

		PyObject *args = PyTuple_New(args_size);
		PyObject *result;

		for (arg_count = 0; arg_count < args_size; ++arg_count)
		{
			PyObject *value = PyLong_FromLong(arg_count + 3);

			if (value)
			{
				PyTuple_SetItem(args, arg_count, value);
			}
		}

		result = PyObject_CallObject(func, args);

		Py_DECREF(args);

		if (result)
		{
			printf("Function call result: %ld\n", PyLong_AsLong(result));

			Py_DECREF(result);
		}
	}
}

int discovering_func_args_count(PyObject *func)
{
	int arg_count = -1;

	// Check if has a callable section
	if (PyObject_HasAttrString(func, "__call__"))
	{
		// Assert
		//int expected_args = 2;

		PyObject *func_code = NULL;

		if (PyObject_HasAttrString(func, "__code__"))
		{
			// It is a function
			func_code = PyObject_GetAttrString(func, "__code__");

			// Increment argument count for self if need
			if (PyObject_HasAttrString(func, "im_self"))
			{
				//++expected_args;
			}
		}
		else
		{
			// It is a function object
			PyObject *call_section = PyObject_GetAttrString(func, "__call__");

			if (call_section)
			{
				// Get func code
				func_code = PyObject_GetAttrString(call_section, "__code__");

				// Increment for self argument
				//++expected_args;

				Py_DECREF(call_section);
			}
		}

		if (func_code)
		{
			PyObject *co_argcount = PyObject_GetAttrString(func_code, "co_argcount");

			if (co_argcount)
			{
				arg_count = PyLong_AsLong(co_argcount);

				Py_DECREF(co_argcount);
			}

			Py_DECREF(func_code);
		}

		// Assert
		/*
		if (arg_count != -1)
		{
			if (arg_count > expected_args)
			{
				printf("Too many arguments (expected args: %d)\n", expected_args);
			}
			else if (arg_count < expected_args)
			{
				PyObject *flags = PyObject_GetAttrString(func_code, "co_flags");

				if (flags)
				{
					int flags_mask = PyLong_AsLong(flags);

					if (flags_mask & 4)
					{
						printf("Hanlder has too few args, expected 1\n");
					}

					Py_DECREF(flags);
				}
			}
		}
		*/
	}

	return arg_count;
}

void discovering_inspection(PyObject *func, int args_count)
{
	if (func && args_count >= 0)
	{
		PyObject *module_name = PyUnicode_DecodeFSDefault("inspect");
		PyObject *inspect_module = PyImport_Import(module_name);

		Py_DECREF(module_name);

		if (inspect_module)
		{
			PyObject *func_signature = PyObject_GetAttrString(inspect_module, "signature");

			if (func_signature && PyCallable_Check(func_signature))
			{
				PyObject *args = PyTuple_New(1);
				PyObject *result = NULL;

				printf("Function loaded correctly\n");

				if (args)
				{
					PyTuple_SetItem(args, 0, func);

					printf("Calling function signature\n");

					result = PyObject_CallObject(func_signature, args);

					Py_DECREF(args);
				}

				if (result)
				{
					PyObject *str_result = PyObject_Str(result);

					// Get parameters
					PyObject *parameters = PyObject_GetAttrString(result, "parameters");

					printf("There is a result man!\n");

					PyObject_Print(result, stdout, 0);

					PyObject_Print(str_result, stdout, 0);

					printf("\n\n");

					if (parameters && PyMapping_Check(parameters))
					{
						PyObject *parameter_list = PyMapping_Values(parameters);

						Py_ssize_t parameter_list_size = PyMapping_Size(parameters);

						printf("++ parameters implementation object:\n");

						PyObject_Print(parameters, stdout, 0);

						printf("\n++ parameter_list implementation object:\n");

						PyObject_Print(parameter_list, stdout, 0);

						printf("\n");

						if (parameter_list && PyList_Check(parameter_list))
						{
							Py_ssize_t iterator;

							for (iterator = 0; iterator < parameter_list_size; ++iterator)
							{
								PyObject *parameter = PyList_GetItem(parameter_list, iterator);

								if (parameter)
								{
									PyObject *name = PyObject_GetAttrString(parameter, "name");
									PyObject *annotation = PyObject_GetAttrString(parameter, "annotation");

									printf("Parameter %ld:\n", iterator);

									PyObject_Print(parameter, stdout, 0);

									printf("\n -> Name: ");

									PyObject_Print(name, stdout, 0);

									printf("\n -> Annotation: ");

									PyObject_Print(annotation, stdout, 0);

									printf("\n\n");
								}
							}

						}

						Py_DECREF(parameters);
					}

					printf("\n\n");

					Py_DECREF(result);
				}

				Py_DECREF(func_signature);
			}
			else
			{
				printf("Function not found\n");

				Py_XDECREF(func_signature);
			}

			Py_DECREF(inspect_module);
		}
		else
		{
			printf("Module not loaded\n");
		}

	}
}

void discovering(void)
{
	PyObject *module_name = PyUnicode_DecodeFSDefault("example");
	PyObject *example_module = PyImport_Import(module_name);

	Py_DECREF(module_name);

	if (example_module && PyModule_Check(example_module))
	{
		PyObject *dict = PyModule_GetDict(example_module);

		printf("Module loaded correctly\n");

		if (dict)
		{
			PyObject *key, *value;
			Py_ssize_t position = 0;

			printf("Discovering script API:\n");

			while (PyDict_Next(dict, &position, &key, &value))
			{
				const char *key_str = PyUnicode_AsUTF8(key);

				// check if is a function
				if (PyCallable_Check(value))
				{
					int args_count = discovering_func_args_count(value);

					// Print
					printf("Name [%s] - Func [", key_str);

					PyObject_Print(value, stdout, 0);

					// Get argument size
					printf("] - ArgsC [%d]\n\n", args_count);

					// Apply introspection
					discovering_inspection(value, args_count);
				}
			}

			Py_DECREF(dict);
		}

		Py_DECREF(example_module);
	}
	else
	{
		printf("Module not loaded\n");
	}
}

void python_run_example()
{
	PyObject *system_path, *current_path;

	Py_Initialize();

	system_path = PySys_GetObject("path");

	current_path = PyUnicode_DecodeFSDefault(".");

	PyList_Append(system_path, current_path);

	Py_DECREF(current_path);

	simple_script();

	example_script(&multiply_callback);

	discovering();

	Py_Finalize();
}
