/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <rb_loader/rb_loader_impl.h>
#include <rb_loader/rb_loader_port.h>

#include <format/format.h>

#include <metacall/metacall.h>

#include <rb_loader/rb_loader_include.h>

static loader_impl rb_loader_impl = NULL;

static VALUE rb_loader_port_load_from_file(VALUE self, VALUE tag_value, VALUE paths_value)
{
	const char *tag;
	const char **paths;
	size_t size;
	int result;

	(void)self;

	/* Get tag */
	if (TYPE(tag_value) != T_STRING)
	{
		rb_raise(rb_eArgError, "First parameter expected to be a string indicating the tag of the loader (py, node, c, ...)");
		return Qnil;
	}

	tag = StringValuePtr(tag_value);

	/* Get array size */
	if (TYPE(paths_value) != T_ARRAY)
	{
		rb_raise(rb_eArgError, "Second parameter expected to be an array of strings with the desired files to be loaded");
		return Qnil;
	}

	size = RARRAY_LEN(paths_value);

	if (size == 0)
	{
		rb_raise(rb_eArgError, "Second parameter cannot be an empty file path list");
		return Qnil;
	}

	/* Parse the array */
	{
		size_t iterator;
		VALUE *array_ptr = RARRAY_PTR(paths_value);

		paths = (const char **)malloc(sizeof(const char *) * size);

		if (paths == NULL)
		{
			rb_raise(rb_eArgError, "Invalid paths argument allocation");
			return Qnil;
		}

		for (iterator = 0; iterator < size; ++iterator)
		{
			if (TYPE(array_ptr[iterator]) != T_STRING)
			{
				rb_raise(rb_eArgError, "Second parameter expected to be an array of strings, but the element %" PRIuS " of the array is not a string", iterator);
				free(paths);
				return Qnil;
			}

			paths[iterator] = StringValuePtr(array_ptr[iterator]);
		}
	}

	/* Execute load from file */
	result = metacall_load_from_file(tag, (const char **)paths, size, NULL);

	free(paths);

	return LONG2NUM(result);
}

static VALUE rb_loader_port_load_from_memory(VALUE self, VALUE tag_value, VALUE buffer_value)
{
	const char *tag;
	const char *buffer;
	size_t size;
	int result;

	(void)self;

	/* Get tag */
	if (TYPE(tag_value) != T_STRING)
	{
		rb_raise(rb_eArgError, "First parameter expected to be a string indicating the tag of the loader (py, node, c, ...)");
		return Qnil;
	}

	tag = StringValuePtr(tag_value);

	/* Get buffer size */
	if (TYPE(buffer_value) != T_STRING)
	{
		rb_raise(rb_eArgError, "Second parameter expected to be an string with the code to be loaded");
		return Qnil;
	}

	size = RSTRING_LEN(buffer_value) + 1;

	if (size == 1)
	{
		rb_raise(rb_eArgError, "Second parameter cannot be an empty string");
		return Qnil;
	}

	/* Get buffer */
	buffer = StringValuePtr(buffer_value);

	/* Execute load from memory */
	result = metacall_load_from_memory(tag, buffer, size, NULL);

	return LONG2NUM(result);
}

static VALUE rb_loader_port_metacall(int argc, VALUE *argv, VALUE self)
{
	const char *function_name;
	size_t args_size, iterator;
	value *args, result;

	(void)self;

	if (argc <= 0)
	{
		rb_raise(rb_eArgError, "Wrong # of arguments (expected at least 1 argument, received 0)");
		return Qnil;
	}

	/* Get function name */
	if (TYPE(argv[0]) != T_STRING)
	{
		rb_raise(rb_eArgError, "First parameter expected to be a string indicating the function name to be called");
		return Qnil;
	}

	function_name = StringValuePtr(argv[0]);

	/* Allocate arguments */
	args_size = argc - 1;

	args = args_size > 0 ? (value *)malloc(sizeof(value) * args_size) : metacall_null_args;

	if (args_size > 0 && args == NULL)
	{
		rb_raise(rb_eArgError, "Invalid arguments allocation");
		return Qnil;
	}

	/* Convert the arguments into MetaCall values */
	for (iterator = 0; iterator < args_size; ++iterator)
	{
		(void)rb_type_deserialize(rb_loader_impl, argv[iterator + 1], &args[iterator]);
	}

	/* Execute the call */
	result = metacallv_s(function_name, args, args_size);

	/* Clear the arguments */
	if (args_size > 0)
	{
		for (iterator = 0; iterator < args_size; ++iterator)
		{
			value_type_destroy(args[iterator]);
		}

		free(args);
	}

	return rb_type_serialize(result);
}

static VALUE rb_loader_port_inspect(VALUE self)
{
	VALUE result;
	size_t size = 0;
	char *result_str = NULL, *inspect_str = NULL;
	struct metacall_allocator_std_type std_ctx = { &malloc, &realloc, &free };

	/* Create the allocator */
	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	(void)self;

	/* Retrieve inspect data */
	result_str = inspect_str = metacall_inspect(&size, allocator);

	if (inspect_str == NULL || size == 0)
	{
		static const char empty[] = "{}";

		result_str = (char *)empty;
		size = sizeof(empty);

		rb_raise(rb_eArgError, "Inspect returned an invalid size or string");
	}

	result = rb_str_new(result_str, size - 1);

	if (inspect_str != NULL && size > 0)
	{
		metacall_allocator_free(allocator, inspect_str);
	}

	metacall_allocator_destroy(allocator);

	return result;
}

static VALUE rb_loader_port_atexit(VALUE self)
{
	static int atexit_executed = 0;

	(void)self;

	if (atexit_executed == 0 && rb_loader_impl_destroy(rb_loader_impl) != 0)
	{
		rb_raise(rb_eSystemExit, "Failed to destroy Ruby Loader from MetaCall");
	}

	atexit_executed = 1;

	return Qnil;
}

int rb_loader_port_initialize(loader_impl impl)
{
	VALUE rb_loader_port;

	if (impl == NULL)
	{
		return 1;
	}

	if (rb_loader_impl != NULL)
	{
		return 0;
	}

	rb_loader_port = rb_define_module("MetaCallRbLoaderPort");
	rb_define_module_function(rb_loader_port, "metacall_load_from_file", rb_loader_port_load_from_file, 2);
	rb_define_module_function(rb_loader_port, "metacall_load_from_memory", rb_loader_port_load_from_memory, 2);
	rb_define_module_function(rb_loader_port, "metacall", rb_loader_port_metacall, -1);
	rb_define_module_function(rb_loader_port, "metacall_inspect", rb_loader_port_inspect, 0);
	rb_define_module_function(rb_loader_port, "rb_loader_port_atexit", rb_loader_port_atexit, 0);

	rb_loader_impl = impl;

	return 0;
}
