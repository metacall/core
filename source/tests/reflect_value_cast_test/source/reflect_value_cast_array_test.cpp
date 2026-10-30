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

class reflect_value_cast_array_test : public testing::Test
{
public:
};

/* Casting a single value to an array must not free the value it just
 * wrapped -- the array's only element aliases the same allocation, so
 * destroying it here leaves the array holding a dangling pointer. */
TEST_F(reflect_value_cast_array_test, single_to_array_cast)
{
	value v = value_create_int(42);
	value arr = value_type_cast(v, TYPE_ARRAY);

	ASSERT_NE((value)NULL, arr);

	value *elements = value_to_array(arr);

	EXPECT_EQ((int)42, (int)value_to_int(elements[0]));

	value_type_destroy(arr);
}

/* Casting a 1-element array back to a single value of the SAME type
 * must not free the element it is about to return -- value_type_cast
 * destroys the array container, whose recursive destroy would also
 * free the element being handed back to the caller. */
TEST_F(reflect_value_cast_array_test, array_to_single_same_type_cast)
{
	value inner = value_create_int(7);
	value arr = value_type_create(&inner, sizeof(value), TYPE_ARRAY);

	ASSERT_NE((value)NULL, arr);

	value dest = value_type_cast(arr, TYPE_INT);

	EXPECT_EQ((int)7, (int)value_to_int(dest));

	value_type_destroy(dest);
}

/* Casting a 1-element array back to a single value of a DIFFERENT type
 * must not double-free the element -- the recursive promotion/demotion
 * cast already consumes it, so the array's own destroy must not touch
 * it again. */
TEST_F(reflect_value_cast_array_test, array_to_single_promoted_type_cast)
{
	value inner = value_create_int(7);
	value arr = value_type_create(&inner, sizeof(value), TYPE_ARRAY);

	ASSERT_NE((value)NULL, arr);

	value dest = value_type_cast(arr, TYPE_LONG);

	EXPECT_EQ((long)7, (long)value_to_long(dest));

	value_type_destroy(dest);
}
