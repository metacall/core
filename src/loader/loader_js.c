#include <loader/loader_js.h>
//#include <loader/loader_impl_js.h>

#include <stdlib.h>

loader_impl loader_impl_js()
{
	static struct loader_impl_type loader_impl_js_obj =
	{
		NULL, //&loader_impl_js_initialize,
		NULL, //&loader_impl_js_execution_path,
		NULL, //&loader_impl_js_load,
		NULL, //&loader_impl_js_clear,
		NULL //&loader_impl_js_destroy
	};

	return &loader_impl_js_obj;
}
