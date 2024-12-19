/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_python_port_https_test : public testing::Test
{
public:
};

TEST_F(metacall_python_port_https_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		// Test import bug (__metacall_import__() missing 1 required positional argument: 'name')
		static const char buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"import metacall\n"
			"from http import client\n"
			"def fetch_http_py(url: str):\n"
			"	try:\n"
			"		conn = client.HTTPSConnection(url, 443)\n"
			"		conn.request('GET', '/')\n"
			"		response = conn.getresponse()\n"
			"		data = response.read()\n"
			"		conn.close()\n"
			"		return data\n"
			"	except Exception as e:\n"
			"		print(e)\n"
			"		sys.stdout.flush()\n"
			"		return b'<!doctype invalid>'\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", buffer, sizeof(buffer), NULL));

		void *ret = metacall("fetch_http_py", "www.google.com");

		static const char prefix[] = "<!doctype html>";

		ASSERT_EQ((enum metacall_value_id)METACALL_BUFFER, (enum metacall_value_id)metacall_value_id(ret));

		EXPECT_EQ((int)0, (int)strncmp((const char *)metacall_value_to_buffer(ret), prefix, sizeof(prefix) - 1));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}
