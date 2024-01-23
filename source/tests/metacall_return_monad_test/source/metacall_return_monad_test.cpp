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

void *c_function(void *args[])
{
	printf("%s\n", (char *)args[0]);

	return metacall_value_create_int(1);
}

class metacall_return_monad_test : public testing::Test
{
public:
};

TEST_F(metacall_return_monad_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_stdio_type log_stdio = { stdout };

	ASSERT_EQ((int)0, (int)metacall_log(METACALL_LOG_STDIO, (void *)&log_stdio));

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			"#!/usr/bin/env python3\n"
			"def monad(value: int):\n"
			"\tresult = 'asd';\n"
			"\tif value > 0:\n"
			"\t\treturn result;\n"
			"\telse:\n"
			"\t\treturn bytes(result, 'utf-8');\n";

		static const char tag[] = "py";

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		void *ret = NULL;

		size_t size = 0;

		char *value_str = NULL;

		ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

		ret = metacall("monad", 1);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_STRING, (enum metacall_value_id)metacall_value_id(ret));

		EXPECT_EQ((int)0, (int)strcmp("asd", metacall_value_to_string(ret)));

		value_str = metacall_serialize(metacall_serial(), ret, &size, allocator);

		std::cout << value_str << std::endl;

		metacall_allocator_free(allocator, value_str);

		metacall_value_destroy(ret);

		ret = metacall("monad", 0);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_BUFFER, (enum metacall_value_id)metacall_value_id(ret));

		EXPECT_EQ((int)0, (int)memcmp(metacall_value_to_buffer(ret), "asd", metacall_value_size(ret)));

		value_str = metacall_serialize(metacall_serial(), ret, &size, allocator);

		std::cout << value_str << std::endl;

		metacall_allocator_free(allocator, value_str);

		metacall_value_destroy(ret);

		metacall_allocator_destroy(allocator);
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
