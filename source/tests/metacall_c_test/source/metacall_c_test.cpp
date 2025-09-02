/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
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

class metacall_c_test : public testing::Test
{
protected:
};

void *sum_callback(size_t argc, void *args[], void *data)
{
	int a = metacall_value_to_int(args[0]);
	int b = metacall_value_to_int(args[1]);
	int result = a + b;

	(void)argc;
	(void)data;

	printf("sum(%d, %d) == %d\n", a, b, result);

	return metacall_value_create_int(result);
}

void *test_string_reference(size_t argc, void *args[], void *data)
{
	printf("ptr %p\n", args[0]);
	fflush(stdout);

	void *string_value = metacall_value_to_ptr(args[0]);

	printf("string ptr %p\n", string_value);
	printf("type id %s\n", metacall_value_type_name(string_value));
	fflush(stdout);

	char *str = metacall_value_to_string(string_value);

	(void)argc;
	(void)data;

	printf("native string %s\n", str);

	EXPECT_STREQ("asd", str);

	static const char yeet[] = "yeet";

	metacall_value_from_string(string_value, yeet, sizeof(yeet) - 1);

	printf("type id %s\n", metacall_value_type_name(string_value));
	printf("native string %s\n", str);
	fflush(stdout);

	return metacall_value_create_null();
}

TEST_F(metacall_c_test, DefaultConstructor)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* File */
	const char *c_scripts[] = {
		"compiled.c"
	};

	EXPECT_EQ((int)0, (int)metacall_load_from_file("c", c_scripts, sizeof(c_scripts) / sizeof(c_scripts[0]), NULL));

	void *ret = metacall("compiled_sum", 3, 4);

	EXPECT_NE((void *)NULL, (void *)ret);

	EXPECT_EQ((long)metacall_value_to_long(ret), (long)7);

	metacall_value_destroy(ret);

	/* https://github.com/metacall/core/issues/570 */
	{
		/* void apply_blur_filter(int pixels[], int width, int height) */

		/* Call by array */
		{
			void *args[] = {
				metacall_value_create_array(NULL, 100),
				metacall_value_create_int(10),
				metacall_value_create_int(10)
			};

			void **array_ptr = metacall_value_to_array(args[0]);

			for (int i = 0; i < 100; ++i)
			{
				array_ptr[i] = metacall_value_create_int(i);
			}

			std::cout << "value: " << args[0] << std::endl;
			std::cout << "array: " << array_ptr << std::endl;

			ret = metacallv("apply_blur_filter", args);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_NULL);

			metacall_value_destroy(ret);

			metacall_value_destroy(args[0]);
			metacall_value_destroy(args[1]);
			metacall_value_destroy(args[2]);
		}

		/* Call by pointer */
		{
			int array[100];

			void *args[] = {
				metacall_value_create_ptr(array),
				metacall_value_create_int(10),
				metacall_value_create_int(10)
			};

			for (int i = 0; i < 100; ++i)
			{
				array[i] = i;
			}

			ret = metacallv("apply_blur_filter", args);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_NULL);

			metacall_value_destroy(ret);

			metacall_value_destroy(args[0]);
			metacall_value_destroy(args[1]);
			metacall_value_destroy(args[2]);
		}

		/* double calculate_brightness(int pixels[], int size) */

		/* Call by array */
		{
			void *args[] = {
				metacall_value_create_array(NULL, 100),
				metacall_value_create_int(100)
			};

			void **array_ptr = metacall_value_to_array(args[0]);

			for (int i = 0; i < 100; ++i)
			{
				array_ptr[i] = metacall_value_create_int(i);
			}

			std::cout << "value: " << args[0] << std::endl;
			std::cout << "array: " << array_ptr << std::endl;

			ret = metacallv("calculate_brightness", args);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_DOUBLE);

			std::cout << "result: " << metacall_value_to_double(ret) << std::endl;

			EXPECT_EQ((double)metacall_value_to_double(ret), (double)49.5);

			metacall_value_destroy(ret);

			metacall_value_destroy(args[0]);
			metacall_value_destroy(args[1]);
		}

		/* Call by pointer */
		{
			int array[100];

			void *args[] = {
				metacall_value_create_ptr(array),
				metacall_value_create_int(100)
			};

			for (int i = 0; i < 100; ++i)
			{
				array[i] = i;
			}

			ret = metacallv("calculate_brightness", args);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_DOUBLE);

			std::cout << "result: " << metacall_value_to_double(ret) << std::endl;

			metacall_value_destroy(ret);

			metacall_value_destroy(args[0]);
			metacall_value_destroy(args[1]);
		}
	}

	/* File with dependencies */
	const char *c_dep_scripts[] = {
		"ffi.c",
		"ffi.ld"
	};

	/* Set dependency paths */
	EXPECT_EQ((int)0, (int)metacall_execution_path("c", LIBFFI_INCLUDE_DIR));
	EXPECT_EQ((int)0, (int)metacall_execution_path("c", LIBFFI_LIBRARY));

	EXPECT_EQ((int)0, (int)metacall_load_from_file("c", c_dep_scripts, sizeof(c_dep_scripts) / sizeof(c_dep_scripts[0]), NULL));

	ret = metacall("call_fp_address");

	EXPECT_NE((void *)NULL, (void *)ret);

	EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_PTR);

	EXPECT_NE((void *)metacall_value_to_ptr(ret), (void *)NULL);

	metacall_value_destroy(ret);

	ret = metacall("int_type_renaming");

	EXPECT_NE((void *)NULL, (void *)ret);

	EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_INT);

	EXPECT_EQ((int)metacall_value_to_int(ret), (int)345);

	metacall_value_destroy(ret);

	/* Native register */
	metacall_register("sum_callback", sum_callback, NULL, METACALL_INT, 2, METACALL_INT, METACALL_INT);

	void *func = metacall_function("sum_callback");

	EXPECT_NE((void *)NULL, (void *)func);

	void *args[] = {
		metacall_value_create_function(func)
	};

	ret = metacallv_s("c_callback", args, 1);

	EXPECT_NE((void *)NULL, (void *)ret);

	EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_INT);

	EXPECT_EQ((int)metacall_value_to_int(ret), (int)7);

	metacall_value_destroy(ret);

	metacall_value_destroy(args[0]);

	/* Memory */
	// TODO
	// const char c_buffer[] = {
	// 	"int compiled_mult(int a, int b) { return a * b; }"
	// };

	// EXPECT_EQ((int)0, (int)metacall_load_from_memory("c", c_buffer, sizeof(c_buffer), NULL));

	// TODO
	// void *ret = metacall("compiled_mult", 3, 4);

	// EXPECT_NE((void *)NULL, (void *)ret);

	// EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

	// metacall_value_destroy(ret);

	/* References (Native) */
	{
		static const char str[] = "asd";
		void *str_value = metacall_value_create_string(str, sizeof(str) - 1);
		void *str_value_ref = metacall_value_reference(str_value);

		printf("ptr %p\n", str_value_ref);
		printf("string %p\n", str_value);
		printf("string str %s\n", metacall_value_to_string(str_value));
		fflush(stdout);

		{
			void *new_str_value = metacall_value_to_ptr(str_value_ref);
			char *new_str = metacall_value_to_string(new_str_value);

			EXPECT_STREQ("asd", new_str);
		}

		void *args[] = {
			str_value_ref
		};

		metacall_register("test_string_reference", test_string_reference, NULL, METACALL_NULL, 1, METACALL_PTR);

		ret = metacallv_s("test_string_reference", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_NULL);

		metacall_value_destroy(ret);

		printf("type id %s\n", metacall_value_type_name(str_value));
		fflush(stdout);

		// It chops the string because it has a fixed size from 'asd'
		EXPECT_STREQ(metacall_value_to_string(str_value), "yee");

		metacall_value_destroy(str_value);
		metacall_value_destroy(str_value_ref);
	}

	/* References (C) */
	{
		static const char str[] = "asd";
		void *str_value = metacall_value_create_string(str, sizeof(str) - 1);
		void *str_value_ref = metacall_value_reference(str_value);

		printf("(R) ptr %p\n", str_value_ref);
		printf("(R) string ptr %p\n", str_value);
		printf("(R) string str %s\n", metacall_value_to_string(str_value));
		fflush(stdout);

		void *args[] = {
			str_value_ref
		};

		ret = metacallv_s("modify_str_ptr", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_NULL);

		metacall_value_destroy(ret);

		char *str_value_deref = static_cast<char *>(metacall_value_dereference(str_value_ref));

		EXPECT_STREQ(str_value_deref, "yeet");

		metacall_value_destroy(str_value);
		metacall_value_destroy(str_value_ref);
	}

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
