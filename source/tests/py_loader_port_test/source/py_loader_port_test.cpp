/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading python code at run-time into a process.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall-plugins.h>

class py_loader_port_test : public testing::Test
{
public:
};

void * callback_host(void * args[])
{
	const char * str = metacall_value_to_string(args[0]);

	printf("Host callback: %s\n", str);

	EXPECT_EQ((int) 0, (int) strcmp(str, "some text"));

	return metacall_value_create_int(25);
}

TEST_F(py_loader_port_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Native register */
	{
		metacall_register("callback_host", callback_host, METACALL_INT, 1, METACALL_STRING);

		EXPECT_NE((void *) NULL, (void *) metacall_function("callback_host"));
	}

	/* Ruby */
	#if defined(OPTION_BUILD_PLUGINS_RB)
	{
		const char * rb_scripts[] =
		{
			"hello.rb", "second.rb"
		};

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("rb", rb_scripts, sizeof(rb_scripts) / sizeof(rb_scripts[0])));

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
	}
	#endif /* OPTION_BUILD_PLUGINS_RB */

	/* Python */
	#if defined(OPTION_BUILD_PLUGINS_PY)
	{
		const char * py_scripts[] =
		{
			"callback.py"
		};

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0])));

		ret = metacall("hello_world", "some text");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 25, (int) metacall_value_to_int(ret));

		metacall_value_destroy(ret);

		ret = metacall("hello_ruby", 12, 4);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 48, (int) metacall_value_to_int(ret));

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_PLUGINS_PY */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
