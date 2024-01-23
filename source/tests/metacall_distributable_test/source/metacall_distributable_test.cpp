/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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
#include <metacall/metacall_loaders.h>

class metacall_distributable_test : public testing::Test
{
public:
};

TEST_F(metacall_distributable_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char *py_scripts[] = {
			"example.py"
		};

		const long seven_multiples_limit = 10;

		long iterator;

		void *ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		ret = metacall("multiply", 5, 15);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((long)metacall_value_to_long(ret), (long)75);

		metacall_value_destroy(ret);

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			ret = metacall("multiply", 7, iterator);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((long)metacall_value_to_long(ret), (long)(7 * iterator));

			metacall_value_destroy(ret);
		}

		ret = metacall("divide", 64.0, 2.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)32.0);

		metacall_value_destroy(ret);

		ret = metacall("sum", 1000, 3500);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((long)metacall_value_to_long(ret), (long)4500);

		metacall_value_destroy(ret);

		ret = metacall("sum", 3, 4);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((long)metacall_value_to_long(ret), (long)7);

		metacall_value_destroy(ret);

		ret = metacall("hello");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_NULL, (enum metacall_value_id)metacall_value_id(ret));

		EXPECT_EQ((void *)NULL, (void *)metacall_value_to_null(ret));

		metacall_value_destroy(ret);

		ret = metacall("strcat", "Hello ", "Universe");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_to_string(ret), "Hello Universe"));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_PY */

/* Ruby */
#if defined(OPTION_BUILD_LOADERS_RB)
	{
		const char *rb_scripts[] = {
			"hello.rb", "second.rb"
		};

		void *ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("rb", rb_scripts, sizeof(rb_scripts) / sizeof(rb_scripts[0]), NULL));

		ret = metacall("say_multiply", 5, 7);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_to_int(ret), (int)35);

		metacall_value_destroy(ret);

		ret = metacall("say_null");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_NULL, (enum metacall_value_id)metacall_value_id(ret));

		metacall_value_destroy(ret);

		ret = metacall("say_hello", "meta-programmer");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_to_string(ret), "Hello meta-programmer!"));

		metacall_value_destroy(ret);

		ret = metacall("get_second", 5, 12);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_to_int(ret), (int)12);

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_RB */

/* JavaScript SpiderMonkey */
#if defined(OPTION_BUILD_LOADERS_JSM)
	{
		const char *jsm_scripts[] = {
			"spider.jsm"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_file("jsm", jsm_scripts, sizeof(jsm_scripts) / sizeof(jsm_scripts[0]), NULL));

		EXPECT_EQ((void *)NULL, (void *)metacall("say_spider", 8, 4));
	}
#endif /* OPTION_BUILD_LOADERS_JSM */

/* JavaScript V8 */
#if defined(OPTION_BUILD_LOADERS_JS)
	{
		const char *js_scripts[] = {
			"divide.js", "third.js"
		};

		void *ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("js", js_scripts, sizeof(js_scripts) / sizeof(js_scripts[0]), NULL));

		ret = metacall("say_divide", 32.0, 4.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)8.0);

		metacall_value_destroy(ret);

		ret = metacall("some_text", "abc", "def");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_to_string(ret), "abcdef"));

		metacall_value_destroy(ret);

		ret = metacall("third_value", "abc", "def", "efg");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_to_string(ret), "efg"));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_JS */

/* Mock */
#if defined(OPTION_BUILD_LOADERS_MOCK)
	{
		const char *mock_scripts[] = {
			"empty.mock"
		};

		void *ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("mock", mock_scripts, sizeof(mock_scripts) / sizeof(mock_scripts[0]), NULL));

		ret = metacall("my_empty_func");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_to_int(ret), (int)1234);

		metacall_value_destroy(ret);

		ret = metacall("two_doubles", 3.14, 68.3);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_to_double(ret), (double)3.1416);

		metacall_value_destroy(ret);

		ret = metacall("mixed_args", 'E', 16, 34L, 4.6, "hello");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((char)metacall_value_to_char(ret), (char)'A');

		metacall_value_destroy(ret);

		ret = metacall("new_args", "goodbye");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_to_string(ret), "Hello World"));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_MOCK */

/* C# Netcore */
#if defined(OPTION_BUILD_LOADERS_CS)
	{
		const char *cs_scripts[] = {
			"hello.cs"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_file("cs", cs_scripts, sizeof(cs_scripts) / sizeof(cs_scripts[0]), NULL));

		EXPECT_EQ((void *)NULL, (void *)metacall("Say", "Hello para with params!"));
	}
#endif /* OPTION_BUILD_LOADERS_CS */

/* C */
#if defined(OPTION_BUILD_LOADERS_C)
	{
		const char *c_scripts[] = {
			"compiled.c"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_file("c", c_scripts, sizeof(c_scripts) / sizeof(c_scripts[0]), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_C */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
