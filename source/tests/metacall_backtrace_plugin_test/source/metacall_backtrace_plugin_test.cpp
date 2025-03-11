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

#include <stdlib.h>
#include <string.h>

#include <iostream>

void badass_function(void)
{
#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Warray-bounds"
#elif defined(__GNUC__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Warray-bounds"
	#pragma GCC diagnostic ignored "-Wstringop-overflow="
#endif

	char *ptr = (char *)42;
	*ptr = 42;

#if defined(__clang__)
	#pragma clang diagnostic pop
#elif defined(__GNUC__)
	#pragma GCC diagnostic pop
#endif
}

class metacall_backtrace_plugin_test : public testing::Test
{
protected:
};

TEST_F(metacall_backtrace_plugin_test, DefaultConstructor)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* Generate a segmentation fault in order to catch it by backtrace plugin */
	EXPECT_DEATH({ badass_function(); }, "");

	metacall_destroy();
}
