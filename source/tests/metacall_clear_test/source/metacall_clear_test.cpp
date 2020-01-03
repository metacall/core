/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

class metacall_clear_test : public testing::Test
{
public:
};

TEST_F(metacall_clear_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			"#!/usr/bin/env python3\n"
			"def multmem(left: int, right: int) -> int:\n"
			"\tresult = left * right;\n"
			"\tprint(left, ' * ', right, ' = ', result);\n"
			"\treturn result;";

		static const char tag[] = "py";

		void * handle = NULL;

		ASSERT_EQ((int) 0, (int) metacall_load_from_memory(tag, buffer, sizeof(buffer), &handle));

		void * ret = metacall("multmem", 5, 15);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) 75, (long) metacall_value_to_long(ret));

		metacall_value_destroy(ret);

		EXPECT_EQ((int) 0, (int) metacall_clear(handle));

		EXPECT_EQ((void *) NULL, (void *) metacall_function("multmem"));
	}
	#endif /* OPTION_BUILD_LOADERS_PY */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
