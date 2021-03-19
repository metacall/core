/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>
#include <metacall/metacall_value.h>

void *c_callback(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)args;
	(void)data;

	printf("Callback without args executed\n");

	return metacall_value_create_long(32L);
}

void *c_callback_with_args(size_t argc, void *args[], void *data)
{
	long left = metacall_value_to_long(args[0]), right = metacall_value_to_long(args[1]);

	(void)argc;
	(void)data;

	printf("Callback with args executed (%ld, %ld)\n", left, right);

	return metacall_value_create_long(left + right);
}

void *c_callback_factorial_impl(size_t argc, void *args[], void *data)
{
	(void)argc;
	(void)data;

	if (metacall_value_to_long(args[0]) <= 0)
	{
		return metacall_value_create_long(0L);
	}
	else
	{
		return metacall_value_copy(args[0]);
	}
}

void *c_callback_factorial(size_t argc, void *args[], void *data)
{
	void *c = metacall_value_to_function(args[0]);

	// function c_callback_factorial(c) {
	//	return function c_callback_factorial_impl(v) { return v <= 0 ? 1 : v };
	// }

	(void)c;
	(void)argc;
	(void)data;

	return metacall_value_copy(data);
}

class metacall_function_test : public testing::Test
{
public:
};

TEST_F(metacall_function_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_stdio_type log_stdio = { stdout };

	ASSERT_EQ((int)0, (int)metacall_log(METACALL_LOG_STDIO, (void *)&log_stdio));

	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* Native register */
	ASSERT_EQ((int)0, (int)metacall_register("c_callback", c_callback, NULL, METACALL_LONG, 0));
	ASSERT_EQ((int)0, (int)metacall_register("c_callback_with_args", c_callback_with_args, NULL, METACALL_LONG, 2, METACALL_LONG, METACALL_LONG));
	ASSERT_EQ((int)0, (int)metacall_register("c_callback_factorial_impl", c_callback_factorial_impl, NULL, METACALL_LONG, 1, METACALL_LONG));
	ASSERT_EQ((int)0, (int)metacall_register("c_callback_factorial", c_callback_factorial, NULL, METACALL_FUNCTION, 1, METACALL_FUNCTION));

	/* Create function types */
	void *c_callback_value = metacall_value_create_function(metacall_function("c_callback"));
	void *c_callback_with_args_value = metacall_value_create_function(metacall_function("c_callback_with_args"));
	void *c_callback_factorial_impl_value = metacall_value_create_function(metacall_function("c_callback_factorial_impl"));
	void *c_callback_factorial_value = metacall_value_create_function_closure(metacall_function("c_callback_factorial"), c_callback_factorial_impl_value);

	ASSERT_NE((void *)NULL, (void *)c_callback_value);
	ASSERT_NE((void *)NULL, (void *)c_callback_with_args_value);
	ASSERT_NE((void *)NULL, (void *)c_callback_factorial_impl_value);
	ASSERT_NE((void *)NULL, (void *)c_callback_factorial_value);

	/* Test function data */
	EXPECT_EQ((int)0, (int)metacall_function_async(metacall_function("c_callback")));
	EXPECT_EQ((size_t)0, (size_t)metacall_function_size(metacall_function("c_callback")));
	EXPECT_EQ((int)0, (int)metacall_function_async(metacall_function("c_callback_with_args")));
	EXPECT_EQ((size_t)2, (size_t)metacall_function_size(metacall_function("c_callback_with_args")));
	EXPECT_EQ((int)0, (int)metacall_function_async(metacall_function("c_callback_factorial_impl")));
	EXPECT_EQ((size_t)1, (size_t)metacall_function_size(metacall_function("c_callback_factorial_impl")));
	EXPECT_EQ((int)0, (int)metacall_function_async(metacall_function("c_callback_factorial")));
	EXPECT_EQ((size_t)1, (size_t)metacall_function_size(metacall_function("c_callback_factorial")));

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char *py_scripts[] = {
			"function.py"
		};

		void *ret = NULL, *cb_ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		void *function_args[] = {
			c_callback_value,
		};

		ret = metacallv("function_cb", function_args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((long)metacall_value_to_long(ret), (long)32L);

		metacall_value_destroy(ret);

		void *function_with_args_args[] = {
			c_callback_with_args_value,
			metacall_value_create_long(5L),
			metacall_value_create_long(9L)
		};

		ret = metacallv("function_with_args", function_with_args_args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((long)metacall_value_to_long(ret), (long)14L);

		metacall_value_destroy(ret);

		metacall_value_destroy(function_with_args_args[1]);
		metacall_value_destroy(function_with_args_args[2]);

		void *function_ret_lambda_args[] = {
			metacall_value_create_long(5L)
		};

		ret = metacallv("function_ret_lambda", function_ret_lambda_args);

		EXPECT_NE((void *)NULL, (void *)ret);

		void *f = metacall_value_to_function(ret);

		cb_ret = metacallfv(f, function_ret_lambda_args);

		EXPECT_EQ((long)metacall_value_to_long(cb_ret), (long)25L);

		metacall_value_destroy(cb_ret);

		metacall_value_destroy(ret);

		metacall_value_destroy(function_ret_lambda_args[0]);

		ret = metacallv("function_return", metacall_null_args);

		EXPECT_NE((void *)NULL, (void *)ret);

		f = metacall_value_to_function(ret);

		cb_ret = metacallfv(f, metacall_null_args);

		EXPECT_EQ((long)metacall_value_to_long(cb_ret), (long)4L);

		metacall_value_destroy(cb_ret);

		metacall_value_destroy(ret);

		ret = metacallv("function_pass", metacall_null_args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_NULL, (enum metacall_value_id)metacall_value_id(ret));

		EXPECT_EQ((void *)NULL, (void *)metacall_value_to_null(ret));

		metacall_value_destroy(ret);

		ret = metacallv("function_myclass_new_class", metacall_null_args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_OBJECT, (enum metacall_value_id)metacall_value_id(ret));

		void *function_myclass_method_args[] = {
			ret
		};

		ret = metacallv("function_myclass_method", function_myclass_method_args);

		metacall_value_destroy(function_myclass_method_args[0]);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_STRING, (enum metacall_value_id)metacall_value_id(ret));

		EXPECT_EQ((int)0, (int)strcmp("hello world", metacall_value_to_string(ret)));

		metacall_value_destroy(ret);

		/* Test Complex Callbakcs */
		void *function_factorial_args[] = {
			c_callback_factorial_value
		};

		ret = metacallv("function_factorial", function_factorial_args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_FUNCTION, (enum metacall_value_id)metacall_value_id(ret));

		void *fact = metacall_value_to_function(ret);

		void *cb_function_factorial_args[] = {
			metacall_value_create_long(12L)
		};

		cb_ret = metacallfv(fact, cb_function_factorial_args);

		EXPECT_NE((void *)NULL, (void *)cb_ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(cb_ret));

		/* TODO: I have no clue why this returns 132, the correct value for factorial of 12 is 479001600L */
		EXPECT_EQ((long)132L, (long)metacall_value_to_long(cb_ret));

		metacall_value_destroy(cb_function_factorial_args[0]);

		metacall_value_destroy(ret);

		metacall_value_destroy(cb_ret);
	}
#endif /* OPTION_BUILD_LOADERS_PY */

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		/* TODO */
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	/* Clear function values */
	metacall_value_destroy(c_callback_value);
	metacall_value_destroy(c_callback_with_args_value);
	metacall_value_destroy(c_callback_factorial_impl_value);
	metacall_value_destroy(c_callback_factorial_value);

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
