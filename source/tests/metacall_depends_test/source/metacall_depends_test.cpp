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

class metacall_depends_test : public testing::Test
{
public:
};

TEST_F(metacall_depends_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_PLUGINS_PY)
	{
		const char * py_scripts[] =
		{
			"rsasample.py", "sample/rsa_strings.py"
		};

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		EXPECT_NE((void *) NULL, (void *) metacall_function("main"));

		ret = metacall("main");

		EXPECT_NE((void *) NULL, (void *) ret);

		metacall_value_destroy(ret);

		EXPECT_NE((void *) NULL, (void *) metacall_function("encript_decript_strings"));

		ret = metacall("encript_decript_strings");

		EXPECT_NE((void *) NULL, (void *) ret);

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_PLUGINS_PY */

	size_t inspect_buffer_size;

	char * inspect_buffer = metacall_inspect(&inspect_buffer_size);

	EXPECT_NE((const char *) NULL, (const char *) inspect_buffer);

	if (inspect_buffer != NULL)
	{
		free(inspect_buffer);
	}

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
