/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall_value.h>
#include <metacall/metacall-plugins.h>

#include <reflect/reflect_value_type.h>

#include <log/log.h>


void * c_function(void * args[]) {
	printf("%s\n", (char*)args[0]);
	return metacall_value_create_int(1);
}

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

	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* native register */
	{
		metacall_register("c_print", c_function, METACALL_INT, 1, METACALL_STRING);

		value ret = NULL;
		
		int result = 0;

		ret = metacall("c_print", "Hello native function!");

		EXPECT_NE((value)NULL, (value)ret);
		
		result = value_to_int(ret);
		
		EXPECT_EQ(result, 1);

		value_destroy(ret);
	}

	/* Python */
#if defined(OPTION_BUILD_PLUGINS_PY)
	{
		const char * py_scripts[] =
		{
			"example.py"
		};

		const long seven_multiples_limit = 10;

		long iterator;

		value ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0])));

		ret = metacall("multiply", 5, 15);

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((long)value_to_long(ret), (long)75);

		value_destroy(ret);

		log_write("metacall", LOG_LEVEL_DEBUG, "7's multiples dude!");

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			ret = metacall("multiply", 7, iterator);

			EXPECT_NE((value)NULL, (value)ret);

			EXPECT_EQ((long)value_to_long(ret), (long)(7 * iterator));

			value_destroy(ret);
		}

		ret = metacall("divide", 64.0, 2.0);

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((double)value_to_double(ret), (double) 32.0);

		value_destroy(ret);

		ret = metacall("sum", 1000, 3500);

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((long)value_to_long(ret), (long)4500);

		value_destroy(ret);

		ret = metacall("sum", 3, 4);

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((long)value_to_long(ret), (long)7);

		value_destroy(ret);

		EXPECT_EQ((value)NULL, (value)metacall("hello"));

		ret = metacall("strcat", "Hello ", "Universe");

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((int)0, (int)strcmp(value_to_string(ret), "Hello Universe"));

		value_destroy(ret);
	}
#endif /* OPTION_BUILD_PLUGINS_PY */

	/* Ruby */
#if defined(OPTION_BUILD_PLUGINS_RB)
	{
		const char * rb_scripts[] =
		{
			"hello.rb", "second.rb"
		};

		value ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("rb", rb_scripts, sizeof(rb_scripts) / sizeof(rb_scripts[0])));

		ret = metacall("say_multiply", 5, 7);

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((int)value_to_int(ret), (int)35);

		value_destroy(ret);

		EXPECT_EQ((void *)NULL, (void *)metacall("say_null"));

		ret = metacall("say_hello", "meta-programmer");

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((int)0, (int)strcmp(value_to_string(ret), "Hello meta-programmer!"));

		value_destroy(ret);

		ret = metacall("get_second", 5, 12);

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((int)value_to_int(ret), (int)12);

		value_destroy(ret);
	}
#endif /* OPTION_BUILD_PLUGINS_RB */

	/* JavaScript SpiderMonkey */
#if defined(OPTION_BUILD_PLUGINS_JSM)
	{
		const char * jsm_scripts[] =
		{
			"spider.jsm"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_file("jsm", jsm_scripts, sizeof(jsm_scripts) / sizeof(jsm_scripts[0])));

		EXPECT_EQ((void *)NULL, (void *)metacall("say_spider", 8, 4));
	}
#endif /* OPTION_BUILD_PLUGINS_JSM */

	/* JavaScript V8 */
#if defined(OPTION_BUILD_PLUGINS_JS)
	{
		const char * js_scripts[] =
		{
			"divide.js", "third.js"
		};

		value ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("js", js_scripts, sizeof(js_scripts) / sizeof(js_scripts[0])));

		ret = metacall("say_divide", 32.0, 4.0);

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((double)value_to_double(ret), (double) 8.0);

		value_destroy(ret);

		ret = metacall("some_text", "abc", "def");

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((int)0, (int)strcmp(value_to_string(ret), "abcdef"));

		value_destroy(ret);

		ret = metacall("third_value", "abc", "def", "efg");

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((int)0, (int)strcmp(value_to_string(ret), "efg"));

		value_destroy(ret);
	}
#endif /* OPTION_BUILD_PLUGINS_JS */

	/* Mock */
#if defined(OPTION_BUILD_PLUGINS_MOCK)
	{
		const char * mock_scripts[] =
		{
			"empty.mock"
		};

		value ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("mock", mock_scripts, sizeof(mock_scripts) / sizeof(mock_scripts[0])));

		ret = metacall("my_empty_func");

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((int)value_to_int(ret), (int)1234);

		value_destroy(ret);

		ret = metacall("two_doubles", 3.14, 68.3);

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((double)value_to_double(ret), (double) 3.1416);

		value_destroy(ret);

		ret = metacall("mixed_args", 'E', 16, 34L, 4.6, "hello");

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((char)value_to_char(ret), (char) 'A');

		value_destroy(ret);

		ret = metacall("new_args", "goodbye");

		EXPECT_NE((value)NULL, (value)ret);

		EXPECT_EQ((int)0, (int)strcmp(value_to_string(ret), "Hello World"));

		value_destroy(ret);
	}
#endif /* OPTION_BUILD_PLUGINS_MOCK */

	/* C# Netcore */
#if defined(OPTION_BUILD_PLUGINS_CS)
	{
		const char * cs_scripts[] =
		{
			"hello.cs"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_file("cs", cs_scripts, sizeof(cs_scripts) / sizeof(cs_scripts[0])));

		EXPECT_EQ((void *)NULL, (void *)metacall("Say", "Hello para with params!"));
	}
#endif /* OPTION_BUILD_PLUGINS_CS */

	/* C */
#if defined(OPTION_BUILD_PLUGINS_C)
	{
		const char * c_scripts[] =
		{
			"compiled.c"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_file("c", c_scripts, sizeof(c_scripts) / sizeof(c_scripts[0])));
	}
#endif /* OPTION_BUILD_PLUGINS_C */

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
