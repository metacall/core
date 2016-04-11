#include <loader/loader_impl_py_discovering.h>

int loader_impl_py_function_args_count(PyObject * func)
{
	int args_count = -1;

	// check if has a callable section
	if (PyObject_HasAttrString(func, "__call__"))
	{
		// assert
		int expected_args = 0;

		PyObject * func_code = NULL;

		if (PyObject_HasAttrString(func, "__code__"))
		{
			// it is a function
			func_code = PyObject_GetAttrString(func, "__code__");

			// increment argument count for self if need
			if (PyObject_HasAttrString(func, "im_self"))
			{
				++expected_args;
			}
		}
		else
		{
			// it is a function object
			PyObject * call_section = PyObject_GetAttrString(func, "__call__");

			if (call_section)
			{
				// get func code
				func_code = PyObject_GetAttrString(call_section, "__code__");

				// increment for self argument
				++expected_args;

				Py_DECREF(call_section);
			}
		}

		if (func_code)
		{
			PyObject * co_argcount = PyObject_GetAttrString(func_code, "co_argcount");

			if (co_argcount)
			{
				args_count = PyLong_AsLong(co_argcount);

				Py_DECREF(co_argcount);
			}

			Py_DECREF(func_code);
		}

		// assert
		if (args_count != -1)
		{
			if (args_count > expected_args)
			{
				printf("warning: too many arguments (args: %d) (expected args: %d)\n", args_count, expected_args);
			}
			else if (args_count < expected_args)
			{
				PyObject * flags = PyObject_GetAttrString(func_code, "co_flags");

				if (flags)
				{
					int flags_mask = PyLong_AsLong(flags);

					if (flags_mask & 4)
					{
						printf("warning: hanlder has too few arguments (args: %d) (expected args: %d)\n", args_count, expected_args);
					}

					Py_DECREF(flags);
				}
			}
		}
	}

	return args_count;
}

int loader_impl_py_function_inspect(PyObject * func, int args_count)
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
