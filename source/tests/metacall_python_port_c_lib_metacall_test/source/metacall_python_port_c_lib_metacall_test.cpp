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

class metacall_python_port_c_lib_metacall_test : public testing::Test
{
public:
};

TEST_F(metacall_python_port_c_lib_metacall_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	static const char buffer[] =
		/* Python */
		"import sys\n"
		"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
		"from metacall import metacall_load_from_package_ex\n"
		/* MetaCall require with options */
		"metacall_lib = metacall_load_from_package_ex('c', 'metacall', {\n"
		"    'include_search_paths': ['" METACALL_API_INCLUDE_DIR "', '" METACALL_API_BUILD_INCLUDE_DIR "', '" METACALL_INCLUDE_DIR "'],\n"
		"    'headers': ['" METACALL_INCLUDE_DIR "/metacall/metacall.h'],\n"
		"    'libs': ['" METACALL_LIBRARY "']\n"
		"})\n"
		/* Print all MetaCall APIs */
		"print([k for k in dir(metacall_lib) if k.startswith('metacall')])\n"
		/* Print info */
		"metacall_lib.metacall_print_info()\n"
		/* MetaCall Load from Memory */
		"script = b'module.exports = { metacircular: () => 46 }'\n"
		"assert metacall_lib.metacall_load_from_memory('node', script, len(script) + 1, None) == 0, 'metacall load from memory failed'\n"
		"result = metacall_lib.metacall('metacircular')\n"
		"print(result)\n"
		"metacircular = metacall_lib.metacall_value_to_double(result)\n"
		"print(metacircular)\n"
		"assert metacircular == 46, 'metacircular must be 46, invoke failed'\n"
		"\n";

	ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));

	metacall_destroy();
}
