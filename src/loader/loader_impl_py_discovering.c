#include <loader/loader_impl_py_discovering.h>

PyObject ** loader_impl_py_inspect()
{
	static PyObject * inspect_module = NULL;

	return &inspect_module;
}

PyObject ** loader_impl_py_inspect_signature()
{
	static PyObject * func_signature = NULL;

	return &func_signature;
}

int loader_impl_py_discovering_initialize()
{
	PyObject ** inspect = loader_impl_py_inspect();
	PyObject ** signature = loader_impl_py_inspect_signature();

	if (*inspect == NULL)
	{
		PyObject * module_name = PyUnicode_DecodeFSDefault("inspect");
		PyObject * inspect_module = PyImport_Import(module_name);

		Py_DECREF(module_name);

		if (inspect_module)
		{
			*inspect = inspect_module;
		}
		else
		{
			printf("error: inspect module not loaded\n");

			return 1;
		}
	}

	if (*inspect && *signature == NULL)
	{
		PyObject * func_signature = PyObject_GetAttrString(*inspect, "signature");

		if (func_signature && PyCallable_Check(func_signature))
		{
			*signature = func_signature;
		}
		else
		{
			printf("error: function signature not loaded\n");

			return 1;
		}
	}

	return 0;
}

int loader_impl_py_discovering_destroy()
{
	int result = 0;

	PyObject ** inspect = loader_impl_py_inspect();
	PyObject ** signature = loader_impl_py_inspect_signature();

	if (*signature != NULL)
	{
		PyObject * func_signature = *signature;

		printf("debug: signature pointer %p\n", *signature);

		if (func_signature && PyCallable_Check(func_signature))
		{
			Py_DECREF(func_signature);
		}
		else
		{
			Py_XDECREF(func_signature);

			printf("error: function signature not destroyed properly\n");

			result = 1;
		}
	}

	if (*inspect != NULL)
	{
		PyObject * inspect_module = *inspect;

		printf("debug: inspect pointer %p\n", *inspect);

		Py_DECREF(inspect_module);

		if (inspect_module)
		{
			printf("error: inspect module not destroyed\n");

			result = 1;
		}

		*inspect = NULL;
	}

	return result;
}

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
		PyObject ** signature = loader_impl_py_inspect_signature();

		if (*signature != NULL)
		{
			PyObject * args = PyTuple_New(1);
			PyObject * result = NULL;

			if (args)
			{
				PyTuple_SetItem(args, 0, func);

				result = PyObject_CallObject(*signature, args);

				Py_DECREF(args);
			}

			if (result)
			{
				PyObject * str_result = PyObject_Str(result);

				PyObject * parameters = PyObject_GetAttrString(result, "parameters");

				printf("debug: function signature result ");

				PyObject_Print(result, stdout, 0);

				printf("\ndebug: ");

				PyObject_Print(str_result, stdout, 0);

				printf("\n");

				if (parameters && PyMapping_Check(parameters))
				{
					PyObject * parameter_list = PyMapping_Values(parameters);

					Py_ssize_t parameter_list_size = PyMapping_Size(parameters);

					printf("debug: function signature parameters ");

					PyObject_Print(parameters, stdout, 0);

					printf("\ndebug: ");

					PyObject_Print(parameter_list, stdout, 0);

					printf("\n");

					if (parameter_list && PyList_Check(parameter_list))
					{
						Py_ssize_t iterator;

						for (iterator = 0; iterator < parameter_list_size; ++iterator)
						{
							PyObject * parameter = PyList_GetItem(parameter_list, iterator);

							if (parameter)
							{
								PyObject * name = PyObject_GetAttrString(parameter, "name");
								PyObject * annotation = PyObject_GetAttrString(parameter, "annotation");

								printf("debug: parameter %zu ", iterator);

								PyObject_Print(parameter, stdout, 0);

								printf("\ndebug: parameter name ");

								PyObject_Print(name, stdout, 0);

								printf("\ndebug: parameter annotation ");

								PyObject_Print(annotation, stdout, 0);

								printf("\n");
							}
						}
					}

					Py_DECREF(parameters);
				}

				Py_DECREF(result);
			}
		}
	}

	return 0;
}
