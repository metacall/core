/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

class metacall_python_gc_test : public testing::Test
{
public:
};

TEST_F(metacall_python_gc_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char * py_scripts[] =
		{
			"garbage.py"
		};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		EXPECT_EQ((void *) NULL, (void *) metacall("set_debug"));

		void * ret = metacall("garbage");

		ASSERT_NE((void *) NULL, (void *) ret);

		std::cout << metacall_value_to_string(ret) << std::endl;

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_LOADERS_PY */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
