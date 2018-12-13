/*
 *	Loader Library by Parra Studios
 *	A plugin for loading mock code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
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
