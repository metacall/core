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

void * c_callback(void * args[])
{
	(void)args;

	printf("Callback without args executed\n");

	return metacall_value_create_long(34L);
}

void * c_callback_with_args(void * args[])
{
	long left = metacall_value_to_long(args[0]), right = metacall_value_to_long(args[1]);

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

		ret = metacallv("function", function_args);

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

		/* TODO: Return */
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
