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

class metacall_map_test : public testing::Test
{
public:
};

TEST_F(metacall_map_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_PLUGINS_PY)
	{
		const char * py_scripts[] =
		{
			"example.py"
		};

		const long seven_multiples_limit = 10;

		long iterator;

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		static const char left[] = "left";
		static const char right[] = "right";

		void * keys[] =
		{
			metacall_value_create_string(left, sizeof(left) - 1),
			metacall_value_create_string(right, sizeof(right) - 1)
		};

		void * values[] =
		{
			metacall_value_create_long(7),
			metacall_value_create_long(0)
		};

		void * func = metacall_function("multiply");

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			values[1] = metacall_value_from_long(values[1], iterator);

			ret = metacallfmv(func, keys, values);

			EXPECT_NE((void *) NULL, (void *) ret);

			EXPECT_EQ((long) metacall_value_to_long(ret), (long) (7 * iterator));
		}

		metacall_value_destroy(keys[0]);
		metacall_value_destroy(keys[1]);

		metacall_value_destroy(values[0]);
		metacall_value_destroy(values[1]);
	}
	#endif /* OPTION_BUILD_PLUGINS_PY */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
