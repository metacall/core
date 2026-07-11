/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
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

#include <set>
#include <string>

class metacall_c_conversion_test : public testing::Test
{
protected:
};

TEST_F(metacall_c_conversion_test, DefaultConstructor)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	std::set<std::pair<std::string, size_t>> unique_types;

	// Load the types
	{
		metacall_loader_type *c_loader_types = NULL;
		size_t size = 0;

		ASSERT_EQ((int)0, (int)metacall_loader_types("c", &c_loader_types, &size));

		for (size_t i = 0; i < size; ++i)
		{
			std::cout
				<< "name: " << c_loader_types[i].name
				<< ", id: " << static_cast<int>(c_loader_types[i].id)
				<< std::endl;

			auto metacall_type_name = std::string(metacall_value_id_name((enum metacall_value_id)c_loader_types[i].id));

			unique_types.insert(std::make_pair(metacall_type_name, c_loader_types[i].id));
		}
	}

	// Run identity type
	{
		const char c_buffer[] = {
			"#include <stdbool.h>\n"
			"bool identity_Boolean(bool a) { return a; }\n"
			"char identity_Char(char a) { return a; }\n"
			"short identity_Short(short a) { return a; }\n"
			"int identity_Int(int a) { return a; }\n"
			"long identity_Long(long a) { return a; }\n"
			"float identity_Float(float a) { return a; }\n"
			"double identity_Double(double a) { return a; }\n"
			"const char *identity_String(const char *a) { return a; }\n"
			"char *identity_Buffer(char *a) { return a; }\n"
			"int *identity_Array(int *a) { return a; }\n"
			"void *identity_Pointer(void *a) { return a; }\n"
			"typedef int (*func_t)(int);\n"
			"func_t identity_Function(func_t a) { return a; }\n"
			"void identity_Null(void) { }\n"
		};

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("c", c_buffer, sizeof(c_buffer), NULL));

		// TODO:
		// 1) Test with cmake -DOPTION_BUILD_ADDRESS_SANITIZER=on -DOPTION_BUILD_THREAD_SANITIZER=off -DOPTION_BUILD_LOADERS_C=on ..
		// 2) Solve the leaks that appear form sanitizer

		for (const auto &[name, id] : unique_types)
		{
			std::string func_name = std::string("identity_") + name;
			void *f = metacall_function(func_name.c_str());

			void *args[1] = { metacall_value_create((enum metacall_value_id)id) };
			void *ret = metacallfv(f, args);

			ASSERT_NE(ret, nullptr);
			ASSERT_EQ(metacall_value_id(ret), (enum metacall_value_id)id);

			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}
	}

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
