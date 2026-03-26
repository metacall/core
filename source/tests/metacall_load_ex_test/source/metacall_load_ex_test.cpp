/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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
#include <metacall/metacall_loaders.h>

class metacall_load_ex_test : public testing::Test
{
public:
};

TEST_F(metacall_load_ex_test, DefaultConstructor)
{
	metacall_print_info();

	metacall_log_stdio_type log_stdio = { stdout };

	ASSERT_EQ((int)0, (int)metacall_log(METACALL_LOG_STDIO, (void *)&log_stdio));

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Mock */
#if defined(OPTION_BUILD_LOADERS_MOCK)
	{
		/* Use an arbitrary user data value to verify the _ex API accepts it */
		int user_data = 0xBEEF;

		/* load_from_file_ex: with non-NULL data */
		{
			const char *scripts[] = { "empty.mock" };
			void *handle = NULL;

			EXPECT_EQ((int)0, (int)metacall_load_from_file_ex("mock", scripts, sizeof(scripts) / sizeof(scripts[0]), &handle, (void *)&user_data));
			EXPECT_NE((void *)NULL, (void *)handle);

			EXPECT_EQ((int)0, (int)metacall_clear(handle));
		}

		/* load_from_file_ex: with NULL data (backward compatibility) */
		{
			const char *scripts[] = { "empty.mock" };
			void *handle = NULL;

			EXPECT_EQ((int)0, (int)metacall_load_from_file_ex("mock", scripts, sizeof(scripts) / sizeof(scripts[0]), &handle, NULL));
			EXPECT_NE((void *)NULL, (void *)handle);

			EXPECT_EQ((int)0, (int)metacall_clear(handle));
		}

		/* load_from_memory_ex: with non-NULL data */
		{
			static const char buffer[] = "mock_memory_module";
			void *handle = NULL;

			EXPECT_EQ((int)0, (int)metacall_load_from_memory_ex("mock", buffer, sizeof(buffer), &handle, (void *)&user_data));
			EXPECT_NE((void *)NULL, (void *)handle);

			EXPECT_EQ((int)0, (int)metacall_clear(handle));
		}

		/* load_from_memory_ex: with NULL data (backward compatibility) */
		{
			static const char buffer[] = "mock_memory_module";
			void *handle = NULL;

			EXPECT_EQ((int)0, (int)metacall_load_from_memory_ex("mock", buffer, sizeof(buffer), &handle, NULL));
			EXPECT_NE((void *)NULL, (void *)handle);

			EXPECT_EQ((int)0, (int)metacall_clear(handle));
		}

		/* load_from_package_ex: with non-NULL data */
		{
			void *handle = NULL;

			EXPECT_EQ((int)0, (int)metacall_load_from_package_ex("mock", "empty.mock", &handle, (void *)&user_data));
			EXPECT_NE((void *)NULL, (void *)handle);

			EXPECT_EQ((int)0, (int)metacall_clear(handle));
		}

		/* load_from_package_ex: with NULL data (backward compatibility) */
		{
			void *handle = NULL;

			EXPECT_EQ((int)0, (int)metacall_load_from_package_ex("mock", "empty.mock", &handle, NULL));
			EXPECT_NE((void *)NULL, (void *)handle);

			EXPECT_EQ((int)0, (int)metacall_clear(handle));
		}
	}
#endif /* OPTION_BUILD_LOADERS_MOCK */

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		int user_data = 0xCAFE;

		static const char buffer[] =
			"#!/usr/bin/env python3\n"
			"def hello_ex(left, right):\n"
			"\treturn left + right\n";

		void *handle = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_memory_ex("py", buffer, sizeof(buffer), &handle, (void *)&user_data));
		EXPECT_NE((void *)NULL, (void *)handle);

		EXPECT_EQ((int)0, (int)metacall_clear(handle));
	}
#endif /* OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}
