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

#include <metacall/metacall.hpp>

#include <map>
#include <string>
#include <vector>

class metacall_cxx_port_test : public testing::Test
{
protected:
	void SetUp() override
	{
		ASSERT_EQ((int)0, (int)metacall::metacall_initialize());
	}

	void TearDown() override
	{
		metacall::metacall_destroy();
	}
};

static bool cxx_void_test_called = false;

void cxx_void_test(void)
{
	printf("hello from void\n");
	cxx_void_test_called = true;
}

static int cxx_void_args_test_value = 0;

void cxx_void_args_test(int a, int b)
{
	printf("hello from void with args %d %d\n", a, b);
	cxx_void_args_test_value = a + b;
}

int cxx_register_by_name(void)
{
	return 78;
}

std::nullptr_t cxx_map_test(metacall::map_typed<std::string, float> &m)
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

std::nullptr_t cxx_map_array_test(metacall::map_typed<std::string, metacall::array> &m)
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

std::nullptr_t cxx_recursive_map_test(metacall::map_typed<std::string, metacall::map_typed<std::string, float>> &m)
{
	EXPECT_EQ((float)m["hello"]["world"], (float)4.0f);

	printf("hello => %f\n", m["hello"]["world"]);
	fflush(stdout);

	return nullptr;
}

float cxx_float_int_int_test(int a0, int a1)
{
	EXPECT_EQ(a0, 7);
	EXPECT_EQ(a1, 8);

	return 3.0f;
}

TEST_F(metacall_cxx_port_test, void_function)
{
	cxx_void_test_called = false;

	auto fn = metacall::register_function(cxx_void_test);

	fn();

	EXPECT_EQ(cxx_void_test_called, true);
}

TEST_F(metacall_cxx_port_test, void_function_args)
{
	cxx_void_args_test_value = 0;

	auto fn = metacall::register_function("cxx_void_args_test", cxx_void_args_test);

	fn(3, 4);

	EXPECT_EQ(cxx_void_args_test_value, 7);

	void *args[] = {
		metacall::metacall_value_create_int(5),
		metacall::metacall_value_create_int(6)
	};

	void *ret = metacall::metacallv_s("cxx_void_args_test", args, 2);

	ASSERT_NE((void *)NULL, (void *)ret);
	EXPECT_EQ((enum metacall::metacall_value_id)metacall::METACALL_NULL, (enum metacall::metacall_value_id)metacall::metacall_value_id(ret));
	EXPECT_EQ(cxx_void_args_test_value, 11);

	metacall::metacall_value_destroy(ret);
	metacall::metacall_value_destroy(args[0]);
	metacall::metacall_value_destroy(args[1]);

	metacall::metacall<void>("cxx_void_args_test", 8, 9);

	EXPECT_EQ(cxx_void_args_test_value, 17);
}

TEST_F(metacall_cxx_port_test, register_by_name)
{
	auto fn = metacall::register_function("cxx_register_by_name", cxx_register_by_name);

	EXPECT_EQ(78, fn().to_value());
}

TEST_F(metacall_cxx_port_test, map)
{
	metacall::map_typed<std::string, float> m = {
		{ "hello", 3.0f },
		{ "world", 4.0f }
	};

	auto fn = metacall::register_function(cxx_map_test);
	auto v = fn(m);

	EXPECT_EQ(nullptr, v.to_value());
}

TEST_F(metacall_cxx_port_test, array)
{
	metacall::array a(3, 4.0f);

	auto fn = metacall::register_function(cxx_array_test);

	EXPECT_EQ(nullptr, fn(a).to_value());
}

TEST_F(metacall_cxx_port_test, array_return)
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

TEST_F(metacall_cxx_port_test, map_array)
{
	/* The temporary arrays are moved into the map, without copying them */
	metacall::map_typed<std::string, metacall::array> m = {
		{ "includes", metacall::array("/a/path", "/another/path") },
		{ "libraries", metacall::array("/a/path", "/another/path") }
	};

	auto fn = metacall::register_function(cxx_map_array_test);

	EXPECT_EQ(nullptr, fn(m).to_value());
}

TEST_F(metacall_cxx_port_test, map_array_no_copy)
{
	void *includes_raw = nullptr, *libraries_raw = nullptr;

	auto create_array = [](void *&raw) {
		metacall::array a("/a/path", "/another/path");
		raw = a.to_raw();
		return a;
	};

	metacall::array libraries("/a/path", "/another/path");

	libraries_raw = libraries.to_raw();

	metacall::map_typed<std::string, metacall::array> m = {
		{ "includes", create_array(includes_raw) },
		{ "libraries", std::move(libraries) }
	};

	EXPECT_EQ(m["includes"].to_raw(), includes_raw);
	EXPECT_EQ(m["libraries"].to_raw(), libraries_raw);

	/* Arrays passed as lvalue are copied, because they are still owned by the caller */
	metacall::array headers("/a/path");

	metacall::map_typed<std::string, metacall::array> copied = {
		{ "headers", headers }
	};

	EXPECT_NE(copied["headers"].to_raw(), headers.to_raw());
	EXPECT_STREQ(headers[0].as<std::string>().c_str(), "/a/path");
}

TEST_F(metacall_cxx_port_test, recursive_map_call)
{
	metacall::map_typed<std::string, metacall::map_typed<std::string, float>> m = {
		{ "hello", { { "world", 4.0f } } }
	};

	auto fn = metacall::register_function(cxx_recursive_map_test);

	EXPECT_EQ(nullptr, fn(m).to_value());
}

TEST_F(metacall_cxx_port_test, recursive_map)
{
	using inner = metacall::map_typed<std::string, float>;
	using middle = metacall::map_typed<std::string, inner>;
	using outer = metacall::map_typed<std::string, middle>;

	outer m = {
		{ "a", { { "b", { { "c", 1.0f }, { "d", 2.0f } } } } },
		{ "e", { { "f", { { "g", 3.0f } } }, { "h", {} } } }
	};

	EXPECT_EQ((size_t)2, (size_t)m.count());
	EXPECT_EQ((float)m["a"]["b"]["c"], (float)1.0f);
	EXPECT_EQ((float)m["a"]["b"]["d"], (float)2.0f);
	EXPECT_EQ((float)m["e"]["f"]["g"], (float)3.0f);
	EXPECT_EQ((size_t)0, (size_t)m["e"]["h"].count());
	EXPECT_FALSE(m("z").has_value());

	// Elements are converted once, accessing them does not convert them again
	EXPECT_EQ(&m["a"], &m["a"]);
	EXPECT_EQ(&m["a"]["b"], &m["a"]["b"]);

	float sum = 0.0f;

	for (const auto &[key, value] : m)
	{
		for (const auto &[inner_key, inner_value] : value)
		{
			for (const auto &[leaf_key, leaf_value] : inner_value)
			{
				sum += leaf_value;
			}
		}
	}

	EXPECT_EQ((float)sum, (float)6.0f);
}

TEST_F(metacall_cxx_port_test, map_view)
{
	using inner = metacall::map_typed<std::string, float>;
	using outer = metacall::map_typed<std::string, inner>;

	outer m = {
		{ "hello", { { "world", 4.0f } } }
	};

	// A copy is a view, it points to the same value and shares the converted elements
	outer view(m);

	EXPECT_EQ(view.to_raw(), m.to_raw());
	EXPECT_EQ(&view["hello"], &m["hello"]);

	// Moving a view into another map copies the value, because the view does not own it
	inner inner_view(m["hello"]);

	outer other = {
		{ "copy", std::move(inner_view) }
	};

	EXPECT_NE(other["copy"].to_raw(), m["hello"].to_raw());
	EXPECT_EQ((float)other["copy"]["world"], (float)4.0f);
	EXPECT_EQ((float)m["hello"]["world"], (float)4.0f);
}

TEST_F(metacall_cxx_port_test, runtime_map_from_range)
{
	using inner = metacall::map_typed<std::string, float>;

	const std::map<std::string, float> values = {
		{ "a", 1.0f },
		{ "b", 2.0f },
		{ "c", 3.0f }
	};

	inner from_range(values.begin(), values.end());

	EXPECT_EQ((size_t)values.size(), (size_t)from_range.count());

	for (const auto &[key, value] : values)
	{
		EXPECT_EQ((float)from_range[key], (float)value);
	}
}

TEST_F(metacall_cxx_port_test, runtime_map_from_move_range)
{
	using inner = metacall::map_typed<std::string, float>;
	using outer = metacall::map_typed<std::string, inner>;

	const std::map<std::string, float> values = {
		{ "a", 1.0f },
		{ "b", 2.0f },
		{ "c", 3.0f }
	};

	std::vector<std::pair<std::string, inner>> pairs;

	for (size_t i = 0; i < 3; ++i)
	{
		pairs.emplace_back("map_" + std::to_string(i), inner(values.begin(), values.end()));
	}

	outer from_move_range(std::make_move_iterator(pairs.begin()), std::make_move_iterator(pairs.end()));

	EXPECT_EQ((size_t)3, (size_t)from_move_range.count());
	EXPECT_EQ((float)from_move_range["map_2"]["c"], (float)3.0f);
}

TEST_F(metacall_cxx_port_test, runtime_map_from_entries)
{
	using inner = metacall::map_typed<std::string, float>;
	using outer = metacall::map_typed<std::string, inner>;

	std::vector<outer::entry> entries;

	for (size_t i = 0; i < 3; ++i)
	{
		std::vector<inner::entry> inner_entries;

		for (size_t j = 0; j <= i; ++j)
		{
			inner_entries.emplace_back("value_" + std::to_string(j), (float)j);
		}

		entries.emplace_back("map_" + std::to_string(i), inner(std::move(inner_entries)));
	}

	outer from_entries(std::move(entries));

	EXPECT_EQ((size_t)3, (size_t)from_entries.count());

	for (size_t i = 0; i < 3; ++i)
	{
		const auto &map = from_entries["map_" + std::to_string(i)];

		EXPECT_EQ((size_t)(i + 1), (size_t)map.count());

		for (size_t j = 0; j <= i; ++j)
		{
			EXPECT_EQ((float)map["value_" + std::to_string(j)], (float)j);
		}
	}

	// Empty map
	outer empty(std::vector<outer::entry>{});

	EXPECT_EQ((size_t)0, (size_t)empty.count());
}

TEST_F(metacall_cxx_port_test, runtime_map_call)
{
	using inner = metacall::map_typed<std::string, float>;
	using outer = metacall::map_typed<std::string, inner>;

	std::vector<outer::entry> hello;

	hello.emplace_back("hello", inner{ { "world", 4.0f } });

	outer runtime_map(std::move(hello));

	auto fn = metacall::register_function(cxx_recursive_map_test);

	EXPECT_EQ(nullptr, fn(runtime_map).to_value());
}

TEST_F(metacall_cxx_port_test, map_value)
{
	static const char json_map[] =
		"{\n"
		"	\"urls\": [\n"
		"		\"http://localhost:8000/\",\n"
		"		\"http://localhost:8001/\",\n"
		"		\"http://localhost:8002/\"\n"
		"	],\n"
		"	\"timeout\": 1000,\n"
		"	\"retry\": 60\n"
		"}\n";

	metacall::metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(metacall::METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	void *json_value = metacall::metacall_deserialize(metacall::metacall_serial(), json_map, sizeof(json_map), allocator);

	metacall::map_typed<std::string, metacall::value> config(json_value);

	auto urls = config["urls"].as<metacall::array>();

	EXPECT_STREQ(urls[0].as<std::string>().c_str(), "http://localhost:8000/");
	EXPECT_STREQ(urls[1].as<std::string>().c_str(), "http://localhost:8001/");
	EXPECT_STREQ(urls[2].as<std::string>().c_str(), "http://localhost:8002/");

	for (auto url : urls)
	{
		std::cout << url.as<std::string>() << std::endl;
	}

	auto timeout = config["timeout"].as<int>();

	EXPECT_EQ(timeout, 1000L);

	auto retry = config["retry"].as<int>();

	EXPECT_EQ(retry, 60L);

	metacall::metacall_value_destroy(json_value);

	metacall::metacall_allocator_destroy(allocator);
}

TEST_F(metacall_cxx_port_test, float_int_int)
{
	auto fn = metacall::register_function(cxx_float_int_int_test);

	EXPECT_EQ(3.0f, fn(7, 8).to_value());
}

TEST_F(metacall_cxx_port_test, inspect)
{
	/* Register a function so it appears in the inspect information */
	auto fn = metacall::register_function("cxx_register_by_name", cxx_register_by_name);

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
