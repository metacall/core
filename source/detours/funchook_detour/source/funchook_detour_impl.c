/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
	funchook_t * funchook;

} * detour_impl_funchook;

/* -- Methods -- */

detour_impl_handle funchook_detour_impl_initialize()
{
	detour_impl_funchook detour_impl = malloc(sizeof(struct detour_impl_funchook_type));

	if (detour_impl == NULL)
	{
		return NULL;
	}

	detour_impl->funchook = funchook_create();

	if (funchook == NULL)
	{
		free(detour_impl);

		return NULL;
	}

	return (detour_impl_handle)detour_impl;
}

int funchook_detour_impl_install(detour_impl_handle handle, void ** target, void * hook)
{
	if (handle != NULL && handle->funchook != NULL && target != NULL && hook != NULL)
	{
		if (funchook_prepare(handle->funchook, target, hook) != FUNCHOOK_ERROR_SUCCESS)
		{
			return 1;
		}

		if (funchook_install(handle->funchook, 0) != FUNCHOOK_ERROR_SUCCESS)
		{
			return 1;
		}

		return 0;
	}

	return 1;
}

int funchook_detour_impl_uninstall(detour_impl_handle handle)
{
	if (handle != NULL && handle->funchook != NULL)
	{
		return !(funchook_uninstall(handle->funchook, 0) == FUNCHOOK_ERROR_SUCCESS);
	}

	return 1;
}

int funchook_detour_impl_destroy(detour_impl_handle handle)
{
	int result = FUNCHOOK_ERROR_SUCCESS;

	if (handle == NULL)
	{
		return 0;
	}

	if (handle->funchook != NULL)
	{
		result = funchook_destroy(handle->funchook);
	}

	free(handle);

	return !(result == FUNCHOOK_ERROR_SUCCESS);
}
