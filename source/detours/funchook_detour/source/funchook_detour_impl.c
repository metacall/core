/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

/* -- Headers -- */

#include <funchook_detour/funchook_detour_impl.h>

#include <log/log.h>

#include <funchook.h>

/* -- Methods -- */

detour_impl_handle funchook_detour_impl_initialize(void)
{
	return (detour_impl_handle)funchook_create();
}

int funchook_detour_impl_install(detour_impl_handle handle, void (**target)(void), void (*hook)(void))
{
	funchook_t *handle_impl = handle;

	if (handle_impl != NULL && target != NULL && hook != NULL)
	{
		void **hook_ptr = (void **)&hook;

		if (funchook_prepare(handle_impl, (void **)target, (void *)*hook_ptr) != FUNCHOOK_ERROR_SUCCESS)
		{
			return 1;
		}

		if (funchook_install(handle_impl, 0) != FUNCHOOK_ERROR_SUCCESS)
		{
			return 1;
		}

		return 0;
	}

	return 1;
}

int funchook_detour_impl_uninstall(detour_impl_handle handle)
{
	funchook_t *handle_impl = handle;

	if (handle_impl != NULL)
	{
		return !(funchook_uninstall(handle_impl, 0) == FUNCHOOK_ERROR_SUCCESS);
	}

	return 1;
}

int funchook_detour_impl_destroy(detour_impl_handle handle)
{
	funchook_t *handle_impl = handle;

	if (handle_impl == NULL)
	{
		return 0;
	}

	return !(funchook_destroy(handle_impl) == FUNCHOOK_ERROR_SUCCESS);
}
