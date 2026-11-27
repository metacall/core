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

#include <adt/adt_hash.h>

#include <cstdint>

namespace
{

/* Canonical djb2 reference: h = 0x1505; while ((c = *s++)) h = ((h << 5) + h) + c; */
uintptr_t djb2_ref(const char *s)
{
	uintptr_t h = 0x1505;

	for (; *s != '\0'; ++s)
	{
		h = ((h << 5) + h) + (uintptr_t)(unsigned char)*s;
	}

	return h;
}

} /* namespace */

/* Regression test: hash_callback_str (source/adt/source/adt_hash.c) must implement
 * canonical djb2. The previous loop tested+advanced (*str++ in the condition) and
 * then mixed in *str (the NEXT char), dropping the first byte of the string.
 * On current code these tests FAIL; they PASS once hash_callback_str is fixed. */

TEST(adt_hash, str_first_char_contributes)
{
	/* strings differing only in byte 0 must not collide */
	EXPECT_NE(hash_callback_str((hash_key) "abc"), hash_callback_str((hash_key) "xbc"));
	EXPECT_NE(hash_callback_str((hash_key) "metacall"), hash_callback_str((hash_key) "detacall"));
}

TEST(adt_hash, str_single_char_not_all_equal)
{
	/* every 1-char string must not hash to the same value */
	hash a = hash_callback_str((hash_key) "a");
	hash b = hash_callback_str((hash_key) "b");
	hash z = hash_callback_str((hash_key) "z");

	EXPECT_NE(a, b);
	EXPECT_NE(a, z);
	EXPECT_NE(b, z);
}

TEST(adt_hash, str_matches_canonical_djb2)
{
	/* independent reference, not the implementation's own output */
	EXPECT_EQ(hash_callback_str((hash_key) ""), (hash)0x1505);
	EXPECT_EQ(hash_callback_str((hash_key) "a"), djb2_ref("a"));
	EXPECT_EQ(hash_callback_str((hash_key) "abc"), djb2_ref("abc"));
	EXPECT_EQ(hash_callback_str((hash_key) "xbc"), djb2_ref("xbc"));
	EXPECT_EQ(hash_callback_str((hash_key) "metacall"), djb2_ref("metacall"));
	EXPECT_EQ(hash_callback_str((hash_key) "detacall"), djb2_ref("detacall"));
}
