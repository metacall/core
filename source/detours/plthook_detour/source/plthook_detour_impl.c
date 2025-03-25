/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

/* -- Headers -- */

#include <plthook_detour/plthook_detour_impl.h>

#include <plthook.h>

/* -- Methods -- */

int plthook_detour_impl_initialize_file(detour_impl_handle *handle, const char *path)
{
	plthook_t *plthook;
	int result;

	if (handle == NULL)
	{
		return 1;
	}

	result = plthook_open(&plthook, path);

	if (result != PLTHOOK_SUCCESS)
	{
		*handle = NULL;
		return result;
	}

	*handle = (void *)plthook;
	return 0;
}

int plthook_detour_impl_initialize_handle(detour_impl_handle *handle, dynlink library)
{
	plthook_t *plthook;
	int result;

	if (handle == NULL)
	{
		return 1;
	}

	result = plthook_open_by_handle(&plthook, dynlink_get_impl(library));

	if (result != PLTHOOK_SUCCESS)
	{
		*handle = NULL;
		return result;
	}

	*handle = (void *)plthook;
	return 0;
}

int plthook_detour_impl_initialize_address(detour_impl_handle *handle, void (*address)(void))

{
	plthook_t *plthook;
	void *ptr;
	int result;

	if (handle == NULL)
	{
		return 1;
	}

	dynlink_symbol_uncast(address, ptr);

	result = plthook_open_by_address(&plthook, ptr);

	if (result != PLTHOOK_SUCCESS)
	{
		*handle = NULL;
		return result;
	}

	*handle = (void *)plthook;
	return 0;
}

int plthook_detour_impl_enumerate(detour_impl_handle handle, unsigned int *position, const char **name, void ***address)
{
	if (handle == NULL)
	{
		return 1;
	}

	return plthook_enum(handle, position, name, address);
}

int plthook_detour_impl_replace(detour_impl_handle handle, const char *function_name, void (*function_addr)(void), void **function_old_addr)
{
	void *ptr;

	if (handle == NULL)
	{
		return 1;
	}

	dynlink_symbol_uncast(function_addr, ptr);

	return plthook_replace(handle, function_name, ptr, function_old_addr);
}

const char *plthook_detour_impl_error(detour_impl_handle handle)
{
	/* TODO: The error should be stored in the handle, this must be modified from plthook library itself */
	(void)handle;

	return plthook_error();
}

void plthook_detour_impl_destroy(detour_impl_handle handle)
{
	plthook_close(handle);
}
