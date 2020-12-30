/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <gmock/gmock.h>

#include <adt/adt_vector.h>

class adt_vector_test : public testing::Test
{
  public:
};

TEST_F(adt_vector_test, DefaultConstructor)
{
	static const size_t capacity = 50;

	vector v = vector_create_reserve_type(size_t, capacity);

	ASSERT_EQ((size_t) vector_type_size(v), (size_t) sizeof(size_t));
	ASSERT_EQ((size_t) vector_size(v), (size_t) 0);

	for (size_t i = 0; i < capacity; ++i)
	{
		vector_push_back_var(v, i);
	}

	ASSERT_EQ((size_t) vector_size(v), (size_t) capacity);

	for (size_t i = 0; i < capacity; ++i)
	{
		size_t current = vector_at_type(v, i, size_t);

		ASSERT_EQ((size_t) current, (size_t) i);
	}

	vector_destroy(v);
}
