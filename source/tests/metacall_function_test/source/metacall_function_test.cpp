/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall_value.h>
#include <metacall/metacall_loaders.h>

void * c_callback(size_t argc, void * args[], void * data)
{
	(void)argc;
	(void)args;
	(void)data;

	printf("Callback without args executed\n");

	return metacall_value_create_long(32L);
}

void * c_callback_with_args(size_t argc, void * args[], void * data)
{
	long left = metacall_value_to_long(args[0]), right = metacall_value_to_long(args[1]);

	(void)argc;
	(void)data;

	printf("Callback with args executed (%ld, %ld)\n", left, right);

	return metacall_value_create_long(left + right);
}

class metacall_function_test : public testing::Test
{
public:
};

TEST_F(metacall_function_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_stdio_type log_stdio = { stdout };

	ASSERT_EQ((int) 0, (int) metacall_log(METACALL_LOG_STDIO, (void *)&log_stdio));

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Native register */
	ASSERT_EQ((int) 0, (int) metacall_register("c_callback", c_callback, METACALL_LONG, 0));
	ASSERT_EQ((int) 0, (int) metacall_register("c_callback_with_args", c_callback_with_args, METACALL_LONG, 2, METACALL_LONG, METACALL_LONG));

	/* Create function types */
	void * c_callback_value = metacall_value_create_function(metacall_function("c_callback"));
	void * c_callback_with_args_value = metacall_value_create_function(metacall_function("c_callback_with_args"));

	ASSERT_NE((void *) NULL, (void *) c_callback_value);
	ASSERT_NE((void *) NULL, (void *) c_callback_with_args_value);

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char * py_scripts[] =
		{
			"function.py"
		};

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		void * function_args[] =
		{
			c_callback_value,
		};

		ret = metacallv("function_cb", function_args);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_to_long(ret), (long) 32L);

		metacall_value_destroy(ret);

		void * function_with_args_args[] =
		{
			c_callback_with_args_value,
			metacall_value_create_long(5L),
			metacall_value_create_long(9L)
		};

		ret = metacallv("function_with_args", function_with_args_args);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_to_long(ret), (long) 14L);

		metacall_value_destroy(ret);

		metacall_value_destroy(function_with_args_args[1]);
		metacall_value_destroy(function_with_args_args[2]);

		void * function_ret_lambda_args[] =
		{
			metacall_value_create_long(5L)
		};

		ret = metacallv("function_ret_lambda", function_ret_lambda_args);

		EXPECT_NE((void *) NULL, (void *) ret);

		void * f = metacall_value_to_function(ret);

		void * cb_ret = metacallfv(f, function_ret_lambda_args);

		EXPECT_EQ((long) metacall_value_to_long(cb_ret), (long) 25L);

		metacall_value_destroy(cb_ret);

		metacall_value_destroy(ret);

		metacall_value_destroy(function_ret_lambda_args[0]);

		ret = metacallv("function_return", metacall_null_args);

		EXPECT_NE((void *) NULL, (void *) ret);

		f = metacall_value_to_function(ret);

		cb_ret = metacallfv(f, metacall_null_args);

		EXPECT_EQ((long) metacall_value_to_long(cb_ret), (long) 4L);

		metacall_value_destroy(cb_ret);

		metacall_value_destroy(ret);

		ret = metacallv("function_pass", metacall_null_args);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((enum metacall_value_id) METACALL_NULL, (enum metacall_value_id) metacall_value_id(ret));

		EXPECT_EQ((void *) NULL, (void *) metacall_value_to_null(ret));

		metacall_value_destroy(ret);

		/* TODO: This is a workaround to achieve class / object callbacks between languages. */
		/* It provides interoperatibility but without proper reflection. */
		/* Enough to implement callbacks with opaque pointers between languages. */

		ret = metacallv("function_capsule_new_class", metacall_null_args);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((enum metacall_value_id) METACALL_PTR, (enum metacall_value_id) metacall_value_id(ret));

		void * function_capsule_method_args[] =
		{
			ret
		};

		ret = metacallv("function_capsule_method", function_capsule_method_args);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((enum metacall_value_id) METACALL_STRING, (enum metacall_value_id) metacall_value_id(ret));

		EXPECT_EQ((int) 0, (int) strcmp("hello world", metacall_value_to_string(ret)));

		metacall_value_destroy(ret);

		metacall_value_destroy(function_capsule_method_args[0]);
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

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
