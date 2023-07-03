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

class metacall_logs_test : public testing::Test
{
public:
};

TEST_F(metacall_logs_test, DefaultConstructor)
{
	metacall_log_stdio_type log_stdio = { stdout };

	metacall_print_info();

	for (int i = 0; i < 50; ++i)
	{
		ASSERT_EQ((int)0, (int)metacall_log(METACALL_LOG_STDIO, (void *)&log_stdio));
	}

	ASSERT_EQ((int)0, (int)metacall_initialize());

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
