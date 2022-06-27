/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <atomic>

std::atomic_bool result(false);

class metacall_node_signal_handler_test : public testing::Test
{
public:
};

void *c_callback(size_t, void *[], void *)
{
	result = true;
	return metacall_value_create_long(32L);
}

TEST_F(metacall_node_signal_handler_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	ASSERT_EQ((int)0, (int)metacall_register("c_callback", c_callback, NULL, METACALL_LONG, 0));

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char buffer[] =
			"const { metacall } = require('" METACALL_NODE_PORT_PATH "');\n"
			"const cp = require('child_process');\n"
			"console.log('node: my_function');\n"
			"let sp = cp.spawn('ps');\n"
			"sp.stdout.on('data', data => {\n"
			"	console.log('node: stdout: ' + data.toString());\n"
			"});\n"
			"sp.on('exit', (code, signal) => {\n"
			"	console.log(`node: child process exited with code ${code}`);\n"
			"});\n"
			"process.on('SIGCHLD', () => {\n"
			"	console.log(`node: Received SIGCHLD signal in process`);\n"
			"	metacall('c_callback');\n"
			"});\n";

		EXPECT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char *inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *)NULL, (char *)inspect_str);

		EXPECT_GT((size_t)size, (size_t)0);

		std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	EXPECT_EQ((int)0, (int)metacall_destroy());

	EXPECT_EQ((bool)result.load(), (bool)true);
}
