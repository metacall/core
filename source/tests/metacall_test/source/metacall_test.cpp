/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall-plugins.h>

#include <reflect/reflect_value_type.h>

#include <log/log.h>

class metacall_test : public testing::Test
{
public:
};

TEST_F(metacall_test, DefaultConstructor)
{
	EXPECT_EQ((int)0, (int)log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	metacall_print_info();

	EXPECT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_PLUGINS_PY)
	{
		const long seven_multiples_limit = 10;
		char * py_scripts[] = {(char *)"example.py"};
		long iterator;

		value ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_files(py_scripts, 1));

		ret = metacall("multiply", 5, 15);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((long) value_to_long(ret), (long) 75);

		value_destroy(ret);

		log_write("metacall", LOG_LEVEL_DEBUG, "7's multiples dude!");

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			ret = metacall("multiply", 7, iterator);

			EXPECT_NE((value) NULL, (value) ret);

			EXPECT_EQ((long) value_to_long(ret), (long) (7 * iterator));

			value_destroy(ret);
		}

		ret = metacall("divide", 64.0, 2.0);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((double) value_to_double(ret), (double) 32.0);

		value_destroy(ret);

		ret = metacall("sum", 1000, 3500);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((long) value_to_long(ret), (long) 4500);

		value_destroy(ret);

		ret = metacall("sum", 3, 4);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((long) value_to_long(ret), (long) 7);

		value_destroy(ret);

		EXPECT_EQ((value) NULL, (value) metacall("hello"));

		ret = metacall("strcat", "Hello ", "Universe");

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((int) 0, (int) strcmp(value_to_string(ret), "Hello Universe"));

		value_destroy(ret);
	}
	#endif /* OPTION_BUILD_PLUGINS_PY */

	/* Ruby */
	#if defined(OPTION_BUILD_PLUGINS_RB)
	{
		value ret = NULL;
		char * rb_scripts[] = {(char *)"hello.rb"};
		
		EXPECT_EQ((int) 0, (int) metacall_load_from_files(rb_scripts, 1));

		ret = metacall("say_multiply", 5, 7);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((int) value_to_int(ret), (int) 35);

		value_destroy(ret);

		EXPECT_EQ((void *) NULL, (void *) metacall("say_null"));

		ret = metacall("say_hello", "meta-programmer");

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((int) 0, (int) strcmp(value_to_string(ret), "Hello meta-programmer!"));

		value_destroy(ret);
	}
	#endif /* OPTION_BUILD_PLUGINS_RB */

	/* JavaScript SpiderMonkey */
	#if defined(OPTION_BUILD_PLUGINS_JSM)
	{
		char * jsm_scripts[] = {(char *)"spider.jsm")};
		EXPECT_EQ((int) 0, (int) metacall_load_from_file(jsm_scripts,1);

		EXPECT_EQ((void *) NULL, (void *) metacall("say_spider", 8, 4));
	}
	#endif /* OPTION_BUILD_PLUGINS_JSM */

	/* JavaScript V8 */
	#if defined(OPTION_BUILD_PLUGINS_JS)
	{
		value ret = NULL;
		char * js_scripts[] = {(char *)"divide.js"};

		EXPECT_EQ((int) 0, (int) metacall_load_from_files(js_scripts ,1));

		ret = metacall("say_divide", 32.0, 4.0);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((double) value_to_double(ret), (double) 8.0);

		value_destroy(ret);

		ret = metacall("some_text", "abc", "def");

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((int) 0, (int) strcmp(value_to_string(ret), "abcdef"));

		value_destroy(ret);
	}
	#endif /* OPTION_BUILD_PLUGINS_JS */

	/* Mock */
	#if defined(OPTION_BUILD_PLUGINS_MOCK)
	{
		value ret = NULL;
		char * mock_scripts[] = {(char *)"empty.mock"};

		EXPECT_EQ((int) 0, (int) metacall_load_from_files(mock_scripts, 1));

		ret = metacall("my_empty_func");

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((int) value_to_int(ret), (int) 1234);

		value_destroy(ret);

		ret = metacall("two_doubles", 3.14, 68.3);

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((double) value_to_double(ret), (double) 3.1416);

		value_destroy(ret);

		ret = metacall("mixed_args", 'E', 16, 34L, 4.6, "hello");

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((char) value_to_char(ret), (char) 'A');

		value_destroy(ret);

		ret = metacall("new_args", "goodbye");

		EXPECT_NE((value) NULL, (value) ret);

		EXPECT_EQ((int) 0, (int) strcmp(value_to_string(ret), "Hello World"));

		value_destroy(ret);
	}
	#endif /* OPTION_BUILD_PLUGINS_MOCK */

	/* C# Netcore */
	#if defined(OPTION_BUILD_PLUGINS_CS)
	{
		char * cs_scripts[] = {(char *)"hello.cs"};
		
		EXPECT_EQ((int) 0, (int) metacall_load_from_files(cs_scripts, 1));

		EXPECT_EQ((void *) NULL, (void *) metacall("Say", "Hello para with params!"));
	}
	#endif /* OPTION_BUILD_PLUGINS_CS */

	/* C */
	#if defined(OPTION_BUILD_PLUGINS_C)
	{
		char * c_scripts[] = {(char *)"compiled.c"};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file(c_scripts, 1));
	}
	#endif /* OPTION_BUILD_PLUGINS_C */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
