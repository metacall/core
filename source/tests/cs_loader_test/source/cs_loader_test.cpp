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

#include <gmock/gmock.h>

#include <metacall/metacall.h>

class cs_loader_test : public testing::Test
{
protected:
};

TEST_F(cs_loader_test, SayHello)
{
	ASSERT_NE((void *) NULL, (void *) metacall_function("SayHello"));

	metacall("SayHello");
}

TEST_F(cs_loader_test, SayAny)
{
	ASSERT_NE((void *) NULL, (void *) metacall_function("Say"));

	metacall("Say", "Any");
}

TEST_F(cs_loader_test, Jump)
{
	void * ret = NULL;

	ASSERT_NE((void *) NULL, (void *) metacall_function("SuperJump"));

	ret = metacall("SuperJump");

	EXPECT_NE((void *) NULL, (void *) ret);

	EXPECT_EQ((int) 2, (int) metacall_value_to_int(ret));

	metacall_value_destroy(ret);
}

TEST_F(cs_loader_test, Sum)
{
	void * ret = NULL;

	ASSERT_NE((void *) NULL, (void *) metacall_function("Sum"));

	ret = metacall("Sum", 5, 10);

	EXPECT_NE((void *) NULL, (void *) ret);

	EXPECT_EQ((int) 15, (int) metacall_value_to_int(ret));

	metacall_value_destroy(ret);
}

TEST_F(cs_loader_test, Concat)
{
	void * ret = NULL;

	ASSERT_NE((void *) NULL, (void *) metacall_function("Concat"));

	ret = metacall("Concat", "Hello ", "World");

	EXPECT_NE((void *) NULL, (void *) ret);

	EXPECT_EQ((int) 0, (int) strcmp((const char *)metacall_value_to_string(ret), "Hello World"));

	metacall_value_destroy(ret);
}
