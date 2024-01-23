/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_rust_class_test : public testing::Test
{
protected:
};

TEST_F(metacall_rust_class_test, DefaultConstructor)
{
	const char *rs_scripts[] = {
		"class.rs"
	};

	ASSERT_EQ((int)0, (int)metacall_initialize());

	EXPECT_EQ((int)0, (int)metacall_load_from_file("rs", rs_scripts, sizeof(rs_scripts) / sizeof(rs_scripts[0]), NULL));
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
	{
		void *book_class = metacall_class("Book");
		ASSERT_NE((void *)NULL, (void *)book_class);

		void *ret_value = metacallv_class(book_class, "get_number", nullptr, 0);

		ASSERT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(ret_value));
		ASSERT_EQ((int)123, (int)metacall_value_to_int(ret_value));
		metacall_value_destroy(ret_value);
		// metacall_value_destroy(book_class);
	}
	{
		void *book_class = metacall_class("Book");
		ASSERT_NE((void *)NULL, (void *)book_class);

		void *constructor_params[] = {
			metacall_value_create_int(111) // param1
		};
		void *new_object_v = metacall_class_new(book_class, "book_one", constructor_params, sizeof(constructor_params) / sizeof(constructor_params[0]));
		metacall_value_destroy(constructor_params[0]);
		void *new_object = metacall_value_to_object(new_object_v);

		void *ret = metacallv_object(new_object, "get_price", nullptr, 0);
		ASSERT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(ret));
		ASSERT_EQ((int)111, (int)metacall_value_to_int(ret));

		void *param2 = metacall_object_get(new_object, "price");
		ASSERT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(param2));
		ASSERT_EQ((int)111, (int)metacall_value_to_int(param2));

		metacall_value_destroy(param2);

		void *int_value = metacall_value_create_int(100);
		int retcode = metacall_object_set(new_object, "price", int_value);
		metacall_value_destroy(int_value);
		ASSERT_EQ((int)0, int(retcode));

		param2 = metacall_object_get(new_object, "price");
		ASSERT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(param2));
		ASSERT_EQ((int)100, (int)metacall_value_to_int(param2));
		metacall_value_destroy(param2);

		metacall_value_destroy(new_object_v);
		metacall_value_destroy(ret);
		// metacall_value_destroy(myclass_value);
		// metacall_value_destroy(book_class);
	}

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
