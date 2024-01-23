/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <gtest/gtest.h>

#include <reflect/reflect_function.h>

#include <log/log.h>

#include <cstdlib>

typedef struct example_arg_type
{
	int a;
	float b;
	char c[10];
} * example_arg;

typedef void (*function_example_ptr)(char, int, void *);

typedef struct function_impl_example_type
{
	function_example_ptr ptr;

} * function_impl_example;

void function_example(char c, int i, void *p)
{
	struct example_arg_type *e = (struct example_arg_type *)p;

	log_write("metacall", LOG_LEVEL_DEBUG, "char: %c; int: %d; ptr: %p", c, i, p);

	log_write("metacall", LOG_LEVEL_DEBUG, "e->a: %d; e->b: %f; e->c: %s", e->a, e->b, e->c);
}

int function_example_interface_create(function func, function_impl impl)
{
	function_impl_example example_impl = (function_impl_example)impl;

	(void)func;

	if (example_impl != NULL)
	{
		example_impl->ptr = &function_example;

		return 0;
	}

	return 1;
}

function_return function_example_interface_invoke(function func, function_impl func_impl, function_args args, size_t size)
{
	function_impl_example example_impl = (function_impl_example)func_impl;

	(void)func;
	(void)size;

	example_impl->ptr(*((char *)(args[0])), *((int *)(args[1])), (void *)(args[2]));

	return NULL;
}

function_return function_example_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;
	(void)resolve_callback;
	(void)reject_callback;
	(void)context;

	return NULL;
}

void function_example_interface_destroy(function func, function_impl func_impl)
{
	(void)func;

	free(func_impl);
}

function_interface function_example_singleton()
{
	static struct function_interface_type example_interface = {
		&function_example_interface_create,
		&function_example_interface_invoke,
		&function_example_interface_await,
		&function_example_interface_destroy
	};

	return &example_interface;
}

class reflect_function_test : public testing::Test
{
public:
};

TEST_F(reflect_function_test, DefaultConstructor)
{
	EXPECT_EQ((int)0, (int)log_configure("metacall",
						  log_policy_format_text(),
						  log_policy_schedule_sync(),
						  log_policy_storage_sequential(),
						  log_policy_stream_stdio(stdout)));

	type char_type = type_create(TYPE_CHAR, "char", NULL, NULL);
	type int_type = type_create(TYPE_INT, "int", NULL, NULL);
	type ptr_type = type_create(TYPE_PTR, "ptr", NULL, NULL);

	EXPECT_NE((type)char_type, (type)NULL);
	EXPECT_NE((type)int_type, (type)NULL);
	EXPECT_NE((type)ptr_type, (type)NULL);

	if (char_type != NULL && int_type != NULL && ptr_type != NULL)
	{
		function f;

		function_impl_example example_impl = (function_impl_example)malloc(sizeof(struct function_impl_example_type));

		f = function_create("example", 3, example_impl, &function_example_singleton);

		EXPECT_NE((function)f, (function)NULL);

		if (f != NULL)
		{
			EXPECT_EQ((int)function_increment_reference(f), (int)0);

			signature_set(function_signature(f), 0, "c", char_type);
			signature_set(function_signature(f), 1, "i", int_type);
			signature_set(function_signature(f), 2, "p", ptr_type);

			/* function call example */
			{
				char c = 'm';
				int i = 123456789;
				struct example_arg_type e = { 5, 3.3f, "ABCDEFGHI" };

				function_args args = { &c, &i, &e };

				function_call(f, args, sizeof(args) / sizeof(args[0]));
			}

			function_destroy(f);
		}

		type_destroy(char_type);
		type_destroy(int_type);
		type_destroy(ptr_type);
	}
}
