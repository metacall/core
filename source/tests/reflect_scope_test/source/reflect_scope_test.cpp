/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <gmock/gmock.h>

#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>

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

function_return function_example_interface_invoke(function func, function_impl func_impl, function_args args)
{
	function_impl_example example_impl = (function_impl_example)func_impl;

	(void)func;

	example_impl->ptr(*((char *)(args[0])), *((int *)(args[1])), (void *)(args[2]));

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
		function f1, f2;

		function_impl_example example_impl = (function_impl_example)malloc(sizeof(struct function_impl_example_type));

		f1 = function_create("example", 3, example_impl, &function_example_singleton);

		EXPECT_NE((function) f1, (function) NULL);

		if (f1 != NULL)
		{
			signature_set(function_signature(f1), 0, "c", char_type);
			signature_set(function_signature(f1), 1, "i", int_type);
			signature_set(function_signature(f1), 2, "p", ptr_type);

			/* function call example */
			{
				char c = 'm';
				int i = 123456789;
				struct example_arg_type e = { 5, 3.3f, "ABCDEFGHI" };

				function_args args = { &c, &i, &e };

				function_call(f1, args);
			}

			EXPECT_EQ((int) scope_define(sp, function_name(f1), f1), (int) 0);
		}

		function_impl_example example_asd_impl = (function_impl_example)malloc(sizeof(struct function_impl_example_type));

		f2 = function_create("example_asd", 3, example_asd_impl, &function_example_singleton);

		EXPECT_NE((function) f2, (function) NULL);

		if (f2 != NULL)
		{
			signature_set(function_signature(f2), 0, "e", char_type);
			signature_set(function_signature(f2), 1, "o", int_type);
			signature_set(function_signature(f2), 2, "u", ptr_type);

			/* function call example_asd */
			{
				char e = 'c';
				int o = 12349;
				struct example_arg_type u = { 25, 1.3f, "ABCDEFGHI" };

				function_args args = { &e, &o, &u };

				function_call(f2, args);
			}

			EXPECT_EQ((int) scope_define(sp, function_name(f2), f2), (int) 0);
		}

		{
			size_t size = 0;

			char * buffer = scope_dump(sp, &size);

			EXPECT_NE((char *) buffer, (char *) NULL);

			log_write("metacall", LOG_LEVEL_DEBUG, "Output scope buffer: [\n%s\n]", buffer);

			free(buffer);
		}

		function_destroy(f1);
		function_destroy(f2);

		type_destroy(char_type);
		type_destroy(int_type);
		type_destroy(ptr_type);

		scope_destroy(sp);
	}
}
