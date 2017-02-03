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
#include <reflect/reflect_value_type_cast.h>

#include <log/log.h>

class metacall_cast_test : public testing::Test
{
public:
};

TEST_F(metacall_cast_test, DefaultConstructor)
{
	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	metacall_print_info();

	EXPECT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_PLUGINS_PY)
	{
		const char * py_scripts[] =
		{
			"example.py"
		};

		const int seven_multiples_limit = 10;

		int iterator;

		value ret = NULL;

		value args[2];

		EXPECT_EQ((int) 0, (int) metacall_load_from_file(py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0])));

		args[0] = value_create_int(5);
		args[1] = value_create_int(15);

		ret = metacallv("multiply", args);

		EXPECT_EQ((int) 75, (int) value_to_int(ret));

		value_destroy(ret);

		log_write("metacall", LOG_LEVEL_DEBUG, "7's multiples dude!");

		args[0] = value_from_int(args[0], 7);

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			args[1] = value_from_int(args[1], iterator);

			ret = metacallv("multiply", args);

			EXPECT_NE((value) NULL, (value) ret);

			ret = value_type_cast(ret, TYPE_INT);

			EXPECT_EQ((int) (7 * iterator), (int) value_to_int(ret));

			value_destroy(ret);
		}

		args[0] = value_from_float(value_type_cast(args[0], TYPE_FLOAT), 64.0f);
		args[1] = value_from_float(value_type_cast(args[1], TYPE_FLOAT), 2.0f);

		ret = metacallv("divide", args);

		EXPECT_NE((value) NULL, (value) ret);

		ret = value_type_cast(ret, TYPE_FLOAT);

		EXPECT_EQ((float) 32.0f, (float) value_to_float(ret));

		value_destroy(ret);

		args[0] = value_from_int(value_type_cast(args[0], TYPE_INT), 1000);
		args[1] = value_from_int(value_type_cast(args[1], TYPE_INT), 3500);

		ret = metacallv("sum", args);

		EXPECT_NE((value) NULL, (value) ret);

		ret = value_type_cast(ret, TYPE_INT);

		EXPECT_EQ((int) 4500, (int) value_to_int(ret));

		value_destroy(ret);

		value_destroy(args[0]);
		value_destroy(args[1]);
	}
	#endif /* OPTION_BUILD_PLUGINS_PY */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
