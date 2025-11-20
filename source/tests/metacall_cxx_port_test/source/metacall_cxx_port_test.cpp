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

#include <metacall/metacall.hpp>

class metacall_cxx_port_test : public testing::Test
{
protected:
};

// TODO:
/*
static bool cxx_void_test_called = false;

void cxx_void_test(void)
{
	printf("hello from void\n");
	cxx_void_test_called = true;
}
*/

std::nullptr_t cxx_map_test(metacall::map<std::string, float> &m)
{
	EXPECT_EQ((float)m["hello"], (float)3.0f);
	EXPECT_EQ((float)m["world"], (float)4.0f);

	printf("hello => %f\n", m["hello"]);
	printf("world => %f\n", m["world"]);
	fflush(stdout);

	return nullptr;
}

std::nullptr_t cxx_array_test(metacall::array &a)
{
	EXPECT_EQ((float)a[0].as<int>(), (int)3);
	EXPECT_EQ((float)a[1].as<float>(), (float)4.0f);

	EXPECT_EQ((float)a.get<int>(0), (int)3);
	EXPECT_EQ((float)a.get<float>(1), (float)4.0f);

	printf("a[0] => %d\n", a[0].as<int>());
	printf("a[1] => %f\n", a[1].as<float>());
	fflush(stdout);

	return nullptr;
}

metacall::array cxx_array_ret_test()
{
	metacall::array a(3, 4.0f);

	return a;
}

std::nullptr_t cxx_map_array_test(metacall::map<std::string, metacall::array> &m)
{
	EXPECT_STREQ(m["includes"][0].as<std::string>().c_str(), "/a/path");
	EXPECT_STREQ(m["includes"][1].as<std::string>().c_str(), "/another/path");

	EXPECT_STREQ(m["libraries"][0].as<std::string>().c_str(), "/a/path");
	EXPECT_STREQ(m["libraries"][1].as<std::string>().c_str(), "/another/path");

	printf("m['includes'][0] => %s\n", m["includes"][0].as<std::string>().c_str());
	printf("m['includes'][1] => %s\n", m["includes"][1].as<std::string>().c_str());

	printf("m['libraries'][0] => %s\n", m["libraries"][0].as<std::string>().c_str());
	printf("m['libraries'][1] => %s\n", m["libraries"][1].as<std::string>().c_str());

	return nullptr;
}

// TODO:
/*
std::nullptr_t cxx_recursive_map_test(metacall::map<std::string, metacall::map<std::string, float>> &m)
{
	EXPECT_EQ((float)m["hello"]["world"], (float)4.0f);

	printf("hello => %f\n", m["hello"]["world"]);
	fflush(stdout);

	return nullptr;
}
*/

float cxx_float_int_int_test(int a0, int a1)
{
	EXPECT_EQ(a0, 7);
	EXPECT_EQ(a1, 8);

	return 3.0f;
}

TEST_F(metacall_cxx_port_test, DefaultConstructor)
{
	ASSERT_EQ((int)0, (int)metacall::metacall_initialize());

	// TODO:
	/*
	{
		metacall::array a(3, 4.0f);

		auto fn = metacall::register_function(cxx_void_test);

		EXPECT_EQ(nullptr, fn().to_value());
		EXPECT_EQ(cxx_void_test_called, true);
	}
	*/

	{
		metacall::map<std::string, float> m = {
			{ "hello", 3.0f },
			{ "world", 4.0f }
		};

		auto fn = metacall::register_function(cxx_map_test);
		auto v = fn(m);

		EXPECT_EQ(nullptr, v.to_value());
	}

	{
		metacall::array a(3, 4.0f);

		auto fn = metacall::register_function(cxx_array_test);

		EXPECT_EQ(nullptr, fn(a).to_value());
	}

	{
		auto fn = metacall::register_function(cxx_array_ret_test);

		auto v = fn();
		auto a = v.to_value();

		EXPECT_EQ((float)a[0].as<int>(), (int)3);
		EXPECT_EQ((float)a[1].as<float>(), (float)4.0f);

		EXPECT_EQ((float)a.get<int>(0), (int)3);
		EXPECT_EQ((float)a.get<float>(1), (float)4.0f);

		printf("a[0] => %d\n", a[0].as<int>());
		printf("a[1] => %f\n", a[1].as<float>());
		fflush(stdout);
	}

#if 0
	{
		metacall::map<std::string, metacall::array> m = {
			{ "includes", metacall::array("/a/path", "/another/path") },
			{ "libraries", metacall::array("/a/path", "/another/path") }
		};

		auto fn = metacall::register_function(cxx_map_array_test);

		EXPECT_EQ(nullptr, fn(m).to_value());
	}
#endif

	// TODO:
	/*
	{
		metacall::map<std::string, metacall::map<std::string, float>> m = {
			{ "hello", { "world", 4.0f } }
		};

		auto fn = metacall::register_function(cxx_recursive_map_test);

		EXPECT_EQ(nullptr, fn(m));
	}
	*/

	{
		auto fn = metacall::register_function(cxx_float_int_int_test);

		EXPECT_EQ(3.0f, fn(7, 8).to_value());
	}

	/* Print inspect information */
	{
		size_t size = 0;

		metacall::metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *allocator = metacall_allocator_create(metacall::METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char *inspect_str = metacall::metacall_inspect(&size, allocator);

		EXPECT_NE((char *)NULL, (char *)inspect_str);

		EXPECT_GT((size_t)size, (size_t)0);

		std::cout << inspect_str << std::endl;

		metacall::metacall_allocator_free(allocator, inspect_str);

		metacall::metacall_allocator_destroy(allocator);
	}

	metacall::metacall_destroy();
}
