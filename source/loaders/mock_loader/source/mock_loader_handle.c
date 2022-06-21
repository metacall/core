/*
 *	Loader Library by Parra Studios
 *	A plugin for loading mock code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
#include <mock_loader/mock_loader_handle.h>

#include <loader/loader_impl.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <log/log.h>

#include <stdlib.h>

struct loader_impl_mock_handle_type;

typedef struct loader_impl_mock_handle_type *loader_impl_mock_handle;

struct loader_impl_mock_handle_type
{
	void *handle_mock_data;
};

loader_handle mock_loader_impl_handle_create(loader_impl impl, const loader_path path, const loader_name name);

int mock_loader_impl_handle_discover(loader_impl impl, loader_handle handle, context ctx);

int mock_loader_impl_handle_destroy(loader_impl impl, loader_handle handle);

loader_impl_handle mock_loader_impl_handle_singleton(void)
{
	static struct loader_impl_handle_type loader_impl_handle_mock = {
		&mock_loader_impl_handle_create,
		&mock_loader_impl_handle_discover,
		&mock_loader_impl_handle_destroy
	};

	return &loader_impl_handle_mock;
}

loader_handle mock_loader_impl_handle_create(loader_impl impl, const loader_path path, const loader_name name)
{
	loader_impl_mock_handle handle = malloc(sizeof(struct loader_impl_mock_handle_type));

	(void)impl;

	if (handle != NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Mock module %s (%s) loaded", path, name);

		handle->handle_mock_data = NULL;

		return (loader_handle)handle;
	}

	return NULL;
}

int mock_loader_impl_handle_discover(loader_impl impl, loader_handle handle, context ctx)
{
	scope sp = context_scope(ctx);

	loader_impl_function mock_function_singleton = mock_loader_impl_function_singleton();

	loader_function mock_function = mock_function_singleton->create(impl, handle);

	function_impl_interface_singleton mock_function_interface = mock_function_singleton->interface();

	if (mock_function != NULL)
	{
		function f = function_create("my_empty_func", 0, mock_function, mock_function_interface);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "Integer"));

		value v = value_create_function(f);

		if (scope_define(sp, function_name(f), v) != 0)
		{
			value_type_destroy(v);
			return 1;
		}
	}

	mock_function = mock_function_singleton->create(impl, handle);

	if (mock_function != NULL)
	{
		function f = function_create("two_doubles", 2, mock_function, mock_function_interface);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "Double"));

		signature_set(s, 0, "first_parameter", loader_impl_type(impl, "Double"));

		signature_set(s, 1, "second_parameter", loader_impl_type(impl, "Double"));

		value v = value_create_function(f);

		if (scope_define(sp, function_name(f), v) != 0)
		{
			value_type_destroy(v);
			return 1;
		}
	}

	mock_function = mock_function_singleton->create(impl, handle);

	if (mock_function != NULL)
	{
		function f = function_create("mixed_args", 5, mock_function, mock_function_interface);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "Char"));

		signature_set(s, 0, "a_char", loader_impl_type(impl, "Char"));

		signature_set(s, 1, "b_int", loader_impl_type(impl, "Integer"));

		signature_set(s, 2, "c_long", loader_impl_type(impl, "Long"));

		signature_set(s, 3, "d_double", loader_impl_type(impl, "Double"));

		signature_set(s, 4, "e_ptr", loader_impl_type(impl, "Ptr"));

		value v = value_create_function(f);

		if (scope_define(sp, function_name(f), v) != 0)
		{
			value_type_destroy(v);
			return 1;
		}
	}

	mock_function = mock_function_singleton->create(impl, handle);

	if (mock_function != NULL)
	{
		function f = function_create("new_args", 1, mock_function, mock_function_interface);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "String"));

		signature_set(s, 0, "str", loader_impl_type(impl, "String"));

		value v = value_create_function(f);

		if (scope_define(sp, function_name(f), v) != 0)
		{
			value_type_destroy(v);
			return 1;
		}
	}

	return 0;
}

int mock_loader_impl_handle_destroy(loader_impl impl, loader_handle handle)
{
	loader_impl_mock_handle mock_handle = (loader_impl_mock_handle)handle;

	(void)impl;

	if (mock_handle != NULL)
	{
		free(mock_handle);

		return 0;
	}

	return 1;
}
