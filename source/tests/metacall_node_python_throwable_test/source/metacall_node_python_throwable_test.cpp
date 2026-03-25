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

#include <cstring>

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
		 * Load a Node script that exports:
		 *   - flip: higher-order function (swaps arg order) - replicates derpyramda.js flip
		 *     used in the original failing Python test (test.py)
		 *   - throwing_node_fn: function that directly throws a JS Error, used to test
		 *     that py_loader_port_invoke (the metacall() Python port) correctly propagates
		 *     the thrown TYPE_THROWABLE as a Python exception rather than returning Py_None
		 *     with a pending exception (which causes a fatal error in Python 3.14+).
		 */
		static const char node_buffer[] =
			"function flip(fn) {\n"
			"    return function(x, y) {\n"
			"        return fn(y, x);\n"
			"    };\n"
			"}\n"
			"function throwing_node_fn() {\n"
			"    throw new Error('test error from node');\n"
			"}\n"
			"module.exports = { flip, throwing_node_fn };\n";

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", node_buffer, sizeof(node_buffer), NULL));

		/*
		 * Load a Python script that defines four test functions exercising the
		 * cross-language throwable propagation fixes:
		 *
		 *   test_flip_returns_correct_value - the original crash scenario (test.py):
		 *     flip(lambda)(5.0, 4.0) must return -1.0.  Before the fix this SEGFAULT'd
		 *     on Python 3.14+ because the return value arrived as TYPE_THROWABLE and
		 *     py_loader_impl_value_to_capi returned NULL without setting a Python
		 *     exception, which Python 3.14 treats as a fatal error.
		 *
		 *   test_flip_with_throwing_callback - validates py_loader_func.c:
		 *     When the Python callback passed to flip raises, the error must propagate
		 *     back as a Python exception (not a SEGFAULT / silent None).
		 *
		 *   test_direct_node_throw_raises_exception - validates py_loader_port_invoke:
		 *     A Node function that directly throws must surface as a Python RuntimeError
		 *     when called via metacall().  Before the fix py_loader_port_invoke returned
		 *     Py_None with a pending exception - a Python 3.14 fatal contract violation.
		 *
		 *   test_node_error_has_stacktrace - validates the stacktrace field is included
		 *     in the RuntimeError message.  node_loader populates exception_stacktrace
		 *     with Error.stack; the new py_loader_impl_value_to_capi handler appends it.
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
			"    def throwing_cb(x, y):\n"
			"        raise ValueError('intentional error from callback')\n"
			"    try:\n"
			"        flip_fn = metacall('flip', throwing_cb)\n"
			"        result = flip_fn(5.0, 4.0)\n"
			"        return False\n"
			"    except Exception:\n"
			"        return True\n"
			"\n"
			"def test_direct_node_throw_raises_exception():\n"
			"    try:\n"
			"        metacall('throwing_node_fn')\n"
			"        return False\n"
			"    except Exception:\n"
			"        return True\n"
			"\n"
			"def test_node_error_has_stacktrace():\n"
			"    try:\n"
			"        metacall('throwing_node_fn')\n"
			"        return ''\n"
			"    except Exception as e:\n"
			"        return str(e)\n";

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

		/* Test 3: a Node function that directly throws must surface as a Python
		 * exception when called via metacall() from the Python port.
		 * Validates the py_loader_port_invoke fix: before the fix, py_loader_port_invoke
		 * returned Py_None with a pending PyErr - a fatal contract violation in Python
		 * 3.14+ ("a function returned a result with an exception set").
		 * After the fix, NULL is returned and Python sees a clean RuntimeError. */
		{
			void *ret = metacall("test_direct_node_throw_raises_exception");

			ASSERT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((enum metacall_value_id)METACALL_BOOL, (enum metacall_value_id)metacall_value_id(ret));

			EXPECT_NE((int)0, (int)metacall_value_to_bool(ret));

			metacall_value_destroy(ret);
		}

		/* Test 4: the JS Error.stack (stacktrace) must appear in the RuntimeError
		 * message raised in Python.  node_loader populates exception_stacktrace with
		 * Error.stack; the updated TYPE_THROWABLE handler appends it after the message.
		 * The Python function returns the exception string; we verify it contains the
		 * expected error message text. */
		{
			void *ret = metacall("test_node_error_has_stacktrace");

			ASSERT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((enum metacall_value_id)METACALL_STRING, (enum metacall_value_id)metacall_value_id(ret));

			const char *msg = metacall_value_to_string(ret);

			EXPECT_NE((void *)NULL, (void *)msg);

			/* The error message must contain the text thrown from JS */
			EXPECT_NE((void *)NULL, (void *)strstr(msg, "test error from node"));

			metacall_value_destroy(ret);
		}
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}
