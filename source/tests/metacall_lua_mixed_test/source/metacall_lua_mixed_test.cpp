/*
 *	Loader Library by Parra Studios
 *	A plugin for loading lua code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <cmath>
#include <cstring>

class metacall_lua_mixed_test : public testing::Test
{
protected:
};

/*
 * Numeric Boundary Tests
 * Tests integer and floating-point edge cases for LuaJIT 64-bit support
 */

TEST_F(metacall_lua_mixed_test, convert_integer_max_boundary)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Test INT64_MAX boundary value */
		static const char buffer[] =
			"function lua_test_int_max()\n"
			"	return 9223372036854775807\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_int_max");
		EXPECT_NE((void *)NULL, (void *)ret);

		/* LuaJIT returns this as a double since it exceeds 32-bit int range */
		if (metacall_value_id(ret) == METACALL_DOUBLE)
		{
			EXPECT_DOUBLE_EQ((double)9223372036854775807.0, (double)metacall_value_to_double(ret));
		}
		else if (metacall_value_id(ret) == METACALL_LONG)
		{
			EXPECT_EQ((long)9223372036854775807L, (long)metacall_value_to_long(ret));
		}

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_integer_min_boundary)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Test INT64_MIN boundary value */
		static const char buffer[] =
			"function lua_test_int_min()\n"
			"	return -9223372036854775808\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_int_min");
		EXPECT_NE((void *)NULL, (void *)ret);

		if (metacall_value_id(ret) == METACALL_DOUBLE)
		{
			EXPECT_DOUBLE_EQ((double)-9223372036854775808.0, (double)metacall_value_to_double(ret));
		}
		else if (metacall_value_id(ret) == METACALL_LONG)
		{
			EXPECT_EQ((long)-9223372036854775808L, (long)metacall_value_to_long(ret));
		}

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_double_infinity)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Test positive infinity */
		static const char buffer[] =
			"function lua_test_double_inf()\n"
			"	return math.huge\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_double_inf");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(ret));

		double val = metacall_value_to_double(ret);
		EXPECT_TRUE(std::isinf(val) && val > 0);

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_double_negative_infinity)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Test negative infinity */
		static const char buffer[] =
			"function lua_test_double_neg_inf()\n"
			"	return -math.huge\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_double_neg_inf");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(ret));

		double val = metacall_value_to_double(ret);
		EXPECT_TRUE(std::isinf(val) && val < 0);

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_double_nan)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Test NaN conversion */
		static const char buffer[] =
			"function lua_test_double_nan()\n"
			"	return 0 / 0\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_double_nan");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(ret));

		double val = metacall_value_to_double(ret);
		EXPECT_TRUE(std::isnan(val));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_double_precision)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Test double precision with fractional values */
		static const char buffer[] =
			"function lua_test_double_precision()\n"
			"	return 3.14159265358979323846\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_double_precision");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(ret));

		double val = metacall_value_to_double(ret);
		EXPECT_DOUBLE_EQ((double)3.14159265358979323846, val);

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

/*
 * String Edge Case Tests
 */

TEST_F(metacall_lua_mixed_test, convert_empty_string)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		static const char buffer[] =
			"function lua_test_empty_string()\n"
			"	return ''\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_empty_string");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_STRING, (enum metacall_value_id)metacall_value_id(ret));

		const char *str = metacall_value_to_string(ret);
		EXPECT_STREQ("", str);

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_unicode_string)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Test UTF-8 string with various Unicode characters */
		static const char buffer[] =
			"function lua_test_unicode()\n"
			"	return 'Hello \\228\\184\\226\\149\\144 \\240\\159\\140\\141'\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_unicode");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_STRING, (enum metacall_value_id)metacall_value_id(ret));

		const char *str = metacall_value_to_string(ret);
		EXPECT_NE((const char *)NULL, (const char *)str);
		EXPECT_GT((size_t)strlen(str), (size_t)5); /* Should be longer than ASCII equivalent */

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_string_with_special_chars)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Test string with escape sequences */
		static const char buffer[] =
			"function lua_test_special_chars()\n"
			"	return 'line1\\nline2\\ttabbed\\\\backslash'\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_special_chars");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_STRING, (enum metacall_value_id)metacall_value_id(ret));

		const char *str = metacall_value_to_string(ret);
		EXPECT_NE((const char *)NULL, (const char *)str);
		/* The string should contain actual newline and tab characters */
		EXPECT_NE((const char *)NULL, (const char *)strchr(str, '\n'));
		EXPECT_NE((const char *)NULL, (const char *)strchr(str, '\t'));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_long_string)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Test with a moderately long string (10KB) */
		static const char buffer[] =
			"function lua_test_long_string()\n"
			"	local s = ''\n"
			"	for i = 1, 10000 do\n"
			"		s = s .. 'a'\n"
			"	end\n"
			"	return s\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_long_string");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_STRING, (enum metacall_value_id)metacall_value_id(ret));

		const char *str = metacall_value_to_string(ret);
		EXPECT_NE((const char *)NULL, (const char *)str);
		EXPECT_EQ((size_t)10000, (size_t)strlen(str));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

/*
 * Container Edge Case Tests
 */

TEST_F(metacall_lua_mixed_test, convert_empty_table_as_map)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Empty tables are converted to maps (not arrays) */
		static const char buffer[] =
			"function lua_test_empty_table()\n"
			"	return {}\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_empty_table");
		EXPECT_NE((void *)NULL, (void *)ret);
		/* Empty table is treated as a map (size 0) */
		EXPECT_EQ((enum metacall_value_id)METACALL_MAP, (enum metacall_value_id)metacall_value_id(ret));
		EXPECT_EQ((size_t)0, (size_t)metacall_value_count(ret));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_sparse_array_with_nil)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Array with holes (nil values) */
		static const char buffer[] =
			"function lua_test_sparse_array()\n"
			"	return {1, nil, 3}\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_sparse_array");
		EXPECT_NE((void *)NULL, (void *)ret);

		/* Sparse arrays with nil become maps due to non-consecutive keys */
		if (metacall_value_id(ret) == METACALL_ARRAY)
		{
			/* If detected as array, check structure */
			size_t count = metacall_value_count(ret);
			EXPECT_GE((size_t)count, (size_t)2);
		}

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_mixed_key_table)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Table with both numeric and string keys becomes a map */
		static const char buffer[] =
			"function lua_test_mixed_table()\n"
			"	return {1, 2, key = 'value'}\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_mixed_table");
		EXPECT_NE((void *)NULL, (void *)ret);
		/* Mixed key tables are maps, not arrays */
		EXPECT_EQ((enum metacall_value_id)METACALL_MAP, (enum metacall_value_id)metacall_value_id(ret));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_zero_indexed_table)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Lua arrays are 1-based; 0 index makes it a map */
		static const char buffer[] =
			"function lua_test_zero_indexed()\n"
			"	return {[0] = 'zero', [1] = 'one'}\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_zero_indexed");
		EXPECT_NE((void *)NULL, (void *)ret);
		/* Zero index breaks consecutive 1-based array detection */
		EXPECT_EQ((enum metacall_value_id)METACALL_MAP, (enum metacall_value_id)metacall_value_id(ret));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_negative_indexed_table)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Negative indices make it a map, not an array */
		static const char buffer[] =
			"function lua_test_negative_indexed()\n"
			"	return {[-1] = 'negative', [1] = 'positive'}\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_negative_indexed");
		EXPECT_NE((void *)NULL, (void *)ret);
		/* Negative index invalidates array detection */
		EXPECT_EQ((enum metacall_value_id)METACALL_MAP, (enum metacall_value_id)metacall_value_id(ret));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_deeply_nested_structure)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Test deep nesting within recursion limit (10) */
		static const char buffer[] =
			"function lua_test_deep_nest()\n"
			"	return {a = {b = {c = {d = {e = {f = {g = {h = {i = {j = 10}}}}}}}}}}\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_deep_nest");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_MAP, (enum metacall_value_id)metacall_value_id(ret));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_exceeds_recursion_limit)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Test nesting beyond recursion limit (15 levels > 10 limit) */
		static const char buffer[] =
			"function lua_test_exceed_recursion()\n"
			"	return {a={b={c={d={e={f={g={h={i={j={k={l={m={n={o=15}}}}}}}}}}}}}}}\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_exceed_recursion");
		/* Should handle gracefully - either NULL or truncated */
		(void)ret;
		if (ret != NULL)
		{
			metacall_value_destroy(ret);
		}
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, convert_large_array)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Test with a large array (1000 elements) */
		static const char buffer[] =
			"function lua_test_large_array()\n"
			"	local arr = {}\n"
			"	for i = 1, 1000 do\n"
			"		arr[i] = i\n"
			"	end\n"
			"	return arr\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_large_array");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_ARRAY, (enum metacall_value_id)metacall_value_id(ret));

		size_t count = metacall_value_count(ret);
		EXPECT_EQ((size_t)1000, (size_t)count);

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

/*
 * Function Call Variation Tests
 */

TEST_F(metacall_lua_mixed_test, call_zero_argument_function)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		static const char buffer[] =
			"function lua_test_no_args()\n"
			"	return 42\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_no_args");
		EXPECT_NE((void *)NULL, (void *)ret);

		if (metacall_value_id(ret) == METACALL_INT)
		{
			EXPECT_EQ((int)42, (int)metacall_value_to_int(ret));
		}
		else if (metacall_value_id(ret) == METACALL_DOUBLE)
		{
			EXPECT_DOUBLE_EQ((double)42.0, (double)metacall_value_to_double(ret));
		}

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, call_function_returning_nil)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		static const char buffer[] =
			"function lua_test_return_nil()\n"
			"	return nil\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_return_nil");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_NULL, (enum metacall_value_id)metacall_value_id(ret));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, call_function_no_explicit_return)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		static const char buffer[] =
			"function lua_test_no_return()\n"
			"	local x = 10\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_no_return");
		/* Function with no explicit return returns nil in Lua */
		EXPECT_EQ((void *)NULL, (void *)ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, call_function_multiple_returns)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		static const char buffer[] =
			"function lua_test_multi_return()\n"
			"	return 1, 2, 3\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_multi_return");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_ARRAY, (enum metacall_value_id)metacall_value_id(ret));

		size_t count = metacall_value_count(ret);
		EXPECT_EQ((size_t)3, (size_t)count);

		void **values = metacall_value_to_array(ret);
		EXPECT_NE((void **)NULL, (void **)values);

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, call_variadic_function)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		static const char buffer[] =
			"function lua_test_variadic(...)\n"
			"	local args = {...}\n"
			"	return #args\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		/* Call with 5 arguments using array interface (variadic functions need this) */
		void *args[5];
		args[0] = metacall_value_create_int(1);
		args[1] = metacall_value_create_int(2);
		args[2] = metacall_value_create_int(3);
		args[3] = metacall_value_create_int(4);
		args[4] = metacall_value_create_int(5);

		void *ret = metacallv_s("lua_test_variadic", args, 5);
		EXPECT_NE((void *)NULL, (void *)ret);

		int count = 0;
		if (metacall_value_id(ret) == METACALL_INT)
		{
			count = metacall_value_to_int(ret);
		}
		else if (metacall_value_id(ret) == METACALL_DOUBLE)
		{
			count = (int)metacall_value_to_double(ret);
		}

		EXPECT_EQ((int)5, count);

		metacall_value_destroy(ret);

		/* Clean up argument values */
		for (int i = 0; i < 5; i++)
		{
			metacall_value_destroy(args[i]);
		}
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, call_function_with_many_arguments)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Function accepting and summing 10 arguments */
		static const char buffer[] =
			"function lua_test_many_args(a, b, c, d, e, f, g, h, i, j)\n"
			"	return a + b + c + d + e + f + g + h + i + j\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		const enum metacall_value_id arg_types[] = {
			METACALL_INT, METACALL_INT, METACALL_INT, METACALL_INT, METACALL_INT,
			METACALL_INT, METACALL_INT, METACALL_INT, METACALL_INT, METACALL_INT
		};

		void *ret = metacallt_s("lua_test_many_args", arg_types, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
		EXPECT_NE((void *)NULL, (void *)ret);

		int sum = 0;
		if (metacall_value_id(ret) == METACALL_INT)
		{
			sum = metacall_value_to_int(ret);
		}
		else if (metacall_value_id(ret) == METACALL_DOUBLE)
		{
			sum = (int)metacall_value_to_double(ret);
		}

		EXPECT_EQ((int)55, sum); /* 1+2+3+...+10 = 55 */

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

/*
 * Error Handling Tests
 */

TEST_F(metacall_lua_mixed_test, runtime_error_propagation)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		static const char buffer[] =
			"function lua_test_runtime_error()\n"
			"	error('Intentional test error')\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		/* Runtime errors should return NULL */
		void *ret = metacall("lua_test_runtime_error");
		EXPECT_EQ((void *)NULL, (void *)ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, runtime_error_with_nil_access)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		static const char buffer[] =
			"function lua_test_nil_access()\n"
			"	local t = nil\n"
			"	return t.value\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		/* Attempt to index nil should return NULL */
		void *ret = metacall("lua_test_nil_access");
		EXPECT_EQ((void *)NULL, (void *)ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, invalid_syntax_error)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Invalid Lua syntax */
		static const char buffer[] =
			"function lua_test_invalid_syntax(\n"
			"	this is not valid lua!!!\n"
			"end\n";

		/* Should fail to load */
		EXPECT_NE((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

/*
 * Resource Management Tests
 */

TEST_F(metacall_lua_mixed_test, multiple_handle_isolation)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Load two separate scripts with same variable names */
		static const char buffer_a[] =
			"x = 111\n"
			"function lua_test_get_x_a()\n"
			"	return x\n"
			"end\n";

		static const char buffer_b[] =
			"x = 222\n"
			"function lua_test_get_x_b()\n"
			"	return x\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer_a, sizeof(buffer_a), NULL));
		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer_b, sizeof(buffer_b), NULL));

		void *ret_a = metacall("lua_test_get_x_a");
		void *ret_b = metacall("lua_test_get_x_b");

		EXPECT_NE((void *)NULL, (void *)ret_a);
		EXPECT_NE((void *)NULL, (void *)ret_b);

		/* Verify isolation - each should return its own value */
		int val_a = 0, val_b = 0;

		if (metacall_value_id(ret_a) == METACALL_INT)
		{
			val_a = metacall_value_to_int(ret_a);
		}
		else if (metacall_value_id(ret_a) == METACALL_DOUBLE)
		{
			val_a = (int)metacall_value_to_double(ret_a);
		}

		if (metacall_value_id(ret_b) == METACALL_INT)
		{
			val_b = metacall_value_to_int(ret_b);
		}
		else if (metacall_value_id(ret_b) == METACALL_DOUBLE)
		{
			val_b = (int)metacall_value_to_double(ret_b);
		}

		EXPECT_EQ((int)111, val_a);
		EXPECT_EQ((int)222, val_b);

		metacall_value_destroy(ret_a);
		metacall_value_destroy(ret_b);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, load_after_failed_load)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* First, try to load invalid script */
		static const char invalid_buffer[] =
			"function invalid(\n"
			"	bad syntax here\n";

		EXPECT_NE((int)0, (int)metacall_load_from_memory(tag, invalid_buffer, sizeof(invalid_buffer), NULL));

		/* Then load valid script - should work */
		static const char valid_buffer[] =
			"function lua_test_after_fail()\n"
			"	return 'success'\n"
			"end\n";

		EXPECT_EQ((int)0, (int)metacall_load_from_memory(tag, valid_buffer, sizeof(valid_buffer), NULL));

		void *ret = metacall("lua_test_after_fail");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_STRING, (enum metacall_value_id)metacall_value_id(ret));
		EXPECT_STREQ("success", metacall_value_to_string(ret));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, mixed_types_in_array)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		static const char tag[] = "lua";

		/* Array with mixed types (no nil - Lua tables with nil create gaps) */
		static const char buffer[] =
			"function lua_test_mixed_array()\n"
			"	return {1, 'string', true, 3.14}\n"
			"end\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		void *ret = metacall("lua_test_mixed_array");
		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)METACALL_ARRAY, (enum metacall_value_id)metacall_value_id(ret));

		size_t count = metacall_value_count(ret);
		EXPECT_EQ((size_t)4, (size_t)count);

		void **values = metacall_value_to_array(ret);
		EXPECT_NE((void **)NULL, (void **)values);

		/* Verify mixed types */
		EXPECT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(values[0]));
		EXPECT_EQ((enum metacall_value_id)METACALL_STRING, (enum metacall_value_id)metacall_value_id(values[1]));
		EXPECT_EQ((enum metacall_value_id)METACALL_BOOL, (enum metacall_value_id)metacall_value_id(values[2]));
		EXPECT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(values[3]));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}

TEST_F(metacall_lua_mixed_test, load_and_call_external_script)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_LUA)
	{
		const char tag[] = "lua";

		/* Load the external mixed_test.lua script */
		const char *lua_scripts[] = {
			"mixed_test.lua"
		};

		ASSERT_EQ((int)0, (int)metacall_load_from_file(tag, lua_scripts, 1, NULL));

		/* Test numeric boundary from external script */
		void *ret_max = metacall("lua_mixed_max_int");
		EXPECT_NE((void *)NULL, (void *)ret_max);
		metacall_value_destroy(ret_max);

		/* Test string operations from external script */
		const enum metacall_value_id str_ids[] = { METACALL_STRING, METACALL_STRING };
		void *ret_concat = metacallt_s("lua_mixed_concat", str_ids, 2, "Hello", "World");
		EXPECT_NE((void *)NULL, (void *)ret_concat);
		EXPECT_STREQ("HelloWorld", metacall_value_to_string(ret_concat));
		metacall_value_destroy(ret_concat);

		/* Test nested table access from external script */
		void *ret_nested = metacall("lua_mixed_get_nested");
		EXPECT_NE((void *)NULL, (void *)ret_nested);
		metacall_value_destroy(ret_nested);

		/* Test variadic function from external script */
		void *ret_varargs = metacall("lua_mixed_sum_varargs", 1, 2, 3, 4, 5);
		EXPECT_NE((void *)NULL, (void *)ret_varargs);
		metacall_value_destroy(ret_varargs);
	}
#endif /* OPTION_BUILD_LOADERS_LUA */

	metacall_destroy();
}
