/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall_value.h>
#include <metacall/metacall_loaders.h>

#ifndef METACALL_NODE_REENTRANT_TEST_NODE_PORT_PATH
#	error "The path to the NodeJS port is not defined"
#endif

class metacall_node_reentrant_test : public testing::Test
{
public:
};

TEST_F(metacall_node_reentrant_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* NodeJS */
	#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		/* List of loads (1 level) */
		{
			static const char buffer[] =
				"const { metacall_load_from_memory, metacall } = require('" METACALL_NODE_REENTRANT_TEST_NODE_PORT_PATH "');\n"
				"metacall_load_from_memory('node', 'module.exports = { node_memory0: () => 0 };');\n"
				"metacall_load_from_memory('node', 'module.exports = { node_memory1: () => 1 };');\n"
				"metacall_load_from_memory('node', 'module.exports = { node_memory2: () => 2 };');\n"
				"metacall_load_from_memory('node', 'module.exports = { node_memory3: () => 3 };');\n"
				"metacall_load_from_memory('node', 'module.exports = { node_memory4: () => 4 };');\n"
				"metacall_load_from_memory('node', 'module.exports = { node_memory5: () => 5 };');\n";

			static const char tag[] = "node";

			ASSERT_EQ((int) 0, (int) metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));
		}

		/* Reentrant */
		#if 0
		{
			static const char buffer[] =
				"const { metacall_load_from_memory, metacall } = require('" METACALL_NODE_REENTRANT_TEST_NODE_PORT_PATH "');\n"
				"metacall_load_from_memory('node', 'module.exports = { node_memory: () => 4 };');\n"
				"console.log('Reentrant node_memory result:', metacall('node_memory'));\n";

				/* TODO: This generates a stack overflow in NodeJS land. We should make a better approach, possibly iterative. */
				/*
				"metacall_load_from_memory('node', '"
					"const { metacall } = require(\"" METACALL_NODE_REENTRANT_TEST_NODE_PORT_PATH "\");"
					"console.log(\"Reentrant node_memory result:\", metacall(\"node_memory\"));"
				"');\n";
				*/

			static const char tag[] = "node";

			ASSERT_EQ((int) 0, (int) metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));
		}
		#endif
	}
	#endif /* OPTION_BUILD_LOADERS_NODE */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void * allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char * inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *) NULL, (char *) inspect_str);

		EXPECT_GT((size_t) size, (size_t) 0);

		std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
