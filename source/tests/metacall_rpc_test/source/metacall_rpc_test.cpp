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

class metacall_rpc_test : public testing::Test
{
public:
};

TEST_F(metacall_rpc_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* RPC */
#if defined(OPTION_BUILD_LOADERS_RPC)
	{
		const char *rpc_scripts[] = {
			"remote.url"
		};

		void *handle = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("rpc", rpc_scripts, sizeof(rpc_scripts) / sizeof(rpc_scripts[0]), &handle));

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

		const enum metacall_value_id divide_ids[] = {
			METACALL_FLOAT, METACALL_FLOAT
		};

		void *ret = metacallht_s(handle, "divide", divide_ids, 2, 50.0f, 10.0f);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((float)metacall_value_to_float(ret), (float)5.0f);

		metacall_value_destroy(ret);

		EXPECT_EQ((int)0, (int)metacall_clear(handle));

		static const char buffer[] = "http://localhost:6094/viferga/example/v1";

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("rpc", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_RPC */

	metacall_destroy();
}
