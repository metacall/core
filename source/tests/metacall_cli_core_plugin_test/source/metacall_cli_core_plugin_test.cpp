/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
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

#include <metacall/metacall.h>

#include <string.h>

#include <iostream>

class metacall_cli_core_plugin_test : public testing::Test
{
protected:
};

TEST_F(metacall_cli_core_plugin_test, DefaultConstructor)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	void *handle = NULL;
	EXPECT_EQ((int)0, (int)metacall_load_from_configuration(CLI_CORE_PLUGIN_PATH, &handle, allocator));

	/* Test load */
	{
		void *args[2];

		args[0] = metacall_value_create_string("py", 2);

		char test_script[] = "example.py";
		void *test_script_v = metacall_value_create_string(test_script, strlen(test_script));
		args[1] = metacall_value_create_array((const void **)&test_script_v, 1);

		void *ret = metacallhv_s(handle, "load", args, 2);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((long)metacall_value_to_int(ret), (long)0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);
	}

	/* Test load */
	{
		void *args[2];
		args[0] = metacall_value_create_string("node", 4);

		char test_script[] = "nod.js";
		void *test_script_v = metacall_value_create_string(test_script, strlen(test_script));
		args[1] = metacall_value_create_array((const void **)&test_script_v, 1);

		void *ret = metacallhv_s(handle, "load", args, 2);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((long)metacall_value_to_int(ret), (long)0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);
	}

	/* Test eval */
	{
		void *args[2];

		args[0] = metacall_value_create_string("py", 2);

		char func_call[] = "print('Testing core_plugin...')\n";
		args[1] = (void **)metacall_value_create_string(func_call, strlen(func_call));

		void *ret = metacallhv_s(handle, "eval", args, 2);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((long)metacall_value_to_int(ret), (long)0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);
	}

	/* Test call */
	{
		char func_call[] = "multiply(7, 3)";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "call", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((long)metacall_value_to_long(ret), (long)21);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test call without args */
	{
		char func_call[] = "hello()";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "call", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test malformed call without last parenthesis */
	{
		char func_call[] = "multiply(7, 3";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "call", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test malformed call without last parenthesis but another character instead */
	{
		char func_call[] = "multiply(7, 3-";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "call", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test malformed call super tricky multiply("()", "eee"- */
	{
		char func_call[] = "multiply(\"()\", \"eee\"~";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "call", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test malformed call with only first parenthesis */
	{
		char func_call[] = "multiply(";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "call", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test malformed call without any parenthesis */
	{
		char func_call[] = "multiply";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "call", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test malformed call without name */
	{
		char func_call[] = "()";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "call", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test malformed call without name and with only one parenthesis */
	{
		char func_call[] = "(";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "call", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test malformed call without name and with only one parenthesis (the oposite) */
	{
		char func_call[] = ")";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "call", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test malformed call completely empty */
	{
		char func_call[] = "";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "call", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test await */
	{
		char func_call[] = "hello_boy_await(2, 2)";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "await", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((long)metacall_value_to_double(ret), (double)4);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test await without args */
	{
		char func_call[] = "return_await()";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "await", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_STREQ((const char *)metacall_value_to_string(ret), (const char *)"Hello World");

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test malformed call without last parenthesis */
	{
		char func_call[] = "hello_boy_await(2, 2";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "await", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test malformed call with only first parenthesis */
	{
		char func_call[] = "hello_boy_await(";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "await", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test malformed call without any parenthesis */
	{
		char func_call[] = "hello_boy_await";
		void *args[] = { metacall_value_create_string(func_call, strlen(func_call)) };

		void *ret = metacallhv_s(handle, "await", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Test clear */
	{
		void *args[2];

		args[0] = metacall_value_create_string("py", 2);

		char test_script[] = "example.py";
		args[1] = metacall_value_create_string(test_script, strlen(test_script));

		void *ret = metacallhv_s(handle, "clear", args, 2);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((long)metacall_value_to_int(ret), (long)0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);
	}

	/* Test inspect */
	{
		void *ret = metacallhv_s(handle, "inspect", metacall_null_args, 0);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((long)metacall_value_to_int(ret), (long)0);

		metacall_value_destroy(ret);
	}

	/* Test debug */
	{
		void *args[] = { metacall_value_create_array(NULL, 3) };
		void **array = metacall_value_to_array(args[0]);

		array[0] = metacall_value_create_string("abc", sizeof("abc") - 1);
		array[1] = metacall_value_create_string("cbd", sizeof("cbd") - 1);
		array[2] = metacall_value_create_string("thc", sizeof("thc") - 1);

		void *ret = metacallhv_s(handle, "debug", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_STREQ((const char *)"{\n\t[0]: abc\n\t[1]: cbd\n\t[2]: thc\n}", (const char *)metacall_value_to_string(ret));

		metacall_value_destroy(ret);
	}

	/* Print inspect information */
	{
		size_t size = 0;

		char *inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *)NULL, (char *)inspect_str);

		EXPECT_GT((size_t)size, (size_t)0);

		std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);
	}

	metacall_allocator_destroy(allocator);

	/* Test destroy */
	{
		void *ret = metacallhv_s(handle, "exit", metacall_null_args, 0);

		EXPECT_EQ((void *)NULL, (void *)ret);
	}
}
