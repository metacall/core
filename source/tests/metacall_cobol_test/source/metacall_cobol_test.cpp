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
#include <metacall/metacall_value.h>

class metacall_cobol_test : public testing::Test
{
public:
};

TEST_F(metacall_cobol_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Cobol */
#if defined(OPTION_BUILD_LOADERS_COB)
	{
		const char *cob_scripts[] = {
			"say.cob"
		};

		const enum metacall_value_id hello_string_ids[] = {
			METACALL_STRING, METACALL_STRING
		};

		static const char tag[] = "cob";

		void *ret = NULL;

		ASSERT_EQ((int)0, (int)metacall_load_from_file(tag, cob_scripts, sizeof(cob_scripts) / sizeof(cob_scripts[0]), NULL));

		ret = metacallt_s("say", hello_string_ids, 2, "hello", "world");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

		metacall_value_destroy(ret);

		/* This is a Python script on purpose, in order to test Cobol when it fails */
		static const char buffer[] =
			"#!/usr/bin/env python3\n"
			"def multmem(left: int, right: int) -> int:\n"
			"\tresult = left * right;\n"
			"\tprint(left, ' * ', right, ' = ', result);\n"
			"\treturn result;";

		EXPECT_EQ((int)1, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_COB */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char *inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *)NULL, (char *)inspect_str);

		EXPECT_GT((size_t)size, (size_t)0);

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
