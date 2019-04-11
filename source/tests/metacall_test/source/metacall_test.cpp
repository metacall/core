/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

void * c_function(void * args[])
{
	printf("%s\n", (char*)args[0]);

	return metacall_value_create_int(1);
}

class metacall_test : public testing::Test
{
public:
};

TEST_F(metacall_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_stdio_type log_stdio = { stdout };

	ASSERT_EQ((int) 0, (int) metacall_log(METACALL_LOG_STDIO, (void *)&log_stdio));

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Native register */
	{
		void * ret = NULL;

		void * func = NULL;

		int result = 0;

		metacall_register("c_print", c_function, METACALL_INT, 1, METACALL_STRING);

		ret = metacall("c_print", "Hello native function!");

		EXPECT_NE((void *) NULL, (void *) ret);

		result = metacall_value_to_int(ret);

		EXPECT_EQ((int) result, (int) 1);

		metacall_value_destroy(ret);

		func = metacall_function("c_print");

		EXPECT_NE((void *) NULL, (void *) func);

		ret = metacallf(func, "Hello native function!");

		EXPECT_NE((void *) NULL, (void *) ret);

		result = metacall_value_to_int(ret);

		EXPECT_EQ((int) result, (int) 1);

		metacall_value_destroy(ret);
	}

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char * py_scripts[] =
		{
			"example.py", "web.py"
		};

		const char web_content[] =
			"<html>\n"
			"  <head>\n"
			"    <title>MetaCall Python Web</title>\n"
			"  </head>\n"
			"  <body>\n"
			"    Hello World\n"
			"  </body>\n"
			"</html>\n";

		const long seven_multiples_limit = 10;

		long iterator;

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		ret = metacall("multiply", 5, 15);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_to_long(ret), (long) 75);

		metacall_value_destroy(ret);

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			ret = metacall("multiply", 7, iterator);

			EXPECT_NE((void *) NULL, (void *) ret);

			EXPECT_EQ((long) metacall_value_to_long(ret), (long) (7 * iterator));

			metacall_value_destroy(ret);
		}

		ret = metacall("divide", 64.0, 2.0);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_to_double(ret), (double) 32.0);

		metacall_value_destroy(ret);

		ret = metacall("sum", 1000, 3500);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_to_long(ret), (long) 4500);

		metacall_value_destroy(ret);

		ret = metacall("sum", 3, 4);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_to_long(ret), (long) 7);

		metacall_value_destroy(ret);

		EXPECT_EQ((void *) NULL, (void *)metacall("hello"));

		ret = metacall("strcat", "Hello ", "Universe");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 0, (int) strcmp(metacall_value_to_string(ret), "Hello Universe"));

		metacall_value_destroy(ret);

		ret = metacall("bytebuff");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((size_t) sizeof("hello world"), (size_t) metacall_value_size(ret));

		EXPECT_EQ((int) 0, (int) memcmp(metacall_value_to_buffer(ret), "hello world", metacall_value_size(ret)));

		metacall_value_destroy(ret);

		ret = metacall("index");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 0, (int) strcmp(metacall_value_to_string(ret), web_content));

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_LOADERS_PY */

	/* Ruby */
	#if defined(OPTION_BUILD_LOADERS_RB)
	{
		const char * rb_scripts[] =
		{
			"hello.rb", "second.rb"
		};

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("rb", rb_scripts, sizeof(rb_scripts) / sizeof(rb_scripts[0]), NULL));

		ret = metacall("say_multiply", 5, 7);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) metacall_value_to_int(ret), (int) 35);

		metacall_value_destroy(ret);

		EXPECT_EQ((void *) NULL, (void *) metacall("say_null"));

		ret = metacall("say_hello", "meta-programmer");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 0, (int) strcmp(metacall_value_to_string(ret), "Hello meta-programmer!"));

		metacall_value_destroy(ret);

		ret = metacall("get_second", 5, 12);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) metacall_value_to_int(ret), (int) 12);

		metacall_value_destroy(ret);

		const enum metacall_value_id backwards_prime_int_ids[] =
		{
			METACALL_INT, METACALL_INT
		};

		ret = metacallt("backwardsPrime", backwards_prime_int_ids, 9900, 10000);

		EXPECT_NE((void *) NULL, (void *) ret);

		void ** v_array = metacall_value_to_array(ret);

		EXPECT_EQ((int) metacall_value_to_int(v_array[0]), (int) 9923);
		EXPECT_EQ((int) metacall_value_to_int(v_array[1]), (int) 9931);
		EXPECT_EQ((int) metacall_value_to_int(v_array[2]), (int) 9941);
		EXPECT_EQ((int) metacall_value_to_int(v_array[3]), (int) 9967);

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_LOADERS_RB */

	/* JavaScript SpiderMonkey */
	#if defined(OPTION_BUILD_LOADERS_JSM)
	{
		const char * jsm_scripts[] =
		{
			"spider.jsm"
		};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("jsm", jsm_scripts, sizeof(jsm_scripts) / sizeof(jsm_scripts[0]), NULL));

		EXPECT_EQ((void *) NULL, (void *) metacall("say_spider", 8, 4));
	}
	#endif /* OPTION_BUILD_LOADERS_JSM */

	/* JavaScript V8 */
	#if defined(OPTION_BUILD_LOADERS_JS) && !defined(OPTION_BUILD_LOADERS_NODE) /* TODO: NodeJS and V8 runtimes do not work at same time */
	{
		const char * js_scripts[] =
		{
			"divide.js", "third.js"
		};

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("js", js_scripts, sizeof(js_scripts) / sizeof(js_scripts[0]), NULL));

		ret = metacall("say_divide", 32.0, 4.0);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_to_double(ret), (double) 8.0);

		metacall_value_destroy(ret);

		ret = metacall("some_text", "abc", "def");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 0, (int) strcmp(metacall_value_to_string(ret), "abcdef"));

		metacall_value_destroy(ret);

		ret = metacall("third_value", "abc", "def", "efg");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 0, (int) strcmp(metacall_value_to_string(ret), "efg"));

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_LOADERS_JS */

	/* Mock */
	#if defined(OPTION_BUILD_LOADERS_MOCK)
	{
		const char * mock_scripts[] =
		{
			"empty.mock"
		};

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("mock", mock_scripts, sizeof(mock_scripts) / sizeof(mock_scripts[0]), NULL));

		ret = metacall("my_empty_func");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) metacall_value_to_int(ret), (int) 1234);

		metacall_value_destroy(ret);

		ret = metacall("two_doubles", 3.14, 68.3);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_to_double(ret), (double) 3.1416);

		metacall_value_destroy(ret);

		ret = metacall("mixed_args", 'E', 16, 34L, 4.6, "hello");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((char) metacall_value_to_char(ret), (char) 'A');

		metacall_value_destroy(ret);

		ret = metacall("new_args", "goodbye");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 0, (int) strcmp(metacall_value_to_string(ret), "Hello World"));

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_LOADERS_MOCK */

	/* C# NetCore */
	#if defined(OPTION_BUILD_LOADERS_CS)
	{
		const char * cs_scripts[] =
		{
			"hello.cs"
		};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("cs", cs_scripts, sizeof(cs_scripts) / sizeof(cs_scripts[0]), NULL));

		EXPECT_EQ((void *) NULL, (void *) metacall("Say", "Hello para with params!"));
	}
	#endif /* OPTION_BUILD_LOADERS_CS */

	/* C */
	#if defined(OPTION_BUILD_LOADERS_C)
	{
		const char * c_scripts[] =
		{
			"compiled.c"
		};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("c", c_scripts, sizeof(c_scripts) / sizeof(c_scripts[0]), NULL));
	}
	#endif /* OPTION_BUILD_LOADERS_C */

	/* NodeJS */
	#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char * node_scripts[] =
		{
			"nod.js"
		};

		const enum metacall_value_id hello_boy_double_ids[] =
		{
			METACALL_DOUBLE, METACALL_DOUBLE
		};

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("node", node_scripts, sizeof(node_scripts) / sizeof(node_scripts[0]), NULL));

		ret = metacallt("hello_boy", hello_boy_double_ids, 3.0, 4.0);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_to_double(ret), (double) 7.0);

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_LOADERS_NODE */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void * allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char * inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *) NULL, (char *) inspect_str);

		EXPECT_GT((size_t) size, (size_t) 0);

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
