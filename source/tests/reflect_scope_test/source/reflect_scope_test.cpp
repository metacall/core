/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
#include <reflect/reflect_scope.h>
#include <reflect/reflect_value_type.h>

#include <serial/serial.h>

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

void function_example(char c, int i, void * p)
{
	struct example_arg_type * e = (struct example_arg_type *)p;

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

function_return function_example_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void * context)
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
	static struct function_interface_type example_interface =
	{
		&function_example_interface_create,
		&function_example_interface_invoke,
		&function_example_interface_await,
		&function_example_interface_destroy
	};

	return &example_interface;
}

class reflect_scope_test : public testing::Test
{
  public:
};

TEST_F(reflect_scope_test, DefaultConstructor)
{
	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	// Initialize serial
	EXPECT_EQ((int) 0, (int) serial_initialize());

	// Create serial
	serial s = serial_create("rapid_json");

	scope sp = scope_create("test");

	EXPECT_NE((scope) sp, (scope) NULL);

	type char_type = type_create(TYPE_CHAR, "char", NULL, NULL);
	type int_type = type_create(TYPE_INT, "int", NULL, NULL);
	type ptr_type = type_create(TYPE_PTR, "ptr", NULL, NULL);

	EXPECT_NE((type) char_type, (type) NULL);
	EXPECT_NE((type) int_type, (type) NULL);
	EXPECT_NE((type) ptr_type, (type) NULL);

	if (sp && char_type != NULL && int_type != NULL && ptr_type != NULL)
	{
		function f1, f2, f3, f4, f5, f6, f7, f8, f9;

		function_impl_example example_impl = (function_impl_example)malloc(sizeof(struct function_impl_example_type));

		f1 = function_create("example", 3, example_impl, &function_example_singleton);

		EXPECT_NE((function) f1, (function) NULL);

		if (f1 != NULL)
		{
			signature_set(function_signature(f1), 0, "c", char_type);
			signature_set(function_signature(f1), 1, "i", int_type);
			signature_set(function_signature(f1), 2, "p", ptr_type);

			EXPECT_EQ((int) scope_define(sp, function_name(f1), value_create_function(f1)), (int) 0);
		}

		function_impl_example example_asd_impl = (function_impl_example)malloc(sizeof(struct function_impl_example_type));

		f2 = function_create("example_asd", 3, example_asd_impl, &function_example_singleton);

		EXPECT_NE((function) f2, (function) NULL);

		if (f2 != NULL)
		{
			signature_set(function_signature(f2), 0, "e", char_type);
			signature_set(function_signature(f2), 1, "o", int_type);
			signature_set(function_signature(f2), 2, "u", ptr_type);

			EXPECT_EQ((int) scope_define(sp, function_name(f2), value_create_function(f2)), (int) 0);
		}

		function_impl_example example_ret_impl = (function_impl_example)malloc(sizeof(struct function_impl_example_type));

		f3 = function_create("example_ret", 3, example_ret_impl, &function_example_singleton);

		EXPECT_NE((function) f3, (function) NULL);

		if (f3 != NULL)
		{
			signature_set_return(function_signature(f3), int_type);
			signature_set(function_signature(f3), 0, "e", char_type);
			signature_set(function_signature(f3), 1, "o", int_type);
			signature_set(function_signature(f3), 2, "u", ptr_type);

			EXPECT_EQ((int) scope_define(sp, function_name(f3), value_create_function(f3)), (int) 0);
		}

		function_impl_example example_duck_impl = (function_impl_example)malloc(sizeof(struct function_impl_example_type));

		f4 = function_create("example_duck", 3, example_duck_impl, &function_example_singleton);

		EXPECT_NE((function) f4, (function) NULL);

		if (f4 != NULL)
		{
			signature_set(function_signature(f4), 0, "e", NULL);
			signature_set(function_signature(f4), 1, "o", NULL);
			signature_set(function_signature(f4), 2, "u", NULL);

			EXPECT_EQ((int) scope_define(sp, function_name(f4), value_create_function(f4)), (int) 0);
		}

		function_impl_example example_duck_ret_impl = (function_impl_example)malloc(sizeof(struct function_impl_example_type));

		f5 = function_create("example_duck_ret", 3, example_duck_ret_impl, &function_example_singleton);

		EXPECT_NE((function) f5, (function) NULL);

		if (f5 != NULL)
		{
			signature_set_return(function_signature(f5), int_type);
			signature_set(function_signature(f5), 0, "asdfe", NULL);
			signature_set(function_signature(f5), 1, "do", NULL);
			signature_set(function_signature(f5), 2, "dafu", NULL);

			EXPECT_EQ((int) scope_define(sp, function_name(f5), value_create_function(f5)), (int) 0);
		}

		function_impl_example example_duck_mix_impl = (function_impl_example)malloc(sizeof(struct function_impl_example_type));

		f6 = function_create("example_duck_mix", 4, example_duck_mix_impl, &function_example_singleton);

		EXPECT_NE((function) f6, (function) NULL);

		if (f6 != NULL)
		{
			signature_set(function_signature(f6), 0, "asdfe", NULL);
			signature_set(function_signature(f6), 1, "do", int_type);
			signature_set(function_signature(f6), 2, "dafu", NULL);
			signature_set(function_signature(f6), 3, "dafu", ptr_type);

			EXPECT_EQ((int) scope_define(sp, function_name(f6), value_create_function(f6)), (int) 0);
		}

		function_impl_example example_duck_mix_ret_impl = (function_impl_example)malloc(sizeof(struct function_impl_example_type));

		f7 = function_create("example_duck_mix_ret", 4, example_duck_mix_ret_impl, &function_example_singleton);

		EXPECT_NE((function) f7, (function) NULL);

		if (f7 != NULL)
		{
			signature_set_return(function_signature(f7), int_type);
			signature_set(function_signature(f7), 0, "asdfe", int_type);
			signature_set(function_signature(f7), 1, "do", NULL);
			signature_set(function_signature(f7), 2, "dafu", ptr_type);
			signature_set(function_signature(f7), 3, "dafufs", NULL);

			EXPECT_EQ((int) scope_define(sp, function_name(f7), value_create_function(f7)), (int) 0);
		}

		function_impl_example example_empty_ret_impl = (function_impl_example)malloc(sizeof(struct function_impl_example_type));

		f8 = function_create("example_empty_ret", 0, example_empty_ret_impl, &function_example_singleton);

		EXPECT_NE((function) f8, (function) NULL);

		if (f8 != NULL)
		{
			signature_set_return(function_signature(f8), int_type);

			EXPECT_EQ((int) scope_define(sp, function_name(f8), value_create_function(f8)), (int) 0);
		}

		function_impl_example example_empty_impl = (function_impl_example)malloc(sizeof(struct function_impl_example_type));

		f9 = function_create("example_empty", 0, example_empty_impl, &function_example_singleton);

		EXPECT_NE((function) f9, (function) NULL);

		if (f9 != NULL)
		{
			EXPECT_EQ((int) scope_define(sp, function_name(f9), value_create_function(f9)), (int) 0);
		}

		{
			size_t size = 0;

			value v = scope_metadata(sp);

			EXPECT_NE((value) NULL, (value) v);

			memory_allocator allocator = memory_allocator_std(&std::malloc, &std::realloc, &std::free);

			char * str = serial_serialize(s, v, &size, allocator);

			log_write("metacall", LOG_LEVEL_DEBUG, "Scope serialization info: %s", str);

			memory_allocator_deallocate(allocator, str);

			memory_allocator_destroy(allocator);

			value_type_destroy(v);
		}

		type_destroy(char_type);
		type_destroy(int_type);
		type_destroy(ptr_type);

		scope_destroy(sp);
	}

	// Clear serial
	EXPECT_EQ((int) 0, (int) serial_clear(s));

	// Destroy serial
	serial_destroy();
}
