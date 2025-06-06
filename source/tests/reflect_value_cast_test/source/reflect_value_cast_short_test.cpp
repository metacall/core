/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_cast.h>

class reflect_value_cast_short_test : public testing::Test
{
public:
};

TEST_F(reflect_value_cast_short_test, short_to_bool_cast)
{
	value v = value_create_short(1923);

	boolean b = 1L; /* Because C compiler cast doesn't convert 1923 to 1L */

	v = value_type_cast(v, TYPE_BOOL);

	EXPECT_EQ((boolean)b, (boolean)value_to_bool(v));

	value_destroy(v);
}

TEST_F(reflect_value_cast_short_test, short_to_char_cast)
{
	value v = value_create_short(1923);

	short s = value_to_short(v);

	v = value_type_cast(v, TYPE_CHAR);

	EXPECT_EQ((char)s, (char)value_to_char(v));

	value_destroy(v);
}

TEST_F(reflect_value_cast_short_test, short_to_short_cast)
{
	value v = value_create_short(1923);

	short s = value_to_short(v);

	v = value_type_cast(v, TYPE_SHORT);

	EXPECT_EQ((short)s, (short)value_to_short(v));

	value_destroy(v);
}

TEST_F(reflect_value_cast_short_test, short_to_int_cast)
{
	value v = value_create_short(1923);

	short s = value_to_short(v);

	v = value_type_cast(v, TYPE_INT);

	EXPECT_EQ((int)s, (int)value_to_int(v));

	value_destroy(v);
}

TEST_F(reflect_value_cast_short_test, short_to_long_cast)
{
	value v = value_create_short(1923);

	short s = value_to_short(v);

	v = value_type_cast(v, TYPE_LONG);

	EXPECT_EQ((long)s, (long)value_to_long(v));

	value_destroy(v);
}

TEST_F(reflect_value_cast_short_test, short_to_float_cast)
{
	value v = value_create_short(1923);

	short s = value_to_short(v);

	v = value_type_cast(v, TYPE_FLOAT);

	EXPECT_EQ((float)s, (float)value_to_float(v));

	value_destroy(v);
}

TEST_F(reflect_value_cast_short_test, short_to_double_cast)
{
	value v = value_create_short(1923);

	short s = value_to_short(v);

	v = value_type_cast(v, TYPE_DOUBLE);

	EXPECT_EQ((double)s, (double)value_to_double(v));

	value_destroy(v);
}
