/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

/* -- Headers -- */

#include <funhook_detour/funhook_detour_impl.h>

#include <log/log.h>

#include <funhook.h>

/* -- Type Definitions -- */

typedef struct detour_impl_funhook_type
{
	funchook_t * funchook;
	void * target;

} * detour_impl_funhook;

/* -- Methods -- */

detour_impl_handle funhook_detour_impl_initialize()
{
	detour_impl_funhook detour_impl = malloc(sizeof(struct detour_impl_funhook_type));

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

	detour_impl->target = NULL; 

	return (detour_impl_handle)detour_impl;
}

int funhook_detour_impl_install(detour_impl_handle handle, void ** target, void * hook)
{
	if (handle != NULL && handle->funhook != NULL && target != NULL && hook != NULL)
	{
		if (funchook_prepare(handle->funhook, target, hook) != FUNCHOOK_ERROR_SUCCESS)
		{
			return 1;
		}

		if (funchook_install(handle->funhook, 0) != FUNCHOOK_ERROR_SUCCESS)
		{
			return 1;
		}

		handle->target == *target;

		return 0;
	}

	return 1;
}


void * funhook_detour_impl_trampoline(detour_impl_handle handle)
{
	if (handle == NULL)
	{
		return NULL;
	}

	return handle->target;
}

int funhook_detour_impl_uninstall(detour_impl_handle handle)
{
	if (handle != NULL && handle->funhook != NULL)
	{
		return !(funchook_uninstall(handle->funhook, 0) == FUNCHOOK_ERROR_SUCCESS);
	}

	return 1;
}

int funhook_detour_impl_destroy(detour_impl_handle handle)
{
	int result = FUNCHOOK_ERROR_SUCCESS;

	if (handle == NULL)
	{
		return 0;
	}

	if (handle->funhook != NULL)
	{
		result = funhook_destroy(handle->funhook);
	}

	free(handle);

	return !(result == FUNCHOOK_ERROR_SUCCESS);
}
