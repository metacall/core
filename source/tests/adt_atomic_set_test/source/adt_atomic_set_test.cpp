/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
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

#include <adt/adt_atomic_set.h>

#include <log/log.h>

class adt_atomic_set_test : public testing::Test
{
public:
};

TEST_F(adt_atomic_set_test, DefaultConstructor)
{
	EXPECT_EQ((int)0, (int)log_configure("metacall",
						  log_policy_format_text(),
						  log_policy_schedule_sync(),
						  log_policy_storage_sequential(),
						  log_policy_stream_stdio(stdout)));

	{
		const size_t size = 16;
		atomic_set s = atomic_set_create(size);

		ASSERT_NE((atomic_set)s, (atomic_set)NULL);

		atomic_set_insert(s, "hello", (void *)0x10);
		atomic_set_insert(s, "bye", (void *)0x20);

		EXPECT_EQ((void *)atomic_set_get(s, "hello"), (void *)0x10);
		EXPECT_EQ((void *)atomic_set_get(s, "bye"), (void *)0x20);

		atomic_set_destroy(s);
	}
}
