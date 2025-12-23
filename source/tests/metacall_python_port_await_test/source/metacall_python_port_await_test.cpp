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

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>
#include <metacall/metacall_value.h>

class metacall_python_port_await_test : public testing::Test
{
public:
};

TEST_F(metacall_python_port_await_test, BasicAwait)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_NODE) && defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"import asyncio\n"
			"from metacall import metacall_load_from_memory, metacall_await\n"
			"\n"
			"# Load Node.js async function\n"
			"node_script = '''\n"
			"async function test_add(a, b) {\n"
			"    return a + b;\n"
			"}\n"
			"module.exports = { test_add };\n"
			"'''\n"
			"metacall_load_from_memory('node', node_script)\n"
			"\n"
			"# Test basic await\n"
			"async def test():\n"
			"    result = await metacall_await('test_add', 5, 3)\n"
			"    if result != 8:\n"
			"        raise Exception(f'Expected 8, got {result}')\n"
			"\n"
			"asyncio.run(test())\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}

TEST_F(metacall_python_port_await_test, MultipleAwaits)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_NODE) && defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"import asyncio\n"
			"from metacall import metacall_load_from_memory, metacall_await\n"
			"\n"
			"node_script = '''\n"
			"async function multiply(a, b) {\n"
			"    return a * b;\n"
			"}\n"
			"module.exports = { multiply };\n"
			"'''\n"
			"metacall_load_from_memory('node', node_script)\n"
			"\n"
			"async def test():\n"
			"    r1 = await metacall_await('multiply', 2, 3)\n"
			"    r2 = await metacall_await('multiply', 4, 5)\n"
			"    r3 = await metacall_await('multiply', 6, 7)\n"
			"    if r1 != 6 or r2 != 20 or r3 != 42:\n"
			"        raise Exception(f'Wrong results: {r1}, {r2}, {r3}')\n"
			"\n"
			"asyncio.run(test())\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}

TEST_F(metacall_python_port_await_test, ConcurrentAwaits)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_NODE) && defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"import asyncio\n"
			"from metacall import metacall_load_from_memory, metacall_await\n"
			"\n"
			"node_script = '''\n"
			"async function delayed(val, ms) {\n"
			"    return new Promise(r => setTimeout(() => r(val), ms));\n"
			"}\n"
			"module.exports = { delayed };\n"
			"'''\n"
			"metacall_load_from_memory('node', node_script)\n"
			"\n"
			"async def test():\n"
			"    results = await asyncio.gather(\n"
			"        metacall_await('delayed', 'a', 30),\n"
			"        metacall_await('delayed', 'b', 20),\n"
			"        metacall_await('delayed', 'c', 10)\n"
			"    )\n"
			"    if set(results) != {'a', 'b', 'c'}:\n"
			"        raise Exception(f'Wrong results: {results}')\n"
			"\n"
			"asyncio.run(test())\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}

TEST_F(metacall_python_port_await_test, MetaCallFunctionClass)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_NODE) && defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"import asyncio\n"
			"from metacall import metacall_load_from_memory, MetaCallFunction\n"
			"\n"
			"node_script = '''\n"
			"async function async_square(x) { return x * x; }\n"
			"function sync_double(x) { return x * 2; }\n"
			"module.exports = { async_square, sync_double };\n"
			"'''\n"
			"metacall_load_from_memory('node', node_script)\n"
			"\n"
			"# Test sync function\n"
			"sync_func = MetaCallFunction('sync_double', is_async=False)\n"
			"result = sync_func(5)\n"
			"if result != 10:\n"
			"    raise Exception(f'Expected 10, got {result}')\n"
			"\n"
			"# Test async function\n"
			"async def test_async():\n"
			"    async_func = MetaCallFunction('async_square', is_async=True)\n"
			"    result = await async_func(4)\n"
			"    if result != 16:\n"
			"        raise Exception(f'Expected 16, got {result}')\n"
			"\n"
			"asyncio.run(test_async())\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}

TEST_F(metacall_python_port_await_test, AwaitWithComplexTypes)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

#if defined(OPTION_BUILD_LOADERS_NODE) && defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"import asyncio\n"
			"from metacall import metacall_load_from_memory, metacall_await\n"
			"\n"
			"node_script = '''\n"
			"async function return_object() {\n"
			"    return { name: 'test', values: [1, 2, 3] };\n"
			"}\n"
			"async function return_array() {\n"
			"    return [{ a: 1 }, { b: 2 }];\n"
			"}\n"
			"module.exports = { return_object, return_array };\n"
			"'''\n"
			"metacall_load_from_memory('node', node_script)\n"
			"\n"
			"async def test():\n"
			"    obj = await metacall_await('return_object')\n"
			"    if obj['name'] != 'test' or obj['values'] != [1, 2, 3]:\n"
			"        raise Exception(f'Wrong object: {obj}')\n"
			"    \n"
			"    arr = await metacall_await('return_array')\n"
			"    if arr[0]['a'] != 1 or arr[1]['b'] != 2:\n"
			"        raise Exception(f'Wrong array: {arr}')\n"
			"\n"
			"asyncio.run(test())\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}
