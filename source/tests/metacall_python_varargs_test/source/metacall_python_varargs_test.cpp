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

#include <cstring>

class metacall_python_varargs_test : public testing::Test
{
public:
};

#if defined(OPTION_BUILD_LOADERS_PY)
static void *metacall_create_test_map(const char *k1, long v1, const char *k2, long v2)
{
	void *map = metacall_value_create_map(NULL, 2);
	void **tuples = metacall_value_to_map(map);

	tuples[0] = metacall_value_create_array(NULL, 2);
	void **t0 = metacall_value_to_array(tuples[0]);
	t0[0] = metacall_value_create_string(k1, strlen(k1));
	t0[1] = metacall_value_create_long(v1);

	tuples[1] = metacall_value_create_array(NULL, 2);
	void **t1 = metacall_value_to_array(tuples[1]);
	t1[0] = metacall_value_create_string(k2, strlen(k2));
	t1[1] = metacall_value_create_long(v2);

	return map;
}
#endif /* OPTION_BUILD_LOADERS_PY */

TEST_F(metacall_python_varargs_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char *py_scripts[] = {
			"varargs.py"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		/* Test 1: test_args(*args) -> (1, 2, 3) -> 6 */
		{
			void *args[] = {
				metacall_value_create_long(1),
				metacall_value_create_long(2),
				metacall_value_create_long(3)
			};

			void *ret = metacallv_s("test_args", args, sizeof(args) / sizeof(args[0]));

			ASSERT_NE((void *)NULL, ret);
			EXPECT_EQ((long)6, (long)metacall_value_to_long(ret));

			metacall_value_destroy(ret);

			for (size_t i = 0; i < sizeof(args) / sizeof(args[0]); ++i)
			{
				metacall_value_destroy(args[i]);
			}
		}

		/* Test 2: test_kwargs(**kwargs) -> (x=10, y=20) -> 30 */
		{
			void *args[] = {
				metacall_create_test_map("x", 10, "y", 20)
			};

			void *ret = metacallv_s("test_kwargs", args, sizeof(args) / sizeof(args[0]));

			ASSERT_NE((void *)NULL, ret);
			EXPECT_EQ((long)30, (long)metacall_value_to_long(ret));

			metacall_value_destroy(ret);

			for (size_t i = 0; i < sizeof(args) / sizeof(args[0]); ++i)
			{
				metacall_value_destroy(args[i]);
			}
		}

		/* Test 3: test_varargs(*args, **kwargs) -> (100, 200, a=1, b=2) -> 303 */
		{
			void *args[] = {
				metacall_value_create_long(100),
				metacall_value_create_long(200),
				metacall_create_test_map("a", 1, "b", 2)
			};

			void *ret = metacallv_s("test_varargs", args, sizeof(args) / sizeof(args[0]));

			ASSERT_NE((void *)NULL, ret);
			EXPECT_EQ((long)303, (long)metacall_value_to_long(ret));

			metacall_value_destroy(ret);

			for (size_t i = 0; i < sizeof(args) / sizeof(args[0]); ++i)
			{
				metacall_value_destroy(args[i]);
			}
		}

		/* Test 4: test_mixed_args(a, b, *args) -> (1, 2, 3, 4) -> 10 */
		{
			void *args[] = {
				metacall_value_create_long(1),
				metacall_value_create_long(2),
				metacall_value_create_long(3),
				metacall_value_create_long(4)
			};

			void *ret = metacallv_s("test_mixed_args", args, sizeof(args) / sizeof(args[0]));

			ASSERT_NE((void *)NULL, ret);
			EXPECT_EQ((long)10, (long)metacall_value_to_long(ret));

			metacall_value_destroy(ret);

			for (size_t i = 0; i < sizeof(args) / sizeof(args[0]); ++i)
			{
				metacall_value_destroy(args[i]);
			}
		}

		/* Test 5: test_mixed_kwargs(a, b, **kwargs) -> (1, 2, x=10, y=20) -> 33 */
		{
			void *args[] = {
				metacall_value_create_long(1),
				metacall_value_create_long(2),
				metacall_create_test_map("x", 10, "y", 20)
			};

			void *ret = metacallv_s("test_mixed_kwargs", args, sizeof(args) / sizeof(args[0]));

			ASSERT_NE((void *)NULL, ret);
			EXPECT_EQ((long)33, (long)metacall_value_to_long(ret));

			metacall_value_destroy(ret);

			for (size_t i = 0; i < sizeof(args) / sizeof(args[0]); ++i)
			{
				metacall_value_destroy(args[i]);
			}
		}

		/* Test 6: test_mixed(a, b, *args, **kwargs) -> (10, 20, 30, 40, key=50, other=10) -> 160 */
		{
			void *args[] = {
				metacall_value_create_long(10),
				metacall_value_create_long(20),
				metacall_value_create_long(30),
				metacall_value_create_long(40),
				metacall_create_test_map("key", 50, "other", 10)
			};

			void *ret = metacallv_s("test_mixed", args, sizeof(args) / sizeof(args[0]));

			ASSERT_NE((void *)NULL, ret);
			EXPECT_EQ((long)160, (long)metacall_value_to_long(ret));

			metacall_value_destroy(ret);

			for (size_t i = 0; i < sizeof(args) / sizeof(args[0]); ++i)
			{
				metacall_value_destroy(args[i]);
			}
		}
	}
#endif /* OPTION_BUILD_LOADERS_PY */

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