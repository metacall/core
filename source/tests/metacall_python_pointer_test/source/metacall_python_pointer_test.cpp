/*
 *	Loader Library by Parra Studios
 *	A plugin for loading python code at run-time into a process.
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

class metacall_python_pointer_test : public testing::Test
{
public:
};

struct test_type
{
	long value;
	unsigned char r, g, b;
};

void *native_set_value(size_t argc, void *args[], void *data)
{
	struct test_type *t = (struct test_type *)metacall_value_to_ptr(args[0]);
	long value = metacall_value_to_long(args[1]);

	(void)argc;
	(void)data;

	printf("Pointer: %p\n", (void *)t);
	printf("Value: %ld\n", value);

	t->value = value;

	t->r = 10U;
	t->g = 50U;
	t->b = 70U;

	return metacall_value_create_ptr((void *)t);
}

void *native_get_value(size_t argc, void *args[], void *data)
{
	struct test_type *t = (struct test_type *)metacall_value_to_ptr(args[0]);

	const void *array[] = {
		metacall_value_create_char(t->r),
		metacall_value_create_char(t->g),
		metacall_value_create_char(t->b)
	};

	const size_t size = sizeof(array) / sizeof(array[0]);

	(void)argc;
	(void)data;

	printf("Array Size: %zu\n", size);

	return metacall_value_create_array(array, size);
}

void *native_ret_null_ptr(size_t argc, void *args[], void *data)
{
	void *ptr = metacall_value_to_ptr(args[0]);

	EXPECT_EQ((void *)ptr, (void *)NULL);

	(void)argc;
	(void)data;

	return metacall_value_create_ptr(NULL);
}

TEST_F(metacall_python_pointer_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* Native register */
	{
		metacall_register("native_set_value", native_set_value, NULL, METACALL_PTR, 2, METACALL_PTR, METACALL_LONG);

		EXPECT_NE((void *)NULL, (void *)metacall_function("native_set_value"));

		metacall_register("native_get_value", native_get_value, NULL, METACALL_ARRAY, 1, METACALL_PTR);

		EXPECT_NE((void *)NULL, (void *)metacall_function("native_get_value"));

		metacall_register("native_ret_null_ptr", native_ret_null_ptr, NULL, METACALL_PTR, 0);

		EXPECT_NE((void *)NULL, (void *)metacall_function("native_ret_null_ptr"));
	}

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const char *py_scripts[] = {
			"pointer.py"
		};

		void *ret = NULL;

		struct test_type t = { 0L, 0U, 0U, 0U };

		void *t_ptr = (void *)&t;

		long value = 3000L;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("py", py_scripts, sizeof(py_scripts) / sizeof(py_scripts[0]), NULL));

		const enum metacall_value_id ids[] = {
			METACALL_PTR, METACALL_LONG
		};

		ret = metacallt("python_set_value", ids, t_ptr, value);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_PTR, (enum metacall_value_id)metacall_value_id(ret));

		EXPECT_EQ((void *)t_ptr, (void *)metacall_value_to_ptr(ret));

		EXPECT_EQ((long)value, (long)t.value);
		EXPECT_EQ((unsigned char)10U, (unsigned char)t.r);
		EXPECT_EQ((unsigned char)50U, (unsigned char)t.g);
		EXPECT_EQ((unsigned char)70U, (unsigned char)t.b);

		metacall_value_destroy(ret);

		void *args[] = {
			metacall_value_create_ptr(NULL)
		};

		ret = metacallv("python_ret_null", args);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_PTR, (enum metacall_value_id)metacall_value_id(ret));

		EXPECT_EQ((void *)NULL, (void *)metacall_value_to_ptr(ret));

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}
#endif /* OPTION_BUILD_LOADERS_PY */

	metacall_destroy();
}
