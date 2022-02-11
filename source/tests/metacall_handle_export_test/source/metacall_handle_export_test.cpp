/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
#include <metacall/metacall_loaders.h>

class metacall_handle_export_test : public testing::Test
{
public:
};

TEST_F(metacall_handle_export_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char *py_scripts[] = {
			"example.py"
		};

		void *v, *handle = NULL;

		char *value_str = NULL;

		size_t size = 0;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), &handle));

		ASSERT_NE((void *)NULL, (void *)handle);

		v = metacall_handle_export(handle);

		EXPECT_NE((void *)NULL, (void *)v);

		value_str = metacall_serialize(metacall_serial(), v, &size, allocator);

		EXPECT_NE((char *)NULL, (char *)value_str);

		EXPECT_GT((size_t)size, (size_t)0);

		std::cout << value_str << std::endl;

		metacall_value_destroy(v);

		metacall_allocator_free(allocator, value_str);
	}
#endif /* OPTION_BUILD_LOADERS_PY */

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char *node_scripts[] = {
			"nod.js"
		};

		void *v, *handle = NULL;

		char *value_str = NULL;

		size_t size = 0;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("node", node_scripts, sizeof(node_scripts) / sizeof(node_scripts[0]), &handle));

		ASSERT_NE((void *)NULL, (void *)handle);

		v = metacall_handle_export(handle);

		EXPECT_NE((void *)NULL, (void *)v);

		value_str = metacall_serialize(metacall_serial(), v, &size, allocator);

		EXPECT_NE((char *)NULL, (char *)value_str);

		EXPECT_GT((size_t)size, (size_t)0);

		std::cout << value_str << std::endl;

		metacall_value_destroy(v);

		metacall_allocator_free(allocator, value_str);
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	/* Test bad handle allocation */
	{
		struct
		{
			int random;
			int broken;
			int handle;
			int yeet;
			char padding[600];
		} broken_handle = { 0, 0, 0, 0, { 0 } };

		EXPECT_EQ((void *)NULL, (void *)metacall_handle_export((void *)&broken_handle));
	}

	/* Print inspect information */
	{
		size_t size = 0;

		char *inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *)NULL, (char *)inspect_str);

		EXPECT_GT((size_t)size, (size_t)0);

		std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);
	}

	metacall_allocator_destroy(allocator);

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
