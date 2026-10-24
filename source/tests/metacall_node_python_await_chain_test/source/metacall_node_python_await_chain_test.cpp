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

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>
#include <metacall/metacall_value.h>

class metacall_node_python_await_chain_test : public testing::Test
{
public:
};

TEST_F(metacall_node_python_await_chain_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS & Python */
#if defined(OPTION_BUILD_LOADERS_NODE) && defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char buffer[] =
			/* NodeJS */
			"const { metacall_await, metacall_load_from_memory } = require('" METACALL_NODE_PORT_PATH "');\n"
			"metacall_load_from_memory('node', `"
			/* NodeJS */
			"function node_sleep(n) { return new Promise(resolve => { setTimeout(() => resolve(n), 100); }); }\n"
			"module.exports = { node_sleep };\n"
			"`);\n"
			"metacall_load_from_memory('py', `"
			/* Python */
			"async def py_double(n):\n"
			"\treturn n * 2\n"
			"`);\n"
			/* NodeJS Check */
			"let buffer = new SharedArrayBuffer(4);\n"
			"let int32 = new Int32Array(buffer);\n"
			"Atomics.store(int32, 0, 0);\n"
			"process.on('exit', () => {\n"
			"	if (Atomics.load(int32, 0) != 5) {\n"
			"		process.exit(3);\n"
			"	}\n"
			"});\n"
			"const step = (expected, v) => {\n"
			"	console.log('STEP:', Atomics.load(int32, 0), 'RESULT:', v);\n"
			"	if (v !== expected) {\n"
			"		process.exit(1);\n"
			"	}\n"
			"	Atomics.add(int32, 0, 1);\n"
			"};\n"
			/* NodeJS Promise Chain, each then returns another await or a plain value */
			"metacall_await('node_sleep', 1).then(v => {\n"
			"	step(1, v);\n"
			"	return metacall_await('py_double', v);\n"
			"}).then(v => {\n"
			"	step(2, v);\n"
			"	return metacall_await('node_sleep', v + 3);\n"
			"}).then(v => {\n"
			"	step(5, v);\n"
			"	return v * 10;\n"
			"}).then(v => {\n"
			"	step(50, v);\n"
			"	return metacall_await('py_double', v);\n"
			"}).then(v => {\n"
			"	step(100, v);\n"
			"}).catch(v => {\n"
			"	console.log('ERROR:', v);\n"
			"	process.exit(2);\n"
			"});\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}
