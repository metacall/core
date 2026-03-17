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

class metacall_version_test : public testing::Test
{
public:
};

TEST_F(metacall_version_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_STREQ(METACALL_VERSION, metacall_version_str());

	// Test metacall_version() struct
	const struct metacall_version_type *v = metacall_version();
	ASSERT_NE((const struct metacall_version_type *)NULL, v);
	ASSERT_NE((const char *)NULL, v->str);
	ASSERT_NE((const char *)NULL, v->name);
	ASSERT_NE((const char *)NULL, v->revision);
	ASSERT_STREQ(METACALL_VERSION, v->str);

	// Test hex: round-trip make vs get
	uint32_t hex_direct = metacall_version_hex();
	uint32_t hex_made = metacall_version_hex_make(v->major, v->minor, v->patch);
	ASSERT_EQ(hex_direct, hex_made);
	ASSERT_GT(hex_direct, (uint32_t)0);

	// Test string helpers are consistent with struct
	ASSERT_STREQ(v->str, metacall_version_str());
	ASSERT_STREQ(v->name, metacall_version_name());
	ASSERT_STREQ(v->revision, metacall_version_revision());
}
