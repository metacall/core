/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>
#include <metacall/metacall_value.h>

#ifndef METACALL_PYTHON_REENTRANT_TEST_PY_PORT_PATH
	#error "The path to the Python port is not defined"
#endif

class metacall_python_reentrant_test : public testing::Test
{
public:
};

TEST_F(metacall_python_reentrant_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_REENTRANT_TEST_PY_PORT_PATH "');\n"
			"from metacall import metacall_load_from_memory, metacall\n"
			"metacall_load_from_memory('py', 'def python_memory(): return 4;');\n"
			"print('Reentrant python_memory result:', metacall('python_memory'));\n";

		static const char tag[] = "py";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));
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

	metacall_destroy();
}
