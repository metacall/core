/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

	/* File with dependencies */
	const char *c_dep_scripts[] = {
		"ffi.c",
		"ffi.ld"
	};

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

	ret = metacallv_s("callback", args, 1);

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
}
