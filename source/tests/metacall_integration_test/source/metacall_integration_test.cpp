/*
 *	MetaCall Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_integration_test : public testing::Test
{
protected:
};

TEST_F(metacall_integration_test, CsSayHello)
{
	metacall("SayHello");
}

TEST_F(metacall_integration_test, CsSayAny)
{
	metacall("Say", "Any");
}

TEST_F(metacall_integration_test, PyMultiply)
{
	void *ret = NULL;

	ret = metacall("multiply", 5, 15);

	EXPECT_NE((void *)NULL, (void *)ret);

	EXPECT_EQ((long)metacall_value_to_long(ret), (long)75L);

	metacall_value_destroy(ret);
}

TEST_F(metacall_integration_test, Sum)
{
	void *ret = NULL;

	ret = metacall("Sum", 5, 10);

	EXPECT_NE((void *)NULL, (void *)ret);

	EXPECT_EQ((int)metacall_value_to_int(ret), (int)15);

	metacall_value_destroy(ret);
}
