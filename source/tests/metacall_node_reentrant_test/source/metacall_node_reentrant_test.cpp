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

#ifndef METACALL_NODE_REENTRANT_TEST_NODE_PORT_PATH
	#error "The path to the NodeJS port is not defined"
#endif

class metacall_node_reentrant_test : public testing::Test
{
public:
};

TEST_F(metacall_node_reentrant_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

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

			ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));
		}

		/* Reentrant */
		{
			static const char buffer[] =
				"const { metacall_load_from_memory, metacall } = require('" METACALL_NODE_REENTRANT_TEST_NODE_PORT_PATH "');\n"
				"metacall_load_from_memory('node', 'module.exports = { node_memory_reentrant: () => 4 };');\n"
				"console.log('Reentrant node_memory_reentrant result (main):', metacall('node_memory_reentrant'));\n"
				"metacall_load_from_memory('node', '"
				"const { metacall } = require(\"" METACALL_NODE_REENTRANT_TEST_NODE_PORT_PATH "\");"
				"console.log(\"Reentrant node_memory_reentrant result (reentrant):\", metacall(\"node_memory_reentrant\"));"
				"');\n";

			static const char tag[] = "node";

			ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));
		}

		/* Recursion */
		{
			static const char buffer[] =
				"const { metacall } = require('" METACALL_NODE_REENTRANT_TEST_NODE_PORT_PATH "');\n"
				"const log = (v) => { console.log(v); return v };"
				"module.exports = { node_memory_recursive: (v) => (v > 0 && v < 100000000000) ? metacall('node_memory_recursive', log(v * 2)) : log(v) };\n";

			static const char tag[] = "node";

			ASSERT_EQ((int)0, (int)metacall_load_from_memory(tag, buffer, sizeof(buffer), NULL));

			const enum metacall_value_id node_memory_recursive_ids[] = {
				METACALL_INT
			};

			void *ret = metacallt("node_memory_recursive", node_memory_recursive_ids, 1);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((double)137438953472.0, (double)metacall_value_to_double(ret));
		}
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

	metacall_destroy();
}
