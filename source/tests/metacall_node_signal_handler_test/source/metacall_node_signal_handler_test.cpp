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

std::atomic_bool callback_result(false);
std::atomic_bool signal_result(false);

class metacall_node_signal_handler_test : public testing::Test
{
public:
};

void *c_callback(size_t, void *[], void *)
{
	callback_result.store(true);
	return metacall_value_create_long(raise(SIGUSR2)); /* Propagate signal */
}

static void handle_signals(int s)
{
	switch (s)
	{
		case SIGUSR1:
			fprintf(stdout, "c++: received signal SIGUSR1\n");
			break;
		case SIGUSR2:
			fprintf(stdout, "c++: received signal SIGUSR1\n");
			signal_result.store(true);
			break;
		case SIGINT:
			fprintf(stdout, "c++: received signal SIGINT\n");
			break;
		case SIGCONT:
			fprintf(stdout, "c++: received signal SIGCONT\n");
			break;
		case SIGCHLD:
			fprintf(stdout, "c++: received signal SIGCHLD\n");
			break;
		case SIGTSTP:
			fprintf(stdout, "c++: received signal SIGTSTP\n");
			break;
		case SIGSTOP:
			fprintf(stdout, "c++: received signal SIGSTOP\n");
			break;
		default:
			fprintf(stdout, "c++: received signal number %d\n", s);
			break;
	}
}

TEST_F(metacall_node_signal_handler_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* Register signal */
	signal(SIGUSR2, handle_signals);

	ASSERT_EQ((int)0, (int)metacall_register("c_callback", c_callback, NULL, METACALL_LONG, 0));

	const char buffer[] =
		"const { metacall } = require('" METACALL_NODE_PORT_PATH "');\n"
		"let notified = false;\n"
		"const notify = () => {\n"
		"	if (notified === true) return;\n"
		"	notified = true;\n"
		"	metacall('c_callback');\n"
		"};\n"
		"const cp = require('child_process');\n"
		"let sp = cp.spawn('ps');\n"
		"sp.stdout.on('data', data => {\n"
		"	console.log('node: stdout: ' + data.toString());\n"
		"});\n"
		"process.on('SIGCHLD', () => {\n"
		"	console.log(`node: Received SIGCHLD signal in process`);\n"
		"	notify()\n"
		"});\n"
		"sp.on('exit', code => {\n"
		"	console.log(`node: child process exited with code ${code}`);\n"
		"	notify();\n" /* Sometimes exit event gets received before SIGCHLD, so we notify anyway to c_callback */
		"});\n";

	EXPECT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

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

	EXPECT_EQ((bool)callback_result.load(), (bool)true);
	EXPECT_EQ((bool)signal_result.load(), (bool)true);
}
