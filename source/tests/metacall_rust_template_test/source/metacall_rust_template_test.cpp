/*
 *	Loader Library by Parra Studios
 *	A plugin for loading rust code at run-time into a process.
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

#include <rs_loader/rs_loader_impl.h>

class metacall_rust_template_test : public testing::Test
{
protected:
	static void SetUpTestSuite()
	{
	}

	static void TearDownTestSuite()
	{
	}
};

TEST_F(metacall_rust_template_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ(0, metacall_initialize());

	const char script[] =
		"pub fn identity<T>(x: T) -> T {"
		"    x"
		"}"
		"pub fn pair<T, U>(a: T, b: U) -> U {"
		"    b"
		"}";

	const char *identity_types[] = {
		"f64"
	};

	const char *pair_types[] = {
		"i32",
		"f64"
	};

	const rs_loader_template_instantiation instantiations[] = {
		{ "identity",
			identity_types,
			1 },
		{ "pair",
			pair_types,
			2 }
	};

	const rs_loader_load_data data = {
		instantiations,
		2
	};

	int result = metacall_load_from_memory_ex(
		"rs",
		script,
		sizeof(script) - 1,
		NULL,
		(void *)&data);

	ASSERT_EQ(0, result);

	void *identity = metacall_function("identity::<f64>");
	void *pair = metacall_function("pair::<i32, f64>");

	std::cout << "identity::<f64>: " << identity << std::endl;
	std::cout << "pair::<i32, f64>: " << pair << std::endl;

	ASSERT_NE(identity, nullptr);
	ASSERT_NE(pair, nullptr);

	void *identity_ret = metacall("identity::<f64>", 42.5);

	ASSERT_NE(identity_ret, nullptr);
	EXPECT_EQ((double)metacall_value_to_double(identity_ret), 42.5);

	void *pair_ret = metacall("pair::<i32, f64>", 123, 3.14);

	ASSERT_NE(pair_ret, nullptr);
	EXPECT_EQ((double)metacall_value_to_double(pair_ret), 3.14);

	metacall_destroy();
}
