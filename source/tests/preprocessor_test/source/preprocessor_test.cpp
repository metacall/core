/*
 *	Preprocssor Library by Parra Studios
 *	A generic header-only preprocessor metaprogramming library.
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

#include <preprocessor/preprocessor.h>

class preprocessor_test : public testing::Test
{
public:
};

TEST_F(preprocessor_test, arguments_comma)
{
	EXPECT_EQ((int)1, (int)PREPROCESSOR_ARGS_COMMA(1, 2, 3, 4, 5));

	EXPECT_EQ((int)1, (int)PREPROCESSOR_ARGS_COMMA(1, 2));

	EXPECT_EQ((int)0, (int)PREPROCESSOR_ARGS_COMMA(1));

	EXPECT_EQ((int)0, (int)PREPROCESSOR_ARGS_COMMA());
}

TEST_F(preprocessor_test, arguments)
{
	size_t first_remove[] = { PREPROCESSOR_ARGS_FIRST_REMOVE(1, 2, 3, 4) };

	size_t second_remove[] = { PREPROCESSOR_ARGS_SECOND_REMOVE(1, 2, 3, 4) };

	EXPECT_EQ((int)1, (int)PREPROCESSOR_ARGS_FIRST(1, 2, 3, 4, 5, 6));

	EXPECT_EQ((size_t)first_remove[0], (size_t)2);

	EXPECT_EQ((size_t)first_remove[1], (size_t)3);

	EXPECT_EQ((size_t)first_remove[2], (size_t)4);

	EXPECT_EQ((int)2, (int)PREPROCESSOR_ARGS_SECOND(1, 2, 3, 4, 5, 6));

	EXPECT_EQ((size_t)second_remove[0], (size_t)1);

	EXPECT_EQ((size_t)second_remove[1], (size_t)3);

	EXPECT_EQ((size_t)second_remove[2], (size_t)4);

	EXPECT_EQ((int)0, (int)PREPROCESSOR_ARGS_COUNT());

	EXPECT_EQ((int)1, (int)PREPROCESSOR_ARGS_COUNT(a));

	EXPECT_EQ((int)2, (int)PREPROCESSOR_ARGS_COUNT(a, b));

	EXPECT_EQ((int)3, (int)PREPROCESSOR_ARGS_COUNT(a, b, c));

	EXPECT_EQ((int)4, (int)PREPROCESSOR_ARGS_COUNT(a, b, c, d));

	EXPECT_EQ((int)0, (int)PREPROCESSOR_ARGS_COUNT(/* ... */));
}

TEST_F(preprocessor_test, bit)
{
	/* TODO */
}

TEST_F(preprocessor_test, boolean)
{
	/* TODO */
}

TEST_F(preprocessor_test, comma)
{
	/* TODO */
}

TEST_F(preprocessor_test, comparison)
{
	/* TODO */
}

TEST_F(preprocessor_test, complement)
{
	/* TODO */
}

TEST_F(preprocessor_test, concatenation)
{
	/* TODO */
}

TEST_F(preprocessor_test, detection)
{
	/* TODO */
}

TEST_F(preprocessor_test, empty)
{
	const char empty_str[] = "";

	EXPECT_STREQ("", PREPROCESSOR_STRINGIFY_OR_EMPTY(PREPROCESSOR_EMPTY_SYMBOL()));
	EXPECT_STREQ(empty_str, PREPROCESSOR_STRINGIFY_OR_EMPTY(PREPROCESSOR_EMPTY_SYMBOL()));

	PREPROCESSOR_EMPTY_EXPANSION(this must compile)

	PREPROCESSOR_EMPTY_EXPANSION_VARIADIC(this, must, compile)
}

TEST_F(preprocessor_test, foreach)
{
#define PREPROCESSOR_TEST_FOR_EACH_STR_SIZE 0x04

	char for_each_str[PREPROCESSOR_TEST_FOR_EACH_STR_SIZE];

	memset(for_each_str, '\0', sizeof(char) * PREPROCESSOR_TEST_FOR_EACH_STR_SIZE);

#define PREPROCESSOR_TEST_FOR_EACH(expr) strncat(for_each_str, expr, PREPROCESSOR_TEST_FOR_EACH_STR_SIZE - 1);

	PREPROCESSOR_FOR_EACH(PREPROCESSOR_TEST_FOR_EACH, "a", "b", "c")

#undef PREPROCESSOR_TEST_FOR_EACH

#undef PREPROCESSOR_TEST_FOR_EACH_STR_SIZE

	EXPECT_STREQ(for_each_str, "abc");
}

TEST_F(preprocessor_test, foreach_empty)
{
#define PREPROCESSOR_TEST_FOR_EACH(expr) "33434"

	EXPECT_EQ((int)1, (int)PREPROCESSOR_ARGS_EMPTY(PREPROCESSOR_FOR_EACH(PREPROCESSOR_TEST_FOR_EACH)));

#undef PREPROCESSOR_TEST_FOR_EACH
}

TEST_F(preprocessor_test, for_empty)
{
#define PREPROCESSOR_TEST_FOR(context, iterator, element) "33434"

	EXPECT_EQ((int)1, (int)PREPROCESSOR_ARGS_EMPTY(PREPROCESSOR_FOR(PREPROCESSOR_TEST_FOR, "yeet")));

#undef PREPROCESSOR_TEST_FOR
}

TEST_F(preprocessor_test, for)
{
#define PREPROCESSOR_TEST_FOR(context, iterator, element) \
	context \
		PREPROCESSOR_STRINGIFY(iterator) \
			element \
		"-"

	const char for_str[] = PREPROCESSOR_FOR(PREPROCESSOR_TEST_FOR, "yeet", "a", "b", "c", "d", "e", "f");

#undef PREPROCESSOR_TEST_FOR

	EXPECT_STREQ(for_str, "yeet0a-yeet1b-yeet2c-yeet3d-yeet4e-yeet5f-");
}

TEST_F(preprocessor_test, if)
{
	EXPECT_EQ((int)0, PREPROCESSOR_IF(1, (int)0, (int)1));

	EXPECT_NE((int)0, PREPROCESSOR_IF(0, (int)0, (int)1));
}

TEST_F(preprocessor_test, if_va_args)
{
#define PREPROCESSOR_TEST_IF_VA_ARGS(A, B, C, ...) \
	PREPROCESSOR_IF(PREPROCESSOR_ARGS_EMPTY(__VA_ARGS__), 1, 0)

	EXPECT_EQ((int)1, (int)PREPROCESSOR_TEST_IF_VA_ARGS(A, B, C));
	EXPECT_EQ((int)0, (int)PREPROCESSOR_TEST_IF_VA_ARGS(A, B, C, D));
	EXPECT_EQ((int)0, (int)PREPROCESSOR_TEST_IF_VA_ARGS(A, B, C, D, E));
	EXPECT_EQ((int)0, (int)PREPROCESSOR_TEST_IF_VA_ARGS(A, B, C, D, E, F));

#undef PREPROCESSOR_TEST_IF_VA_ARGS
}

TEST_F(preprocessor_test, if_va_args_ext)
{
#define PREPROCESSOR_TEST_IF_VA_ARGS_PRINT(A, B, C) \
	do \
	{ \
		printf("%d %d %d\n", A, B, C); \
	} while (0)

#define PREPROCESSOR_TEST_IF_VA_ARGS_PRINT_VA(A, B, C, ...) \
	do \
	{ \
		printf(A, B, C, __VA_ARGS__); \
	} while (0)

#define PREPROCESSOR_TEST_IF_VA_ARGS(A, B, C, ...) \
	PREPROCESSOR_IF(PREPROCESSOR_ARGS_EMPTY(__VA_ARGS__), \
		PREPROCESSOR_TEST_IF_VA_ARGS_PRINT(A, B, C), \
		PREPROCESSOR_TEST_IF_VA_ARGS_PRINT_VA(A, B, C, __VA_ARGS__))

	PREPROCESSOR_TEST_IF_VA_ARGS(1, 2, 3);
	PREPROCESSOR_TEST_IF_VA_ARGS("%s %s %s\n", "B", "C", "D");
	PREPROCESSOR_TEST_IF_VA_ARGS("%s %s %s %s\n", "B", "C", "D", "E");
	PREPROCESSOR_TEST_IF_VA_ARGS("%s %s %s %s %s\n", "B", "C", "D", "E", "F");

#undef PREPROCESSOR_TEST_IF_VA_ARGS
}

TEST_F(preprocessor_test, serial)
{
#define PREPROCSSOR_TEST_SERIAL_TAG abc

	const char serial_id_a[] = PREPROCESSOR_STRINGIFY(PREPROCESSOR_SERIAL_ID(PREPROCESSOR_TEST_SERIAL_TAG));

	const char serial_id_b[] = PREPROCESSOR_STRINGIFY(PREPROCESSOR_SERIAL_ID(PREPROCESSOR_TEST_SERIAL_TAG));

	EXPECT_STRNE(serial_id_a, serial_id_b);

#undef PREPROCESSOR_TEST_SERIAL_TAG
}

TEST_F(preprocessor_test, stringify)
{
#define PREPROCESSOR_TEST_STRINGIFY_TAG abc

	const char stringify_tag[] = "abc";

	EXPECT_STREQ(stringify_tag, PREPROCESSOR_STRINGIFY(PREPROCESSOR_TEST_STRINGIFY_TAG));

	EXPECT_STREQ(stringify_tag, PREPROCESSOR_STRINGIFY_VARIADIC(a, b, c));

#undef PREPROCESSOR_TEST_STRINGIFY_TAG
}

TEST_F(preprocessor_test, tuple)
{
	EXPECT_EQ((int)0, PREPROCESSOR_TUPLE_EXPAND(PREPROCESSOR_TUPLE_MAKE((int)0)));

	PREPROCESSOR_TUPLE_EXPAND_MACRO(EXPECT_EQ, PREPROCESSOR_TUPLE_PREPEND((int)1, PREPROCESSOR_TUPLE_MAKE((int)1)));

	PREPROCESSOR_TUPLE_EXPAND_MACRO(EXPECT_NE, PREPROCESSOR_TUPLE_APPEND((int)1, PREPROCESSOR_TUPLE_MAKE((int)0)));
}

TEST_F(preprocessor_test, if_with_arguments_count)
{
	EXPECT_EQ((int)1, (int)PREPROCESSOR_IF(PREPROCESSOR_ARGS_COUNT(a, b, c), 1, 0));
}
