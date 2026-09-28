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

class metacall_python_varargs_test : public testing::Test
{
public:
};

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

		/* Test 1: test_args(a, b, *args) -> (1, 2, 3, 4) -> 10 */
		{
			void *args[] = {
				metacall_value_create_long(1),
				metacall_value_create_long(2),
				metacall_value_create_long(3),
				metacall_value_create_long(4)
			};

			void *ret = metacallv_s("test_args", args, 4);

			ASSERT_NE((void *)NULL, ret);
			EXPECT_EQ((long)10, (long)metacall_value_to_long(ret));

			metacall_value_destroy(ret);
			for (size_t i = 0; i < 4; ++i)
			{
				metacall_value_destroy(args[i]);
			}
		}

		/* Test 2: test_kwargs(a, b, **kwargs) -> (1, 2, x=10, y=20) -> 33 */
		{
			void *kwargs_map = metacall_value_create_map(NULL, 0);
			metacall_value_map_set(kwargs_map, "x", metacall_value_create_long(10));
			metacall_value_map_set(kwargs_map, "y", metacall_value_create_long(20));

			void *args[] = {
				metacall_value_create_long(1),
				metacall_value_create_long(2),
				kwargs_map
			};

			void *ret = metacallv_s("test_kwargs", args, 3);

			ASSERT_NE((void *)NULL, ret);
			EXPECT_EQ((long)33, (long)metacall_value_to_long(ret));

			metacall_value_destroy(ret);
			metacall_value_destroy(kwargs_map);
			metacall_value_destroy(args[0]);
			metacall_value_destroy(args[1]);
		}

		/* Test 3: test_varargs(a, b, *args, **kwargs) -> (1, 2, 3, 4, x=10, y=20) -> 40 */
		{
			void *kwargs_map = metacall_value_create_map(NULL, 0);
			metacall_value_map_set(kwargs_map, "x", metacall_value_create_long(10));
			metacall_value_map_set(kwargs_map, "y", metacall_value_create_long(20));

			void *args[] = {
				metacall_value_create_long(1),
				metacall_value_create_long(2),
				metacall_value_create_long(3),
				metacall_value_create_long(4),
				kwargs_map
			};

			void *ret = metacallv_s("test_varargs", args, 5);

			ASSERT_NE((void *)NULL, ret);
			EXPECT_EQ((long)40, (long)metacall_value_to_long(ret));

			metacall_value_destroy(ret);
			metacall_value_destroy(kwargs_map);
			for (size_t i = 0; i < 4; ++i)
			{
				metacall_value_destroy(args[i]);
			}
		}

		/* Test 4: test_pure_args(*args) -> (1, 2, 3) -> 6 */
		{
			void *args[] = {
				metacall_value_create_long(1),
				metacall_value_create_long(2),
				metacall_value_create_long(3)
			};

			void *ret = metacallv_s("test_pure_args", args, 3);

			ASSERT_NE((void *)NULL, ret);
			EXPECT_EQ((long)6, (long)metacall_value_to_long(ret));

			metacall_value_destroy(ret);
			for (size_t i = 0; i < 3; ++i)
			{
				metacall_value_destroy(args[i]);
			}
		}

		/* Test 5: test_pure_kwargs(**kwargs) -> (x=10, y=20) -> 30 */
		{
			void *kwargs_map = metacall_value_create_map(NULL, 0);
			metacall_value_map_set(kwargs_map, "x", metacall_value_create_long(10));
			metacall_value_map_set(kwargs_map, "y", metacall_value_create_long(20));

			void *args[] = {
				kwargs_map
			};

			void *ret = metacallv_s("test_pure_kwargs", args, 1);

			ASSERT_NE((void *)NULL, ret);
			EXPECT_EQ((long)30, (long)metacall_value_to_long(ret));

			metacall_value_destroy(ret);
			metacall_value_destroy(kwargs_map);
		}

		/* Test 6: test_pure_varargs(*args, **kwargs) -> (100, 200, a=1, b=2) -> 303 */
		{
			void *kwargs_map = metacall_value_create_map(NULL, 0);
			metacall_value_map_set(kwargs_map, "a", metacall_value_create_long(1));
			metacall_value_map_set(kwargs_map, "b", metacall_value_create_long(2));

			void *args[] = {
				metacall_value_create_long(100),
				metacall_value_create_long(200),
				kwargs_map
			};

			void *ret = metacallv_s("test_pure_varargs", args, 3);

			ASSERT_NE((void *)NULL, ret);
			EXPECT_EQ((long)303, (long)metacall_value_to_long(ret));

			metacall_value_destroy(ret);
			metacall_value_destroy(kwargs_map);
			metacall_value_destroy(args[0]);
			metacall_value_destroy(args[1]);
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