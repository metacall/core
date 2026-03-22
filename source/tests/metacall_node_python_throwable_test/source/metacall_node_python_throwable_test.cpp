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

class metacall_node_python_throwable_test : public testing::Test
{
public:
};

TEST_F(metacall_node_python_throwable_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* NodeJS & Python */
#if defined(OPTION_BUILD_LOADERS_NODE) && defined(OPTION_BUILD_LOADERS_PY)
	{
		/*
		 * Load a Node script that exports `flip`: a higher-order function that
		 * takes a two-argument function `fn` and returns a new function that
		 * calls `fn` with its arguments swapped.
		 *
		 * This replicates the derpyramda.js `flip` function used in the
		 * original failing Python test (test.py) to validate for now and make sure
		 * This doesn't happen in future
		 */
		static const char node_buffer[] =
			"function flip(fn) {\n"
			"    return function(x, y) {\n"
			"        return fn(y, x);\n"
			"    };\n"
			"}\n"
			"module.exports = { flip };\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", node_buffer, sizeof(node_buffer), NULL));

		/*
		 * Load a Python script that:
		 *   1. Imports the metacall Python port to call cross-language functions.
		 *   2. Defines test_flip_returns_correct_value(): passes a Python lambda
		 *      to Node's `flip`, then calls the returned closure with (5.0, 4.0).
		 *      Expected result: flip swaps arguments → lambda(4.0, 5.0) = 4.0 - 5.0 = -1.0
		 *
		 * Before the fix in py_loader_impl.c, calling this returned closure
		 * would cause a SEGFAULT on Python 3.14+ because the return value
		 * could arrive as TYPE_THROWABLE (id=18) and py_loader_impl_value_to_capi()
		 * did not handle that type - it returned NULL without setting a Python
		 * exception, which Python 3.14 treats as a fatal error.
		 *
		 * After the fix, TYPE_THROWABLE is properly converted to a Python
		 * RuntimeError exception instead of a NULL-without-exception crash.
		 */
		static const char py_buffer[] =
			"import sys\n"
			"sys.path.insert(0, '" METACALL_PYTHON_PORT_PATH "')\n"
			"from metacall import metacall, metacall_load_from_memory\n"
			"\n"
			"def test_flip_returns_correct_value():\n"
			"    flip_fn = metacall('flip', lambda x, y: x - y)\n"
			"    result = flip_fn(5.0, 4.0)\n"
			"    return result\n"
			"\n"
			"def test_flip_with_throwing_callback():\n"
			"    '''\n"
			"    Verify that when the callback passed to flip raises a Python exception,\n"
			"    the error propagates back to the caller as a Python exception\n"
			"    (not a SEGFAULT).\n"
			"    '''\n"
			"    def throwing_cb(x, y):\n"
			"        raise ValueError('intentional error from callback')\n"
			"    try:\n"
			"        flip_fn = metacall('flip', throwing_cb)\n"
			"        result = flip_fn(5.0, 4.0)\n"
			"        return False\n"
			"    except Exception:\n"
			"        return True\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("py", py_buffer, sizeof(py_buffer), NULL));

		/* Test 1: flip(lambda x, y: x - y)(5.0, 4.0) should return -1.0
		 * This is the exact scenario from the original failing test.py.
		 * Before the fix, this would SEGFAULT on Python 3.14+ because the
		 * return value arrived as TYPE_THROWABLE and py_loader_impl_value_to_capi
		 * returned NULL without setting a Python exception. */
		{
			void *ret = metacall("test_flip_returns_correct_value");

			ASSERT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(ret));

			EXPECT_EQ((double)-1.0, (double)metacall_value_to_double(ret));

			metacall_value_destroy(ret);
		}

		/* Test 2: flip with a throwing callback should not SEGFAULT and should
		 * propagate the exception back to the caller cleanly.
		 * The Python function returns True if an exception was raised and caught,
		 * False if the call unexpectedly succeeded. */
		{
			void *ret = metacall("test_flip_with_throwing_callback");

			ASSERT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((enum metacall_value_id)METACALL_BOOL, (enum metacall_value_id)metacall_value_id(ret));

			/* The Python function returns True when the exception was caught */
			EXPECT_NE((int)0, (int)metacall_value_to_bool(ret));

			metacall_value_destroy(ret);
		}
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}
