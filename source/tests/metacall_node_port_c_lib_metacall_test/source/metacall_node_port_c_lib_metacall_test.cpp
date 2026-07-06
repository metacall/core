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

class metacall_node_port_c_lib_metacall_test : public testing::Test
{
public:
};

TEST_F(metacall_node_port_c_lib_metacall_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	static const char buffer[] =
		/* NodeJS */
		"const assert = require('assert');\n"
		"const { metacall_load_from_package_ex } = require('" METACALL_NODE_PORT_PATH "');\n"
		/* MetaCall Require with options */
		"const metacall = metacall_load_from_package_ex('c', 'metacall', {\n"
		"	include_search_paths: ['" METACALL_API_INCLUDE_DIR "', '" METACALL_INCLUDE_DIR "'],\n"
		"	headers: ['" METACALL_INCLUDE_DIR "/metacall/metacall.h'],\n"
		"	libs: ['" METACALL_LIBRARY "']\n"
		"});\n"
		/* Print all MetaCall APIs */
		"console.log(Object.keys(metacall).filter(k => k.startsWith('metacall')));\n"
		/* Print info */
		"const { metacall_print_info } = metacall;\n"
		"console.log(metacall_print_info);\n"
		"console.log(metacall_print_info());\n"
		/* MetaCall Load from Memory */
		"{\n"
		"	const script = 'module.exports = { metacircular: () => 46 }';\n"
		"	assert(metacall.metacall_load_from_memory('node', script, script.length + 1, null) == 0, 'metacall load from memory failed');\n"
		"	const result = metacall.metacall('metacircular');\n"
		"	console.log(result);\n"
		"	const metacircular = metacall.metacall_value_to_double(result);\n"
		"	console.log(metacircular);\n"
		"	assert(metacircular == 46, 'metacircular must be 46, invoke failed');\n"
		"}\n"
		/* MetaCall Load from Memory with Handle */
		"{\n"
		"	const script = 'module.exports = { metacircular: (text) => { console.log(text); return 69; } }';\n"
		"	const handle_ptr = metacall.metacall_value_create_ptr(null);\n"
		"	console.log('handle_ptr:', handle_ptr);\n"
		"	assert(metacall.metacall_load_from_memory('node', script, script.length + 1, handle_ptr) == 0, 'metacall load from memory with handle failed');\n"
		"	console.log('handle_ptr after:', handle_ptr);\n"
		"	const handle = metacall.metacall_value_to_ptr(handle_ptr);"
		"	console.log('handle_ptr contents after:', handle);\n"
		"	const text = 'this is a test';\n"
		"	const args = [ metacall.metacall_value_create_string(text, text.length) ];\n"
		"	const result = metacall.metacallhv(handle, 'metacircular', args);\n"
		"	console.log(result);\n"
		// TODO: Implement enum type in C Loader
		// "	console.log('value id:', metacall.metacall_value_id(result));\n"
		"	const metacircular = metacall.metacall_value_to_double(result);\n"
		"	console.log(metacircular);\n"
		"	assert(metacircular == 69, 'metacircular must be 69, invoke failed');\n"
		"}\n"
		"\n";

	ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

	metacall_destroy();
}
