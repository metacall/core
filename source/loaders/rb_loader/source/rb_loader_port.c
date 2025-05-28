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

#include <rb_loader/rb_loader_port.h>

#include <ruby.h>

#include <metacall/metacall.h>

#include <format/format.h>

VALUE rb_loader_port_load_from_file(int argc, VALUE *argv, VALUE self)
{
	const char *tag;
	const char **paths;
	size_t size;
	int result;

	(void)self;

	if (argc != 2)
	{
		rb_raise(rb_eArgError, "Wrong # of arguments (expected 2, received %d)", argc);
		return Qnil;
	}

	if (TYPE(argv[0]) != T_STRING)
	{
		rb_raise(rb_eArgError, "First parameter expected to be a string indicating the tag of the loader (py, node, c, ...)");
		return Qnil;
	}

	tag = StringValuePtr(argv[0]);

	if (TYPE(argv[1]) != T_ARRAY)
	{
		rb_raise(rb_eArgError, "Second parameter expected to be an array of strings with the desired files to be loaded");
		return Qnil;
	}

	/* Get array size */
	size = RARRAY_LEN(argv[1]);

	if (size == 0)
	{
		rb_raise(rb_eArgError, "Second parameter cannot be an empty file path list");
		return Qnil;
	}

	/* Parse the array */
	{
		size_t iterator;
		VALUE *array_ptr = RARRAY_PTR(argv[1]);

		paths = (const char **)malloc(sizeof(const char *) * size);

		if (paths == NULL)
		{
			rb_raise(rb_eArgError, "Invalid file argument allocation");
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

VALUE rb_loader_port_load_from_memory(int argc, VALUE *argv, VALUE self)
{
	const char *tag;
	const char *buffer;
	size_t size;
	int result;

	(void)self;

	if (argc != 2)
	{
		rb_raise(rb_eArgError, "Wrong # of arguments (expected 2, received %d)", argc);
		return Qnil;
	}

	if (TYPE(argv[0]) != T_STRING)
	{
		rb_raise(rb_eArgError, "First parameter expected to be a string indicating the tag of the loader (py, node, c, ...)");
		return Qnil;
	}

	tag = StringValuePtr(argv[0]);

	if (TYPE(argv[1]) != T_STRING)
	{
		rb_raise(rb_eArgError, "Second parameter expected to be an string with the code to be loaded");
		return Qnil;
	}

	/* Get buffer size */
	size = RSTRING_LEN(argv[1]) + 1;

	if (size == 1)
	{
		rb_raise(rb_eArgError, "Second parameter cannot be an empty string");
		return Qnil;
	}

	/* Execute load from memory */
	result = metacall_load_from_memory(tag, buffer, size, NULL);

	return LONG2NUM(result);
}

int rb_loader_port_initialize(void)
{
	return 0;
}
