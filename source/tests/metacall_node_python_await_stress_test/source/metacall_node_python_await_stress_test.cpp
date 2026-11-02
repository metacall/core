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

#include <preprocessor/preprocessor_stringify.h>

/* Amount of awaits and chains launched at once, each one creates its own async handle */
#define METACALL_NODE_PYTHON_AWAIT_STRESS_SIZE 2000

class metacall_node_python_await_stress_test : public testing::Test
{
public:
};

TEST_F(metacall_node_python_await_stress_test, DefaultConstructor)
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
			"function node_sleep(n) { return new Promise(resolve => { setTimeout(() => resolve(n), 0); }); }\n"
			"module.exports = { node_sleep };\n"
			"`);\n"
			"metacall_load_from_memory('py', `"
			/* Python */
			"async def py_double(n):\n"
			"\treturn n * 2\n"
			"`);\n"
			/* NodeJS Check, everything runs in the V8 thread so plain counters are enough */
			"const size = " PREPROCESSOR_STRINGIFY(METACALL_NODE_PYTHON_AWAIT_STRESS_SIZE) ";\n"
			"let awaits = 0;\n"
			"let chains = 0;\n"
			"process.on('exit', () => {\n"
			"	console.log('AWAITS:', awaits, 'CHAINS:', chains, 'SIZE:', size);\n"
			"	if (awaits != size || chains != size) {\n"
			"		process.exit(3);\n"
			"	}\n"
			"});\n"
			"const check = (expected, v) => {\n"
			"	if (v !== expected) {\n"
			"		console.log('EXPECTED:', expected, 'RESULT:', v);\n"
			"		process.exit(1);\n"
			"	}\n"
			"};\n"
			"const error = v => {\n"
			"	console.log('ERROR:', v);\n"
			"	process.exit(2);\n"
			"};\n"
			/* Python awaits launched at once, all of them are resolved from the Python thread */
			"for (let i = 0; i < size; i++) {\n"
			"	metacall_await('py_double', i).then(v => {\n"
			"		check(i * 2, v);\n"
			"		awaits++;\n"
			"	}).catch(error);\n"
			"}\n"
			/* NodeJS Promise Chains launched at once, mixing resolutions from the NodeJS and the Python threads */
			"for (let i = 0; i < size; i++) {\n"
			"	metacall_await('node_sleep', i).then(v => {\n"
			"		check(i, v);\n"
			"		return metacall_await('py_double', v);\n"
			"	}).then(v => {\n"
			"		check(i * 2, v);\n"
			"		return metacall_await('node_sleep', v + 1);\n"
			"	}).then(v => {\n"
			"		check(i * 2 + 1, v);\n"
			"		chains++;\n"
			"	}).catch(error);\n"
			"}\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}
