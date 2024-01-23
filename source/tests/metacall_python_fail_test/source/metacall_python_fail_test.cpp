/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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
#include <metacall/metacall_loaders.h>

class metacall_python_fail_test : public testing::Test
{
public:
};

TEST_F(metacall_python_fail_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char buffer[] =
			"def sumList(list: [int]):\n"
			"	return sum(list)\n";

		// Apparently this loads the function but it does not understand the type
		EXPECT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));

		enum metacall_value_id id;

		EXPECT_EQ((int)0, (int)metacall_function_parameter_type(metacall_function("sumList"), 0, &id));

		// The type of list must be invalid once it loads
		EXPECT_EQ((enum metacall_value_id)METACALL_INVALID, (enum metacall_value_id)id);

		const char *py_scripts[] = {
			"this_does_not_exists_yeet.py"
		};

		EXPECT_EQ((int)1, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		const char buffer_fail[] =
			"def sdf: as asf return";

		// This must fail
		EXPECT_EQ((int)1, (int)metacall_load_from_memory("py", buffer_fail, sizeof(buffer_fail), NULL));

		const char *py_scripts_non_installed[] = {
			"badimport.py"
		};

		// Print traceback
		EXPECT_EQ((int)1, (int)metacall_load_from_file("py", py_scripts_non_installed, sizeof(py_scripts_non_installed) / sizeof(py_scripts_non_installed[0]), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_PY */

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

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
