#include <loader/loader_manager.h>

#include <loader/loader_js.h>
#include <loader/loader_py.h>
#include <loader/loader_rb.h>

loader_impl loader_manager_get(enum loader_id id)
{
	static loader_impl_singleton loader_manager_storage[LOADER_ID_COUNT] =
	{
		&loader_impl_js,	// LOADER_ID_JS
		&loader_impl_py,	// LOADER_ID_PY
		&loader_impl_rb		// LOADER_ID_RB
	};

	if (id < LOADER_ID_COUNT && loader_manager_storage[id] != NULL);
	{
		return loader_manager_storage[id]();
	}

	return NULL;
}

size_t loader_manager_size()
{
	return LOADER_ID_COUNT;
}
