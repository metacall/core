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
#include <metacall/metacall_loaders.h>

class metacall_callback_complex_test : public testing::Test
{
public:
};

TEST_F(metacall_callback_complex_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		/* This is the equivalent code of this test in NodeJS */
		/*
			const { function_chain, py_function_factorial } = require('function.py');
			const py_py_factorial = function_chain(py_function_factorial);
			assert.notStrictEqual(py_py_factorial, undefined);
			assert.strictEqual(py_py_factorial(5), 120);
		*/

		const char * py_scripts[] =
		{
			"function.py"
		};

		ASSERT_EQ((int) 0, (int) metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		void * py_function_factorial = metacall_function("py_function_factorial");
		ASSERT_NE((void *) NULL, (void *) py_function_factorial);

		void * v_py_function_factorial = metacall_value_create_function(py_function_factorial);
		ASSERT_NE((void *) NULL, (void *) v_py_function_factorial);

		void * args[] = { v_py_function_factorial };
		void * py_py_factorial = metacallv("function_chain", args);
		ASSERT_NE((void *) NULL, (void *) py_py_factorial);

		void * args_fact[] = { metacall_value_create_long(5L) };
		void * ret = metacallfv(metacall_value_to_function(py_py_factorial), args_fact);
		ASSERT_NE((void *) NULL, (void *) ret);
		EXPECT_EQ((long) 120L, (long) metacall_value_to_long(ret));
		metacall_value_destroy(ret);

		ret = metacallfv(metacall_value_to_function(py_py_factorial), args_fact);
		ASSERT_NE((void *) NULL, (void *) ret);
		EXPECT_EQ((long) 120L, (long) metacall_value_to_long(ret));
		metacall_value_destroy(ret);

		metacall_value_destroy(args_fact[0]);

		metacall_value_destroy(v_py_function_factorial);
		metacall_value_destroy(py_py_factorial);
	}
	#endif /* OPTION_BUILD_LOADERS_PY */

	/* NodeJS */
	#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		/* This is the equivalent code of this test in NodeJS */
		/*
			const { js_function_chain, js_function_factorial } = require('factcallback.js');
			const js_js_factorial = js_function_chain(js_function_factorial);
			assert.notStrictEqual(js_js_factorial, undefined);
			assert.strictEqual(js_js_factorial(5), 120);
		*/

		const char * node_scripts[] =
		{
			"factcallback.js"
		};

		ASSERT_EQ((int) 0, (int) metacall_load_from_file("node", node_scripts, sizeof(node_scripts) / sizeof(node_scripts[0]), NULL));

		void * js_function_factorial = metacall_function("js_function_factorial");
		ASSERT_NE((void *) NULL, (void *) js_function_factorial);

		void * v_js_function_factorial = metacall_value_create_function(js_function_factorial);
		ASSERT_NE((void *) NULL, (void *) v_js_function_factorial);
		EXPECT_EQ((enum metacall_value_id) METACALL_FUNCTION, (enum metacall_value_id) metacall_value_id(v_js_function_factorial));

		void * args[] = { v_js_function_factorial };
		void * js_js_factorial = metacallv("js_function_chain", args);
		ASSERT_NE((void *) NULL, (void *) js_js_factorial);
		EXPECT_EQ((enum metacall_value_id) METACALL_FUNCTION, (enum metacall_value_id) metacall_value_id(js_js_factorial));

		void * args_fact[] = { metacall_value_create_long(5L) };
		void * ret = metacallfv(metacall_value_to_function(js_js_factorial), args_fact);
		ASSERT_NE((void *) NULL, (void *) ret);
		EXPECT_EQ((long) 120L, (long) metacall_value_to_long(ret));
		metacall_value_destroy(ret);

		ret = metacallfv(metacall_value_to_function(js_js_factorial), args_fact);
		ASSERT_NE((void *) NULL, (void *) ret);
		EXPECT_EQ((long) 120L, (long) metacall_value_to_long(ret));
		metacall_value_destroy(ret);

		metacall_value_destroy(args_fact[0]);

		metacall_value_destroy(v_js_function_factorial);
		metacall_value_destroy(js_js_factorial);
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
