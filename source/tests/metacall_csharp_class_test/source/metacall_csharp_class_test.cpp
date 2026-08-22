/*
 * MetaCall Library by Parra Studios
 * A library for providing a foreign function interface calls.
 *
 * Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <gtest/gtest.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>
#include <metacall/metacall_value.h>

#include <cstdlib>
#include <iostream>

class metacall_csharp_class_test : public testing::Test
{
public:
};

TEST_F(metacall_csharp_class_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* C# Netcore */
#if defined(OPTION_BUILD_LOADERS_CS)
	{
		const char* cs_scripts[] = {
			"class.cs"
		};

		ASSERT_EQ((int)0, (int)metacall_load_from_file("cs", cs_scripts, sizeof(cs_scripts) / sizeof(cs_scripts[0]), NULL));

		/* Get the class handle */
		void* myclass = metacall_class("Counter");
		ASSERT_NE((void*)NULL, (void*)myclass);

		/* Construct: new Counter(5) */
		void* constructor_params[] = {
			metacall_value_create_int(5)
		};

		void* new_object_v = metacall_class_new(myclass, "Counter", constructor_params, sizeof(constructor_params) / sizeof(constructor_params[0]));
		ASSERT_NE((void*)NULL, (void*)new_object_v);

		void* new_object = metacall_value_to_object(new_object_v);

		/* Read the instance attribute: count == 5 */
		{
			void* count = metacall_object_get(new_object, "count");
			EXPECT_EQ((int)5, (int)metacall_value_to_int(count));
			metacall_value_destroy(count);
		}

		/* Invoke the instance method: Add(7) == 12 */
		{
			void* args[] = {
				metacall_value_create_int(7)
			};

			void* ret = metacallt_object(new_object, "Add", METACALL_INT, args, sizeof(args) / sizeof(args[0]));
			EXPECT_EQ((int)12, (int)metacall_value_to_int(ret));
			metacall_value_destroy(ret);
		}

		/* Invoke the static method: Counter.Twice(21) == 42 */
		{
			void* args[] = {
				metacall_value_create_int(21)
			};

			void* ret = metacallt_class(myclass, "Twice", METACALL_INT, args, sizeof(args) / sizeof(args[0]));
			EXPECT_EQ((int)42, (int)metacall_value_to_int(ret));
			metacall_value_destroy(ret);
		}

		/* Static attribute: Counter.total set/get == 99 */
		{
			void* val = metacall_value_create_int(99);
			ASSERT_EQ((int)0, (int)metacall_class_static_set(myclass, "total", val));
			metacall_value_destroy(val);

			void* ret = metacall_class_static_get(myclass, "total");
			EXPECT_EQ((int)99, (int)metacall_value_to_int(ret));
			metacall_value_destroy(ret);
		}

		metacall_value_destroy(new_object_v);
	}
#endif /* OPTION_BUILD_LOADERS_CS */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void* allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void*)&std_ctx);

		char* inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char*)NULL, (char*)inspect_str);

		EXPECT_GT((size_t)size, (size_t)0);

		std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	metacall_destroy();
}
