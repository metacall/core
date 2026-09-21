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
#include <string>

class metacall_path_overflow_test : public testing::Test
{
public:
};

TEST_F(metacall_path_overflow_test, DefaultConstructor)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* 
	 * LOADER_PATH_SIZE == PORTABILITY_PATH_SIZE == PATH_MAX (4096).
	 * strncpy() in metacall_load_from_file_ex() does not null-terminate
	 * when the source is >= LOADER_PATH_SIZE, causing a heap-buffer-overflow
	 * read in loader_impl_handle_name(). This must be rejected cleanly.
	 */
	std::string too_long_path(5000, 'A');
	const char *scripts[] = {
		too_long_path.c_str()
	};
	const size_t size = sizeof(scripts) / sizeof(scripts[0]);

	EXPECT_NE((int)0, (int)metacall_load_from_file("mock", scripts, size, NULL));

	metacall_destroy();
}
