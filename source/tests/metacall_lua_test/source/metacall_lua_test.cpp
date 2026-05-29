/*
 *	Loader Library by Parra Studios
 *	A plugin for loading lua code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_lua_test : public testing::Test
{
protected:
};

TEST_F(metacall_lua_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Lua */
#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Load from memory and call simple function */
		{
			static const char buffer[] =
				"function luamin(left, right)\n"
				"	if (left > right) then\n"
				"		return right;\n"
				"	else\n"
				"		return left;\n"
				"	end\n"
				"end\n";

			const enum metacall_value_id min_ids[] = {
				METACALL_FLOAT, METACALL_FLOAT
			};

			ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

			void *ret = metacallt_s("luamin", min_ids, 2, 3.0f, 6.0f);

			EXPECT_NE((void *)NULL, (void *)ret);

			/* Lua returns integers as long, check for long */
			if (metacall_value_id(ret) == METACALL_LONG)
			{
				EXPECT_EQ((long)3, (long)metacall_value_to_long(ret));
			}
			else if (metacall_value_id(ret) == METACALL_DOUBLE)
			{
				EXPECT_EQ((double)3.0, (double)metacall_value_to_double(ret));
			}
			else if (metacall_value_id(ret) == METACALL_FLOAT)
			{
				EXPECT_EQ((float)3.0f, (float)metacall_value_to_float(ret));
			}

			metacall_value_destroy(ret);
		}

		/* Load from file */
		{
			const char *lua_scripts[] = {
				"max.lua"
			};

			const enum metacall_value_id max_ids[] = {
				METACALL_INT, METACALL_INT
			};

			ASSERT_EQ((int)0, (int)metacall_load_from_file(tag, lua_scripts, sizeof(lua_scripts) / sizeof(lua_scripts[0]), NULL));

			void *ret = metacallt_s("luamax", max_ids, 2, 3, 6);

			EXPECT_NE((void *)NULL, (void *)ret);

			/* Lua returns numbers as long or double depending on value */
			if (metacall_value_id(ret) == METACALL_LONG)
			{
				EXPECT_EQ((long)6, (long)metacall_value_to_long(ret));
			}
			else if (metacall_value_id(ret) == METACALL_DOUBLE)
			{
				EXPECT_EQ((double)6.0, (double)metacall_value_to_double(ret));
			}

			metacall_value_destroy(ret);
		}

		/* Test different types: int, double, string, bool */
		{
			static const char type_test_buffer[] =
				"function type_test(int_val, double_val, str_val, bool_val)\n"
				"	return int_val + double_val, str_val, bool_val\n"
				"end\n"
				"function type_test_second(_, double_val, str_val, _)\n"
				"	return str_val, double_val\n"
				"end\n"
				"function string_concat(left, right)\n"
				"	return left .. right\n"
				"end\n"
				"function bool_not(val)\n"
				"	return not val\n"
				"end\n";

			ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, type_test_buffer, sizeof(type_test_buffer), NULL));

			/* Test int and double return */
			const enum metacall_value_id type_ids[] = {
				METACALL_INT, METACALL_DOUBLE, METACALL_STRING, METACALL_BOOL
			};

			void *ret_tuple = metacallt_s("type_test", type_ids, 4, 10, 5.5, "hello", 1);
			EXPECT_NE((void *)NULL, (void *)ret_tuple);
			EXPECT_EQ((int)METACALL_ARRAY, (int)metacall_value_id(ret_tuple));
			metacall_value_destroy(ret_tuple);

			void *ret_second = metacallt_s("type_test_second", type_ids, 4, 10, 5.5, "hello", 1);
			EXPECT_NE((void *)NULL, (void *)ret_second);
			EXPECT_EQ((int)METACALL_ARRAY, (int)metacall_value_id(ret_second));
			{
				void **ret_second_array = metacall_value_to_array(ret_second);
				EXPECT_EQ((int)METACALL_STRING, (int)metacall_value_id(ret_second_array[0]));
				if (metacall_value_id(ret_second_array[0]) == METACALL_STRING)
				{
					EXPECT_STREQ((const char *)metacall_value_to_string(ret_second_array[0]), "hello");
				}
			}
			metacall_value_destroy(ret_second);

			/* Test string concatenation */
			const enum metacall_value_id str_ids[] = {
				METACALL_STRING, METACALL_STRING
			};

			void *ret_str = metacallt_s("string_concat", str_ids, 2, "hello ", "world");
			EXPECT_NE((void *)NULL, (void *)ret_str);
			EXPECT_STREQ((const char *)metacall_value_to_string(ret_str), "hello world");
			metacall_value_destroy(ret_str);

			const enum metacall_value_id bool_ids[] = {
				METACALL_BOOL
			};

			void *ret_bool = metacallt_s("bool_not", bool_ids, 1, 1);
			EXPECT_NE((void *)NULL, (void *)ret_bool);
			EXPECT_EQ((int)metacall_value_to_bool(ret_bool), (int)0);
			metacall_value_destroy(ret_bool);

			ret_bool = metacallt_s("bool_not", bool_ids, 1, 0);
			EXPECT_NE((void *)NULL, (void *)ret_bool);
			EXPECT_EQ((int)metacall_value_to_bool(ret_bool), (int)1);
			metacall_value_destroy(ret_bool);
		}

		/* Test array passing */
		{
			static const char array_buffer[] =
				"function sum_array(arr)\n"
				"	local sum = 0\n"
				"	for i = 1, #arr do\n"
				"		sum = sum + arr[i]\n"
				"	end\n"
				"	return sum\n"
				"end\n"
				"function return_array()\n"
				"	return {1, 2, 3, 4, 5}\n"
				"end\n";

			ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, array_buffer, sizeof(array_buffer), NULL));

			/* Create array for testing */
			void *array_args[] = {
				metacall_value_create_array(NULL, 3)
			};

			/* Fill the array */
			void **array_data = metacall_value_to_array(array_args[0]);
			array_data[0] = metacall_value_create_int(1);
			array_data[1] = metacall_value_create_int(2);
			array_data[2] = metacall_value_create_int(3);

			void *ret_sum = metacallv_s("sum_array", array_args, 1);
			EXPECT_NE((void *)NULL, (void *)ret_sum);

			/* Lua returns numbers as long or double depending on value */
			if (metacall_value_id(ret_sum) == METACALL_LONG)
			{
				EXPECT_EQ((long)6, (long)metacall_value_to_long(ret_sum));
			}
			else if (metacall_value_id(ret_sum) == METACALL_DOUBLE)
			{
				EXPECT_EQ((double)6.0, (double)metacall_value_to_double(ret_sum));
			}

			metacall_value_destroy(ret_sum);
			metacall_value_destroy(array_args[0]);

			/* Test return array */
			void *ret_array = metacall("return_array");
			EXPECT_NE((void *)NULL, (void *)ret_array);
			metacall_value_destroy(ret_array);
		}

		/* Test error handling - loading non-existent file */
		{
			const char *non_existent_scripts[] = {
				"this_file_does_not_exist_12345.lua"
			};

			EXPECT_NE((int)0, (int)metacall_load_from_file(tag, non_existent_scripts, sizeof(non_existent_scripts) / sizeof(non_existent_scripts[0]), NULL));
		}

		/* Test error handling - invalid syntax in memory */
		{
			static const char invalid_buffer[] =
				"function invalid_syntax(\n"
				"	this is not valid lua code!!!\n"
				"end\n";

			EXPECT_NE((int)0, (int)metacall_load_from_memory(tag, invalid_buffer, sizeof(invalid_buffer), NULL));
		}

		/* Test error handling - calling non-existent function */
		{
			void *ret = metacall("non_existent_function_12345");
			EXPECT_EQ((void *)NULL, (void *)ret);
		}

		/* Test handle isolation between independent Lua loads */
		{
			static const char scope_a[] =
				"x = 111\n"
				"function get_x_a()\n"
				"\treturn x\n"
				"end\n";

			static const char scope_b[] =
				"x = 222\n"
				"function get_x_b()\n"
				"\treturn x\n"
				"end\n";

			ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, scope_a, sizeof(scope_a), NULL));
			ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, scope_b, sizeof(scope_b), NULL));

			void *ret_a = metacall("get_x_a");
			void *ret_b = metacall("get_x_b");

			ASSERT_NE((void *)NULL, (void *)ret_a);
			ASSERT_NE((void *)NULL, (void *)ret_b);

			if (metacall_value_id(ret_a) == METACALL_LONG)
			{
				EXPECT_EQ((long)111, (long)metacall_value_to_long(ret_a));
			}
			else if (metacall_value_id(ret_a) == METACALL_DOUBLE)
			{
				EXPECT_EQ((double)111.0, (double)metacall_value_to_double(ret_a));
			}

			if (metacall_value_id(ret_b) == METACALL_LONG)
			{
				EXPECT_EQ((long)222, (long)metacall_value_to_long(ret_b));
			}
			else if (metacall_value_id(ret_b) == METACALL_DOUBLE)
			{
				EXPECT_EQ((double)222.0, (double)metacall_value_to_double(ret_b));
			}

			metacall_value_destroy(ret_a);
			metacall_value_destroy(ret_b);
		}

		/* Wire and validate external test.lua script */
		{
			const char *lua_test_script[] = {
				"test.lua"
			};

			ASSERT_EQ((int)0, (int)metacall_load_from_file(tag, lua_test_script, 1, NULL));

			const enum metacall_value_id script_type_ids[] = {
				METACALL_INT, METACALL_DOUBLE, METACALL_STRING, METACALL_BOOL
			};

			void *ret_types = metacallt_s("lua_test_types", script_type_ids, 4, 2, 3.5, "ok", 1);
			ASSERT_NE((void *)NULL, (void *)ret_types);
			EXPECT_EQ((int)METACALL_ARRAY, (int)metacall_value_id(ret_types));
			{
				void **arr = metacall_value_to_array(ret_types);
				ASSERT_NE((void *)NULL, (void *)arr);
				EXPECT_EQ((int)METACALL_DOUBLE, (int)metacall_value_id(arr[0]));
				if (metacall_value_id(arr[0]) == METACALL_DOUBLE)
				{
					EXPECT_EQ((double)5.5, (double)metacall_value_to_double(arr[0]));
				}
			}
			metacall_value_destroy(ret_types);

			const enum metacall_value_id concat_ids[] = {
				METACALL_STRING, METACALL_STRING
			};

			void *ret_concat = metacallt_s("lua_test_concat_strings", concat_ids, 2, "foo", "bar");
			ASSERT_NE((void *)NULL, (void *)ret_concat);
			EXPECT_STREQ("foobar", metacall_value_to_string(ret_concat));
			metacall_value_destroy(ret_concat);

			const enum metacall_value_id bool_id[] = {
				METACALL_BOOL
			};

			void *ret_negate = metacallt_s("lua_test_negate_bool", bool_id, 1, 1);
			ASSERT_NE((void *)NULL, (void *)ret_negate);
			EXPECT_EQ((int)0, (int)metacall_value_to_bool(ret_negate));
			metacall_value_destroy(ret_negate);

			void *ret_array = metacall("lua_test_get_array");
			ASSERT_NE((void *)NULL, (void *)ret_array);
			EXPECT_EQ((int)METACALL_ARRAY, (int)metacall_value_id(ret_array));
			metacall_value_destroy(ret_array);

			void *ret_dict = metacall("lua_test_get_dict");
			ASSERT_NE((void *)NULL, (void *)ret_dict);
			EXPECT_EQ((int)METACALL_MAP, (int)metacall_value_id(ret_dict));
			metacall_value_destroy(ret_dict);
		}
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char *inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *)NULL, (char *)inspect_str);

		EXPECT_GT((size_t)size, (size_t)0);

		std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	metacall_destroy();
}
