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

int main(int argc, char *argv[])
{
	PyObject *system_path, *current_path;

	Py_Initialize();

	system_path = PySys_GetObject("path");

	current_path = PyUnicode_DecodeFSDefault(".");

	PyList_Append(system_path, current_path);

	Py_DECREF(current_path);

	simple_script();

	example_script(&multiply_callback);

	Py_Finalize();

	return 0;
}
