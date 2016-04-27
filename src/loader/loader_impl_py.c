#include <loader/loader_naming.h>
#include <loader/loader_impl_py.h>
#include <loader/loader_impl_py_discovering.h>

#define LOADER_IMPL_PY_EXECUTION_PATH_DEFAULT	"public/"
#define LOADER_IMPL_PY_EXECUTION_PATH_CURRENT	"."

int loader_impl_py_initialize()
{
	Py_Initialize();

	if (loader_impl_py_execution_path(NULL))
	{
		printf("error: when defining current execution path\n");
	}

	if (loader_impl_py_execution_path(LOADER_IMPL_PY_EXECUTION_PATH_DEFAULT))
	{
		printf("error: when defining default execution path\n");
	}

	return loader_impl_py_discovering_initialize();
}

int loader_impl_py_execution_path(char * execution_path)
{
	PyObject * system_path = PySys_GetObject("path");

	if (system_path != NULL)
	{
		PyObject * execution_path_obj;

		if (execution_path == NULL)
		{
			execution_path = LOADER_IMPL_PY_EXECUTION_PATH_CURRENT;
		}

		execution_path_obj = PyUnicode_DecodeFSDefault(execution_path);

		if (execution_path_obj != NULL)
		{
			PyList_Append(system_path, execution_path_obj);

			printf("execution path %s added\n", execution_path);

			Py_DECREF(execution_path_obj);

			return 0;
		}

		printf("error: invalid execution path %s\n", execution_path);

		return 2;
	}

	printf("error: invalid system path\n");

	return 1;
}

loader_handle loader_impl_py_load(char * path)
{
	if (path != NULL)
	{
		loader_naming_name name;

		PyObject * module_name;

		loader_naming_get_name(path, name);

		printf("debug: path %s, name %s\n", path, name);

		module_name = PyUnicode_DecodeFSDefault(name);

		if (module_name != NULL)
		{
			PyObject * module_import = PyImport_Import(module_name);

			Py_DECREF(module_name);

			if (module_import != NULL)
			{
				printf("debug: module %s imported\n", name);

				return (loader_handle)module_import;
			}
		}
	}

	printf("error: module %s not found\n", path);

	return NULL;
}

int loader_impl_py_clear(loader_handle handle)
{
	if (handle != NULL)
	{
		PyObject * module_import = (PyObject *)handle;

		Py_DECREF(module_import);
	}

	return 0;
}

int loader_impl_py_discover(loader_handle handle)
{
	if (handle != NULL && PyModule_Check(handle))
	{
		PyObject * dict = PyModule_GetDict(handle);

		if (dict != NULL)
		{
			PyObject * key, * value;
			Py_ssize_t position = 0;

			while (PyDict_Next(dict, &position, &key, &value))
			{
				const char * key_str = PyUnicode_AsUTF8(key);

				// check if is a function
				if (PyCallable_Check(value))
				{
					int args_count = loader_impl_py_function_args_count(value);

					printf("debug: function %s argument count %d ", key_str, args_count);

					PyObject_Print(value, stdout, 0);

					printf("\n");

					// apply introspection
					loader_impl_py_function_inspect(value, args_count);
				}
			}

			Py_DECREF(dict);

			return 0;
		}

		printf("error: invalid module handle dictionary\n");

		return 1;
	}

	printf("error: invalid module handle\n");

	return 1;
}

int loader_impl_py_destroy()
{
	int result = 0;

	if (loader_impl_py_discovering_destroy())
	{
		printf("error: invalid discovering destruction\n");

		result = 1;
	}

	Py_Finalize();

	return result;
}
