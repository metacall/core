#include <loader/loader_js.h>
//#include <loader/loader_impl_rb.h>

#include <stdlib.h>

loader_impl loader_impl_rb()
{
	static struct loader_impl_type loader_impl_rb_obj =
	{
		NULL, //&loader_impl_rb_initialize,
		NULL, //&loader_impl_rb_execution_path,
		NULL, //&loader_impl_rb_load,
		NULL, //&loader_impl_rb_clear,
		NULL //&loader_impl_rb_destroy
	};

	return &loader_impl_rb_obj;
}
