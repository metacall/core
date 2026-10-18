/*
*	Reflect Library by Parra Studios
*	A library for provide reflection and metadata representation.
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
#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_cast.h>

class reflect_value_cast_sign_extension_test : public testing::Test
{
public:
};

TEST_F(reflect_value_cast_sign_extension_test, char_to_short_negative)
{
	value v = value_create_char((char)-1);
	value dest = value_type_cast(v, TYPE_SHORT);
	EXPECT_EQ((short)-1, (short)value_to_short(dest));
	value_type_destroy(dest);
}

TEST_F(reflect_value_cast_sign_extension_test, short_to_int_negative)
{
	value v = value_create_short((short)-2);
	value dest = value_type_cast(v, TYPE_INT);
	EXPECT_EQ((int)-2, (int)value_to_int(dest));
	value_type_destroy(dest);
}

TEST_F(reflect_value_cast_sign_extension_test, int_to_long_negative)
{
	value v = value_create_int(-1);
	value dest = value_type_cast(v, TYPE_LONG);
	EXPECT_EQ((long)-1, (long)value_to_long(dest));
	value_type_destroy(dest);
}

TEST_F(reflect_value_cast_sign_extension_test, int_to_double_negative)
{
	value v = value_create_int(-5);
	value dest = value_type_cast(v, TYPE_DOUBLE);
	EXPECT_DOUBLE_EQ(-5.0, value_to_double(dest));
	value_type_destroy(dest);
}
