/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
#include <metacall/metacall_value.h>

class metacall_ducktype_test : public testing::Test
{
public:
};

TEST_F(metacall_ducktype_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char *py_scripts[] = {
			"ducktype.py"
		};

		const long seven_multiples_limit = 10;

		long iterator;

		void *ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		const enum metacall_value_id multiply_ids[] = {
			METACALL_INT, METACALL_INT
		};

		ret = metacallt("multiply", multiply_ids, 5, 15);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((long)metacall_value_cast_long(&ret), (long)75);

		metacall_value_destroy(ret);

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			ret = metacallt("multiply", multiply_ids, 7, iterator);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((long)metacall_value_cast_long(&ret), (long)(7 * iterator));

			metacall_value_destroy(ret);
		}

		const enum metacall_value_id divide_ids[] = {
			METACALL_DOUBLE, METACALL_DOUBLE
		};

		ret = metacallt("divide", divide_ids, 64.0, 2.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_cast_double(&ret), (double)32.0);

		metacall_value_destroy(ret);

		const enum metacall_value_id sum_int_ids[] = {
			METACALL_INT, METACALL_INT
		};

		ret = metacallt("sum", sum_int_ids, 1000, 3500);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((long)metacall_value_cast_long(&ret), (long)4500);

		metacall_value_destroy(ret);

		ret = metacallt("sum", sum_int_ids, 3, 4);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((long)metacall_value_cast_long(&ret), (long)7);

		metacall_value_destroy(ret);

		ret = metacall("hello");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_NULL, (enum metacall_value_id)metacall_value_id(ret));

		EXPECT_EQ((void *)NULL, (void *)metacall_value_to_null(ret));

		metacall_value_destroy(ret);

		const enum metacall_value_id strcat_str_ids[] = {
			METACALL_STRING, METACALL_STRING
		};

		ret = metacallt("strcat", strcat_str_ids, "Hello ", "Universe");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_cast_string(&ret), "Hello Universe"));

		metacall_value_destroy(ret);

		const enum metacall_value_id strcat_float_ids[] = {
			METACALL_FLOAT, METACALL_FLOAT
		};

		ret = metacallt("strcat", strcat_float_ids, 10.0f, 20.0f);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((float)metacall_value_cast_float(&ret), (float)30.0f);

		metacall_value_destroy(ret);

		ret = metacall("old_style", 1, 2);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_cast_int(&ret), (int)3);

		metacall_value_destroy(ret);

		const enum metacall_value_id mixed_int_ids[] = {
			METACALL_INT, METACALL_INT
		};

		ret = metacallt("mixed_style", mixed_int_ids, 200, 100);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((long)metacall_value_cast_long(&ret), (long)300);

		metacall_value_destroy(ret);

		const enum metacall_value_id mixed_noret_int_ids[] = {
			METACALL_INT, METACALL_INT
		};

		ret = metacallt("mixed_style_noreturn", mixed_noret_int_ids, 2, 6);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_cast_int(&ret), (int)8);

		metacall_value_destroy(ret);

		void *args[2];

		args[0] = metacall_value_create_int(5);
		args[1] = metacall_value_create_int(15);

		ret = metacallv("multiply", args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_cast_int(&ret), (int)75);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);

		args[0] = metacall_value_create_double(15.0);
		args[1] = metacall_value_create_double(5.0);

		ret = metacallv("divide", args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_cast_double(&ret), (int)3.0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);

		static const char pepico_str[] = "Pepico";
		static const char walas_str[] = "Walas";

		args[0] = metacall_value_create_string(pepico_str, sizeof(pepico_str) - 1);
		args[1] = metacall_value_create_string(walas_str, sizeof(walas_str) - 1);

		ret = metacallv("strcat", args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_cast_string(&ret), "PepicoWalas"));

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);
	}
#endif /* OPTION_BUILD_LOADERS_PY */

/* Ruby */
#if defined(OPTION_BUILD_LOADERS_RB)
	{
		const char *rb_scripts[] = {
			"ducktype.rb"
		};

		void *ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("rb", rb_scripts, sizeof(rb_scripts) / sizeof(rb_scripts[0]), NULL));

		const enum metacall_value_id say_multiply_int_ids[] = {
			METACALL_INT, METACALL_INT
		};

		ret = metacallt("say_multiply", say_multiply_int_ids, 5, 7);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_cast_int(&ret), (int)35);

		metacall_value_destroy(ret);

		const enum metacall_value_id say_null_invalid_ids[] = {
			METACALL_INVALID
		};

		ret = metacallt("say_null", say_null_invalid_ids);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_NULL, (enum metacall_value_id)metacall_value_id(ret));

		metacall_value_destroy(ret);

		const enum metacall_value_id say_hello_str_ids[] = {
			METACALL_STRING
		};

		ret = metacallt("say_hello", say_hello_str_ids, "meta-programmer");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_cast_string(&ret), "Hello meta-programmer!"));

		metacall_value_destroy(ret);

		const enum metacall_value_id mixed_int_ids[] = {
			METACALL_INT, METACALL_INT, METACALL_INT, METACALL_INT
		};

		ret = metacallt("mixed", mixed_int_ids, 1, 2, 3, 4);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_cast_int(&ret), (int)10);

		metacall_value_destroy(ret);

		ret = metacall("map_style", 1, 2);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_to_int(ret), (int)3);

		metacall_value_destroy(ret);

		void *args[2];

		args[0] = metacall_value_create_int(5);
		args[1] = metacall_value_create_int(12);

		ret = metacallv("get_second", args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_cast_int(&ret), (int)12);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);
	}
#endif /* OPTION_BUILD_LOADERS_RB */

/* JavaScript V8 */
#if defined(OPTION_BUILD_LOADERS_JS)
	{
		const char *js_scripts[] = {
			"ducktype.js"
		};

		void *ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("js", js_scripts, sizeof(js_scripts) / sizeof(js_scripts[0]), NULL));

		const enum metacall_value_id say_divide_double_ids[] = {
			METACALL_DOUBLE, METACALL_DOUBLE
		};

		ret = metacallt("say_divide", say_divide_double_ids, 32.0, 4.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_cast_double(&ret), (double)8.0);

		metacall_value_destroy(ret);

		const enum metacall_value_id say_divide_float_ids[] = {
			METACALL_FLOAT, METACALL_FLOAT
		};

		ret = metacallt("say_divide", say_divide_float_ids, 32.0f, 4.0f);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((float)metacall_value_cast_float(&ret), (float)8.0f);

		metacall_value_destroy(ret);

		const enum metacall_value_id some_text_str_ids[] = {
			METACALL_STRING, METACALL_STRING
		};

		ret = metacallt("some_text", some_text_str_ids, "abc", "def");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_cast_string(&ret), "abcdef"));

		metacall_value_destroy(ret);

		const enum metacall_value_id mixed_double_ids[] = {
			METACALL_DOUBLE, METACALL_DOUBLE, METACALL_DOUBLE, METACALL_DOUBLE
		};

		ret = metacallt("mixed_js", mixed_double_ids, 1.0, 2.0, 3.0, 4.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_cast_double(&ret), (double)10.0);

		metacall_value_destroy(ret);

		const enum metacall_value_id mixed_noreturn_double_ids[] = {
			METACALL_DOUBLE, METACALL_DOUBLE, METACALL_DOUBLE, METACALL_DOUBLE
		};

		ret = metacallt("mixed_noreturn", mixed_noreturn_double_ids, 1.0, 2.0, 3.0, 4.0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((double)metacall_value_cast_double(&ret), (double)10.0);

		metacall_value_destroy(ret);

		const enum metacall_value_id mixed_noreturn_float_ids[] = {
			METACALL_FLOAT, METACALL_FLOAT, METACALL_FLOAT, METACALL_FLOAT
		};

		ret = metacallt("mixed_noreturn", mixed_noreturn_float_ids, 1.0f, 2.0f, 3.0f, 4.0f);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((float)metacall_value_cast_float(&ret), (float)10.0f);

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_JS */

	metacall_destroy();
}
