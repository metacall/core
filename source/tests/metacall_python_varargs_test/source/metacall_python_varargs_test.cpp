/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

class metacall_python_varargs_test : public testing::Test
{
public:
};

TEST_F(metacall_python_varargs_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char python_script[] =
			"#!/usr/bin/env python3\n"
			"values = [10, 20, 30]\n"
			"def varargs(*args):\n"
			"	for (v, a) in zip(values, args):\n"
			"		print(v, ' == ', a);\n"
			"		if v != a:\n"
			"			return 324;\n"
			"	return 20;\n";

		EXPECT_EQ((int) 0, (int) metacall_load_from_memory("py", python_script, sizeof(python_script), NULL));

		void * args2[] =
		{
			metacall_value_create_long(10),
			metacall_value_create_long(20)
		};

		void * args3[] =
		{
			metacall_value_create_long(10),
			metacall_value_create_long(20),
			metacall_value_create_long(30)
		};

		void * ret = metacallv_s("varargs", args2, 2);

		ASSERT_EQ((long) 20, (long) metacall_value_to_long(ret));

		metacall_value_destroy(ret);

		ret = metacallv_s("varargs", args3, 3);

		ASSERT_EQ((long) 20, (long) metacall_value_to_long(ret));

		metacall_value_destroy(ret);

		metacall_value_destroy(args2[0]);
		metacall_value_destroy(args2[1]);

		metacall_value_destroy(args3[0]);
		metacall_value_destroy(args3[1]);
		metacall_value_destroy(args3[2]);
	}
	#endif /* OPTION_BUILD_LOADERS_PY */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void * allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char * inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *) NULL, (char *) inspect_str);

		EXPECT_GT((size_t) size, (size_t) 0);

		std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
