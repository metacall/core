/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
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

void *cxx_map_test(size_t argc, void *args[], void *data)
{
	metacall::map<std::string, float> m(args[0]);

	(void)argc;
	(void)data;

	EXPECT_EQ((float)m["hello"], (float)3.0f);
	EXPECT_EQ((float)m["world"], (float)4.0f);

	printf("hello => %f\n", m["hello"]);
	printf("world => %f\n", m["world"]);
	fflush(stdout);

	return metacall::metacall_value_create_null();
}

void *cxx_array_test(size_t argc, void *args[], void *data)
{
	metacall::array a(args[0]);

	(void)argc;
	(void)data;

	EXPECT_EQ((float)a[0].as<int>(), (int)3);
	EXPECT_EQ((float)a[1].as<float>(), (float)4.0f);

	EXPECT_EQ((float)a.get<int>(0), (int)3);
	EXPECT_EQ((float)a.get<float>(1), (float)4.0f);

	printf("a[0] => %d\n", a[0].as<int>());
	printf("a[1] => %f\n", a[1].as<float>());
	fflush(stdout);

	return metacall::metacall_value_create_null();
}

void *cxx_map_array_test(size_t argc, void *args[], void *data)
{
	metacall::map<std::string, metacall::array> m(args[0]);

	(void)argc;
	(void)data;

	EXPECT_STREQ(m["includes"][0].as<std::string>().c_str(), "/a/path");
	EXPECT_STREQ(m["includes"][1].as<std::string>().c_str(), "/another/path");

	EXPECT_STREQ(m["libraries"][0].as<std::string>().c_str(), "/a/path");
	EXPECT_STREQ(m["libraries"][1].as<std::string>().c_str(), "/another/path");

	printf("m['includes'][0] => %s\n", m["includes"][0].as<std::string>().c_str());
	printf("m['includes'][1] => %s\n", m["includes"][1].as<std::string>().c_str());

	printf("m['libraries'][0] => %s\n", m["libraries"][0].as<std::string>().c_str());
	printf("m['libraries'][1] => %s\n", m["libraries"][1].as<std::string>().c_str());

	return metacall::metacall_value_create_null();
}

// TODO:
/*
void *cxx_recursive_map_test(size_t argc, void *args[], void *data)
{
	metacall::map<std::string, metacall::map<std::string, float>> m(args[0]);

	(void)argc;
	(void)data;

	EXPECT_EQ((float)m["hello"]["world"], (float)4.0f);

	printf("hello => %f\n", m["hello"]["world"]);
	fflush(stdout);

	return metacall_value_create_null();
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

#if 0
	{
		metacall::map<std::string, float> m = {
			{ "hello", 3.0f },
			{ "world", 4.0f }
		};

		metacall::metacall_register("cxx_map_test", cxx_map_test, NULL, metacall::METACALL_NULL, 1, metacall::METACALL_MAP);

		EXPECT_EQ(nullptr, metacall::metacall<std::nullptr_t>("cxx_map_test", m));
	}

	{
		metacall::array a(3, 4.0f);

		metacall::metacall_register("cxx_array_test", cxx_array_test, NULL, metacall::METACALL_NULL, 1, metacall::METACALL_ARRAY);

		EXPECT_EQ(nullptr, metacall::metacall<std::nullptr_t>("cxx_array_test", a));
	}

	{
		metacall::map<std::string, metacall::array> m = {
			{ "includes", metacall::array("/a/path", "/another/path") },
			{ "libraries", metacall::array("/a/path", "/another/path") }
		};

		metacall::metacall_register("cxx_map_array_test", cxx_map_array_test, NULL, metacall::METACALL_NULL, 1, metacall::METACALL_MAP);

		EXPECT_EQ(nullptr, metacall::metacall<std::nullptr_t>("cxx_map_array_test", m));
	}

	// TODO:
	/*
	{
		metacall::map<std::string, metacall::map<std::string, float>> m = {
			{ "hello", { "world", 4.0f } }
		};

		metacall::metacall_register("cxx_recursive_map_test", cxx_recursive_map_test, NULL, metacall::METACALL_NULL, 1, metacall::METACALL_MAP);

		EXPECT_EQ(nullptr, metacall::metacall<std::nullptr_t>("cxx_recursive_map_test", m));
	}
	*/
#endif
	{
		auto fn = metacall::register_function(cxx_float_int_int_test);

		EXPECT_EQ(3.0f, fn(7, 8));
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
