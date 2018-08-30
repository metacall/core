/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

#include <memory/memory.h>

#include <log/log.h>

class metacall_load_configuration_test : public testing::Test
{
public:
};

TEST_F(metacall_load_configuration_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	memory_allocator allocator = memory_allocator_std(&std::malloc, &std::realloc, &std::free);

	ASSERT_NE((memory_allocator) NULL, (memory_allocator) allocator);

	/* Python */
	#if defined(OPTION_BUILD_LOADERS_PY)
	{
		const long seven_multiples_limit = 10;

		long iterator;

		void * ret = NULL;

		ASSERT_EQ((int) 0, (int) metacall_load_from_configuration("metacall_load_from_configuration_py_test_a.json", NULL, allocator));

		ret = metacall("multiply", 5, 15);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_to_long(ret), (long) 75);

		metacall_value_destroy(ret);

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			ret = metacall("multiply", 7, iterator);

			EXPECT_NE((void *) NULL, (void *) ret);

			EXPECT_EQ((long) metacall_value_to_long(ret), (long) (7 * iterator));

			metacall_value_destroy(ret);
		}

		ret = metacall("divide", 64.0, 2.0);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_to_double(ret), (double) 32.0);

		metacall_value_destroy(ret);

		ret = metacall("sum", 1000, 3500);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_to_long(ret), (long) 4500);

		metacall_value_destroy(ret);

		ret = metacall("sum", 3, 4);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_to_long(ret), (long) 7);

		metacall_value_destroy(ret);

		EXPECT_EQ((void *) NULL, (void *)metacall("hello"));

		ret = metacall("strcat", "Hello ", "Universe");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 0, (int) strcmp(metacall_value_to_string(ret), "Hello Universe"));

		metacall_value_destroy(ret);

		ASSERT_EQ((int) 0, (int) metacall_load_from_configuration("metacall_load_from_configuration_py_test_b.json", NULL, allocator));

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

		ret = metacall("s_multiply", 5, 15);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_to_long(ret), (long) 75);

		metacall_value_destroy(ret);

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			ret = metacall("s_multiply", 7, iterator);

			EXPECT_NE((void *) NULL, (void *) ret);

			EXPECT_EQ((long) metacall_value_to_long(ret), (long) (7 * iterator));

			metacall_value_destroy(ret);
		}

		ret = metacall("s_divide", 64.0, 2.0);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_to_double(ret), (double) 32.0);

		metacall_value_destroy(ret);

		ret = metacall("s_sum", 1000, 3500);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((long)metacall_value_to_long(ret), (long)4500);

		metacall_value_destroy(ret);

		ret = metacall("s_sum", 3, 4);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((long)metacall_value_to_long(ret), (long)7);

		metacall_value_destroy(ret);

		EXPECT_EQ((void *)NULL, (void *)metacall("s_hello"));

		ret = metacall("s_strcat", "Hello ", "Universe");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)0, (int)strcmp(metacall_value_to_string(ret), "Hello Universe"));

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_LOADERS_PY */

	/* Ruby */
	#if defined(OPTION_BUILD_LOADERS_RB)
	{
		void * ret = NULL;

		ASSERT_EQ((int) 0, (int) metacall_load_from_configuration("metacall_load_from_configuration_rb_test.json", NULL, allocator));

		ret = metacall("say_multiply", 5, 7);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) metacall_value_to_int(ret), (int) 35);

		metacall_value_destroy(ret);

		EXPECT_EQ((void *) NULL, (void *) metacall("say_null"));

		ret = metacall("say_hello", "meta-programmer");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 0, (int) strcmp(metacall_value_to_string(ret), "Hello meta-programmer!"));

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_LOADERS_RB */

	memory_allocator_destroy(allocator);

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
