#include <loader/loader_py.h>
#include <loader/loader_impl_py.h>

loader_impl loader_impl_py()
{
	static struct loader_impl_type loader_impl_py_obj =
	{
		&loader_impl_py_initialize,
		&loader_impl_py_execution_path,
		&loader_impl_py_load,
		&loader_impl_py_clear,
		&loader_impl_py_destroy
	};

	return &loader_impl_py_obj;
}
