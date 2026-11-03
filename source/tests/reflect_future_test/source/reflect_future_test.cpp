/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#include <reflect/reflect_future.h>
#include <reflect/reflect_value_type.h>

static int future_destroy_count = 0;

static int future_test_interface_create(future f, future_impl impl)
{
	(void)f;
	(void)impl;

	return 0;
}

static future_return future_test_interface_await(future f, future_impl impl, future_resolve_callback resolve_callback, future_reject_callback reject_callback, void *context)
{
	(void)f;
	(void)reject_callback;

	return resolve_callback(value_create_int(*static_cast<int *>(impl)), context);
}

static void future_test_interface_destroy(future f, future_impl impl)
{
	(void)f;
	(void)impl;

	++future_destroy_count;
}

static future_interface future_test_singleton(void)
{
	static struct future_interface_type interface_instance = {
		&future_test_interface_create,
		&future_test_interface_await,
		&future_test_interface_destroy
	};

	return &interface_instance;
}

static value future_test_resolve(value v, void *context)
{
	(void)context;

	return v;
}

class reflect_future_test : public testing::Test
{
protected:
	void SetUp() override
	{
		future_destroy_count = 0;
	}
};

TEST_F(reflect_future_test, CopyKeepsFutureAlive)
{
	int result = 15;

	value v = value_create_future(future_create(&result, &future_test_singleton));

	ASSERT_NE((value)NULL, (value)v);

	value cpy = value_type_copy(v);

	ASSERT_NE((value)NULL, (value)cpy);

	/* Destroying the original must not destroy the future shared with the copy */
	value_type_destroy(v);

	EXPECT_EQ((int)0, (int)future_destroy_count);

	value ret = future_await(value_to_future(cpy), &future_test_resolve, NULL, NULL);

	ASSERT_NE((value)NULL, (value)ret);
	EXPECT_EQ((int)15, (int)value_to_int(ret));

	value_type_destroy(ret);

	/* Destroying the last reference destroys the future */
	value_type_destroy(cpy);

	EXPECT_EQ((int)1, (int)future_destroy_count);
}

TEST_F(reflect_future_test, CopyArrayOfFutures)
{
	int result = 20;

	value array = value_create_array(NULL, 1);

	ASSERT_NE((value)NULL, (value)array);

	value *array_ptr = value_to_array(array);

	array_ptr[0] = value_create_future(future_create(&result, &future_test_singleton));

	value cpy = value_type_copy(array);

	ASSERT_NE((value)NULL, (value)cpy);

	value_type_destroy(array);

	EXPECT_EQ((int)0, (int)future_destroy_count);

	value_type_destroy(cpy);

	EXPECT_EQ((int)1, (int)future_destroy_count);
}
