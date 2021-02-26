/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_cast_test : public testing::Test
{
public:
};

TEST_F(metacall_cast_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_stdio_type log_stdio = { stdout };

	ASSERT_EQ((int) 0, (int) metacall_log(METACALL_LOG_STDIO, (void *)&log_stdio));

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char * py_scripts[] =
		{
			"example.py"
		};

		const int seven_multiples_limit = 10;

		int iterator;

		void * ret = NULL;

		void * args[2];

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		args[0] = metacall_value_create_long(5L);
		args[1] = metacall_value_create_long(15L);

		ret = metacallv("multiply", args);

		EXPECT_EQ((int) 75L, (int) metacall_value_to_long(ret));

		metacall_value_destroy(ret);

		args[0] = metacall_value_from_int(args[0], 7);

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			args[1] = metacall_value_from_int(args[1], iterator);

			ret = metacallv("multiply", args);

			EXPECT_NE((void *) NULL, (void *) ret);

			EXPECT_EQ((int) (7 * iterator), (int) metacall_value_cast_int(&ret));

			metacall_value_destroy(ret);
		}

		args[0] = metacall_value_from_float(metacall_value_cast(args[0], METACALL_FLOAT), 64.0f);
		args[1] = metacall_value_from_float(metacall_value_cast(args[1], METACALL_FLOAT), 2.0f);

		ret = metacallv("divide", args);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((float) 32.0f, (float) metacall_value_cast_float(&ret));

		metacall_value_destroy(ret);

		args[0] = metacall_value_from_int(metacall_value_cast(args[0], METACALL_INT), 1000);
		args[1] = metacall_value_from_int(metacall_value_cast(args[1], METACALL_INT), 3500);

		ret = metacallv("sum", args);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 4500, (int) metacall_value_cast_int(&ret));

		metacall_value_destroy(ret);

		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);
	}
	#endif /* OPTION_BUILD_LOADERS_PY */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
