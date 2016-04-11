#include <loader/loader_impl_py.h>
#include <loader/loader_impl_py_discovering.h>

#define LOADER_IMPL_PY_EXECUTION_PATH_DEFAULT	"code"
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

	return 0;
}

int loader_impl_py_execution_path(char * execution_path)
{
	PyObject * system_path = PySys_GetObject("path");

	if (system_path)
	{
		PyObject * execution_path_obj;

		if (execution_path)
		{
			execution_path_obj = PyUnicode_DecodeFSDefault(execution_path);
		}
		else
		{
			execution_path_obj = PyUnicode_DecodeFSDefault(LOADER_IMPL_PY_EXECUTION_PATH_CURRENT);
		}

		if (execution_path_obj)
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

loader_handle loader_impl_py_load(char * name)
{
	if (name)
	{
		PyObject * module_name = PyUnicode_DecodeFSDefault(name);

		if (module_name)
		{
			PyObject * module_import = PyImport_Import(module_name);

			Py_DECREF(module_name);

			if (module_import)
			{
				return (loader_handle)module_import;
			}
		}
	}

	return NULL;
}

int loader_impl_py_clear(loader_handle handle)
{
	if (handle)
	{
		PyObject * module_import = (PyObject *)handle;

		Py_DECREF(module_import);
	}

	return 0;
}

int loader_impl_py_destroy()
{
	Py_Finalize();

	return 0;
}

