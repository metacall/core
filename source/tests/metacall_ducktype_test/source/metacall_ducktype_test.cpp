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

class metacall_ducktype_test : public testing::Test
{
public:
};

TEST_F(metacall_ducktype_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_PLUGINS_PY)
	{
		/* Load */
		const char * py_scripts[] =
		{
			"ducktype.py"
		};

		const long seven_multiples_limit = 10;

		long iterator;

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0])));

		/* MetaCall Type */
		const enum metacall_value_id multiply_ids[] =
		{
			METACALL_INT, METACALL_INT
		};

		ret = metacallt("multiply", multiply_ids, 5, 15);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_cast_long(&ret), (long) 75);

		metacall_value_destroy(ret);

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			ret = metacallt("multiply", multiply_ids, 7, iterator);

			EXPECT_NE((void *) NULL, (void *) ret);

			EXPECT_EQ((long) metacall_value_cast_long(&ret), (long) (7 * iterator));

			metacall_value_destroy(ret);
		}

		const enum metacall_value_id divide_ids[] =
		{
			METACALL_DOUBLE, METACALL_DOUBLE
		};

		ret = metacallt("divide", divide_ids, 64.0, 2.0);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_cast_double(&ret), (double) 32.0);

		metacall_value_destroy(ret);

		const enum metacall_value_id sum_ids[] =
		{
			METACALL_INT, METACALL_INT
		};

		ret = metacallt("sum", sum_ids, 1000, 3500);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_cast_long(&ret), (long) 4500);

		metacall_value_destroy(ret);

		ret = metacallt("sum", sum_ids, 3, 4);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_cast_long(&ret), (long) 7);

		metacall_value_destroy(ret);

		EXPECT_EQ((void *) NULL, (void *) metacall("hello"));

		const enum metacall_value_id strcat_str_ids[] =
		{
			METACALL_STRING, METACALL_STRING
		};

		ret = metacallt("strcat", strcat_str_ids, "Hello ", "Universe");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 0, (int) strcmp(metacall_value_cast_string(&ret), "Hello Universe"));

		metacall_value_destroy(ret);

		const enum metacall_value_id strcat_float_ids[] =
		{
			METACALL_FLOAT, METACALL_FLOAT
		};

		ret = metacallt("strcat", strcat_float_ids, 10.0f, 20.0f);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((float) metacall_value_cast_float(&ret), (float) 30.0f);

		metacall_value_destroy(ret);

		/* MetaCall Type Value */
		void * args[2];

		args[0] = metacall_value_create_int(5);
		args[1] = metacall_value_create_int(15);

		ret = metacallv("multiply", args);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) metacall_value_cast_int(&ret), (int) 75);

		metacall_value_destroy(ret);

		args[0] = metacall_value_create_double(15.0);
		args[1] = metacall_value_create_double(5.0);

		ret = metacallv("divide", args);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_cast_double(&ret), (double) 3.0);

		metacall_value_destroy(ret);

		const char pepico_str[] = "Pepico";
		const char walas_str[] = "Walas";

		args[0] = metacall_value_create_string(pepico_str, sizeof(pepico_str));
		args[1] = metacall_value_create_string(walas_str, sizeof(walas_str));

		ret = metacallv("strcat", args);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 0, (int) strcmp(metacall_value_cast_string(&ret), "PepicoWalas"));

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_PLUGINS_PY */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
