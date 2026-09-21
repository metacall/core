/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
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

#include <adt/adt_vector.h>

class adt_vector_test : public testing::Test
{
public:
};

TEST_F(adt_vector_test, DefaultConstructor)
{
	static const size_t capacity = 50;

	vector v = vector_create_reserve_type(size_t, capacity);

	ASSERT_EQ((size_t)vector_type_size(v), (size_t)sizeof(size_t));
	ASSERT_EQ((size_t)vector_size(v), (size_t)0);

	for (size_t i = 0; i < capacity; ++i)
	{
		vector_push_back_var(v, i);
	}

	ASSERT_EQ((size_t)vector_size(v), (size_t)capacity);

	for (size_t i = 0; i < capacity; ++i)
	{
		size_t current = vector_at_type(v, i, size_t);

		ASSERT_EQ((size_t)current, (size_t)i);
	}

	vector_destroy(v);
}

static const size_t insert_empty_count = 10;

static vector insert_empty_create_sequence()
{
	vector v = vector_create_type(size_t);

	for (size_t i = 0; i < insert_empty_count; ++i)
	{
		vector_push_back_var(v, i);
	}

	return v;
}

static void insert_empty_set(vector v, size_t position, size_t value)
{
	memcpy(vector_at(v, position), &value, sizeof(size_t));
}

TEST_F(adt_vector_test, InsertEmptyAtFront)
{
	// 0 1 2 3 4 5 6 7 8 9 -> insert at 0 -> X 0 1 2 3 4 5 6 7 8 9
	vector v = insert_empty_create_sequence();

	vector_insert_empty(v, 0);

	ASSERT_EQ((size_t)vector_size(v), (size_t)(insert_empty_count + 1));

	insert_empty_set(v, 0, 100);

	ASSERT_EQ((size_t)vector_at_type(v, 0, size_t), (size_t)100);

	for (size_t i = 0; i < insert_empty_count; ++i)
	{
		ASSERT_EQ((size_t)vector_at_type(v, i + 1, size_t), (size_t)i);
	}

	vector_destroy(v);
}

TEST_F(adt_vector_test, InsertEmptyAtMiddle)
{
	// 0 1 2 3 4 5 6 7 8 9 -> insert at 5 -> 0 1 2 3 4 X 5 6 7 8 9
	static const size_t position = 5;

	vector v = insert_empty_create_sequence();

	vector_insert_empty(v, position);

	ASSERT_EQ((size_t)vector_size(v), (size_t)(insert_empty_count + 1));

	insert_empty_set(v, position, 100);

	for (size_t i = 0; i < position; ++i)
	{
		ASSERT_EQ((size_t)vector_at_type(v, i, size_t), (size_t)i);
	}

	ASSERT_EQ((size_t)vector_at_type(v, position, size_t), (size_t)100);

	for (size_t i = position; i < insert_empty_count; ++i)
	{
		ASSERT_EQ((size_t)vector_at_type(v, i + 1, size_t), (size_t)i);
	}

	vector_destroy(v);
}

TEST_F(adt_vector_test, InsertEmptyAtEnd)
{
	// 0 1 2 3 4 5 6 7 8 9 -> insert at 10 -> 0 1 2 3 4 5 6 7 8 9 (uninitialized slot)
	vector v = insert_empty_create_sequence();

	vector_insert_empty(v, insert_empty_count);

	ASSERT_EQ((size_t)vector_size(v), (size_t)(insert_empty_count + 1));

	for (size_t i = 0; i < insert_empty_count; ++i)
	{
		ASSERT_EQ((size_t)vector_at_type(v, i, size_t), (size_t)i);
	}

	vector_destroy(v);
}
