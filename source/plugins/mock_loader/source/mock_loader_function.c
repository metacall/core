/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading mock code at run-time into a process.
 *
 */

#include <mock_loader/mock_loader_function.h>
#include <mock_loader/mock_loader_function_interface.h>

#include <stdlib.h>

loader_function mock_loader_impl_function_create(loader_impl impl, loader_handle handle);

int mock_loader_impl_function_destroy(loader_impl impl, loader_handle handle, loader_function func);

loader_impl_function mock_loader_impl_function_singleton()
{
	static struct loader_impl_function_type loder_impl_function_mock =
	{
		&mock_loader_impl_function_create,
		&mock_loader_impl_function_interface,
		&mock_loader_impl_function_destroy
	};

	return &loder_impl_function_mock;
}

loader_function mock_loader_impl_function_create(loader_impl impl, loader_handle handle)
{
	loader_impl_mock_function mock_function = malloc(sizeof(struct loader_impl_mock_function_type));

	(void)impl;

	if (mock_function != NULL)
	{
		mock_function->handle = handle;

		mock_function->function_mock_data = NULL;

		return (loader_function)mock_function;
	}

	return NULL;
}

int mock_loader_impl_function_destroy(loader_impl impl, loader_handle handle, loader_function func)
{
	loader_impl_mock_function mock_function = (loader_impl_mock_function)func;

	(void)impl;
	(void)handle;

	if (mock_function != NULL)
	{
		free(mock_function);

		return 0;
	}

	return 1;
}
