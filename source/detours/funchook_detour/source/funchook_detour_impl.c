/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

/* -- Headers -- */

#include <funchook_detour/funchook_detour_impl.h>

#include <log/log.h>

#include <funchook.h>

/* -- Type Definitions -- */

typedef struct detour_impl_funchook_type
{
	funchook_t *funchook;

} * detour_impl_funchook;

/* -- Methods -- */

detour_impl_handle funchook_detour_impl_initialize(void)
{
	detour_impl_funchook detour_impl;

	detour_impl = malloc(sizeof(struct detour_impl_funchook_type));

	if (detour_impl == NULL)
	{
		return NULL;
	}

	detour_impl->funchook = funchook_create();

	if (detour_impl->funchook == NULL)
	{
		free(detour_impl);

		return NULL;
	}

	return (detour_impl_handle)detour_impl;
}

int funchook_detour_impl_install(detour_impl_handle handle, void (**target)(void), void (*hook)(void))
{
	detour_impl_funchook handle_impl = handle;

	if (handle_impl != NULL && handle_impl->funchook != NULL && target != NULL && hook != NULL)
	{
		void **hook_ptr = (void **)&hook;

		if (funchook_prepare(handle_impl->funchook, (void **)target, (void *)*hook_ptr) != FUNCHOOK_ERROR_SUCCESS)
		{
			return 1;
		}

		if (funchook_install(handle_impl->funchook, 0) != FUNCHOOK_ERROR_SUCCESS)
		{
			return 1;
		}

		return 0;
	}

	return 1;
}

int funchook_detour_impl_uninstall(detour_impl_handle handle)
{
	detour_impl_funchook handle_impl = handle;

	if (handle_impl != NULL && handle_impl->funchook != NULL)
	{
		return !(funchook_uninstall(handle_impl->funchook, 0) == FUNCHOOK_ERROR_SUCCESS);
	}

	return 1;
}

int funchook_detour_impl_destroy(detour_impl_handle handle)
{
	detour_impl_funchook handle_impl = handle;

	int result = FUNCHOOK_ERROR_SUCCESS;

	if (handle_impl == NULL)
	{
		return 0;
	}

	if (handle_impl->funchook != NULL)
	{
		result = funchook_destroy(handle_impl->funchook);
	}

	free(handle_impl);

	return !(result == FUNCHOOK_ERROR_SUCCESS);
}
