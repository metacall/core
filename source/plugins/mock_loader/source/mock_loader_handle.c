/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading mock code at run-time into a process.
 *
 */

#include <mock_loader/mock_loader_handle.h>
#include <mock_loader/mock_loader_function.h>

#include <loader/loader_impl.h>

#include <reflect/type.h>
#include <reflect/function.h>
#include <reflect/scope.h>
#include <reflect/context.h>

#include <log/log.h>

#include <stdlib.h>

struct loader_impl_mock_handle_type;

typedef struct loader_impl_mock_handle_type * loader_impl_mock_handle;

struct loader_impl_mock_handle_type
{
	void * handle_mock_data;

};

loader_handle mock_loader_impl_handle_create(loader_impl impl, const loader_naming_path path, const loader_naming_name name);

int mock_loader_impl_handle_discover(loader_impl impl, loader_handle handle, context ctx);

int mock_loader_impl_handle_destroy(loader_impl impl, loader_handle handle);

loader_impl_handle mock_loader_impl_handle_singleton()
{
	static struct loader_impl_handle_type loader_impl_handle_mock =
	{
		&mock_loader_impl_handle_create,
		&mock_loader_impl_handle_discover,
		&mock_loader_impl_handle_destroy
	};

	return &loader_impl_handle_mock;
}

loader_handle mock_loader_impl_handle_create(loader_impl impl, const loader_naming_path path, const loader_naming_name name)
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

		scope_define(sp, function_name(f), f);
	}

	mock_function = mock_function_singleton->create(impl, handle);

	if (mock_function != NULL)
	{
		function f = function_create("two_doubles", 2, mock_function, mock_function_interface);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "Double"));

		signature_set(s, 0, "first_parameter", loader_impl_type(impl, "Double"));

		signature_set(s, 1, "second_parameter", loader_impl_type(impl, "Double"));

		scope_define(sp, function_name(f), f);
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

		scope_define(sp, function_name(f), f);
	}

	mock_function = mock_function_singleton->create(impl, handle);

	if (mock_function != NULL)
	{
		function f = function_create("new_args", 1, mock_function, mock_function_interface);

		signature s = function_signature(f);

		signature_set_return(s, loader_impl_type(impl, "String"));

		signature_set(s, 0, "str", loader_impl_type(impl, "String"));

		scope_define(sp, function_name(f), f);
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
