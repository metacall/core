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

#include <cstdio>

#include <atomic>

std::atomic<int> success_callbacks{};

class metacall_map_await_test : public testing::Test
{
public:
};

static void *hello_boy_await_ok(void *result, void *data)
{
	double *it = static_cast<double *>(data);

	EXPECT_NE((void *)NULL, (void *)result);

	EXPECT_NE((void *)NULL, (void *)data);

	EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_DOUBLE);

	printf("hello_boy_await future (from map) callback: %f\n", metacall_value_to_double(result));

	fflush(stdout);

	EXPECT_EQ((double)metacall_value_to_double(result), (double)(7.0 + *it));

	delete it;

	++success_callbacks;

	return NULL;
}

static void *hello_boy_await_fail(void *, void *data)
{
	double *it = static_cast<double *>(data);

	int this_should_never_happen = 1;

	EXPECT_NE((void *)NULL, (void *)data);

	delete it;

	EXPECT_NE((int)0, (int)this_should_never_happen);

	return NULL;
}

static void *hello_world_await_fail(void *, void *data)
{
	int this_should_never_happen = 1;

	EXPECT_EQ((void *)NULL, (void *)data);

	EXPECT_NE((int)0, (int)this_should_never_happen);

	return NULL;
}

static void *hello_world_await_ok(void *result, void *data)
{
	EXPECT_NE((void *)NULL, (void *)result);

	EXPECT_EQ((void *)NULL, (void *)data);

	EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_STRING);

	printf("hello_world callback: %s\n", metacall_value_to_string(result));

	fflush(stdout);

	EXPECT_EQ((int)0, (int)strcmp(metacall_value_to_string(result), "Hello World"));

	++success_callbacks;

	return NULL;
}

TEST_F(metacall_map_await_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

	void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char *node_scripts[] = {
			"nod.js"
		};

		void *ret = NULL;

		EXPECT_EQ((int)0, (int)metacall_load_from_file("node", node_scripts, sizeof(node_scripts) / sizeof(node_scripts[0]), NULL));

		static const char left[] = "a";
		static const char right[] = "b";

		void *keys[] = {
			metacall_value_create_string(left, sizeof(left) - 1),
			metacall_value_create_string(right, sizeof(right) - 1)
		};

		void *values[] = {
			metacall_value_create_double(7.0),
			metacall_value_create_double(0.0)
		};

		static const char args_map[] = "{\"a\":10,\"b\":2}";

		void *func = metacall_function("hello_boy_await");

		ASSERT_NE((void *)NULL, (void *)func);

		/* Call by map using arrays */
		for (double iterator = 0.0; iterator <= 10.0; iterator += 1.0)
		{
			double *context = new double(iterator);

			values[1] = metacall_value_from_double(values[1], iterator);

			ret = metacallfmv_await(func, keys, values, hello_boy_await_ok, hello_boy_await_fail, (void *)context);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_FUTURE);

			metacall_value_destroy(ret);
		}

		/* Call by map using serial */
		ret = metacallfms_await(
			func, args_map, sizeof(args_map), allocator, [](void *result, void *) -> void * {
				EXPECT_NE((void *)NULL, (void *)result);

				EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_DOUBLE);

				EXPECT_EQ((double)metacall_value_to_double(result), (double)12.0);

				printf("hello_boy_await future (from map serial) callback: %f\n", metacall_value_to_double(result));

				fflush(stdout);

				++success_callbacks;

				return NULL;
			},
			NULL, NULL);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_FUTURE);

		metacall_value_destroy(ret);

		/* Call by map using serial (segmentation fault on vaule destroy) */
		static const char args_random_map[] = "{\"token\":\"abc\",\"serial\":[\"XYZ\",\"ABC\"],\"range\":\"eee\"}";

		func = metacall_function("get_random_data");

		ASSERT_NE((void *)NULL, (void *)func);

		ret = metacallfms_await(
			func, args_random_map, sizeof(args_random_map), allocator, [](void *result, void *) -> void * {
				EXPECT_NE((void *)NULL, (void *)result);

				EXPECT_EQ((enum metacall_value_id)metacall_value_id(result), (enum metacall_value_id)METACALL_DOUBLE);

				EXPECT_EQ((double)metacall_value_to_double(result), (double)12.0);

				printf("get_random_data future (from map serial) callback: %f\n", metacall_value_to_double(result));

				fflush(stdout);

				++success_callbacks;

				return NULL;
			},
			NULL, NULL);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_FUTURE);

		metacall_value_destroy(ret);

		/* Call by map using arrays (nested await) */
		func = metacall_function("hello_boy_nested_await");

		ASSERT_NE((void *)NULL, (void *)func);

		for (double iterator = 0.0; iterator <= 10.0; iterator += 1.0)
		{
			double *context = new double(iterator);

			values[1] = metacall_value_from_double(values[1], iterator);

			ret = metacallfmv_await(func, keys, values, hello_boy_await_ok, hello_boy_await_fail, (void *)context);

			EXPECT_NE((void *)NULL, (void *)ret);

			EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_FUTURE);

			metacall_value_destroy(ret);
		}

		metacall_value_destroy(keys[0]);
		metacall_value_destroy(keys[1]);

		metacall_value_destroy(values[0]);
		metacall_value_destroy(values[1]);

		/* Await function that throws */
		ret = metacall_await("throw_await", metacall_null_args, hello_world_await_fail, hello_world_await_ok, NULL);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_FUTURE);

		metacall_value_destroy(ret);

		/* Await function that returns */
		ret = metacall_await("return_await", metacall_null_args, hello_world_await_ok, hello_world_await_fail, NULL);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_FUTURE);

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	metacall_allocator_destroy(allocator);

	metacall_destroy();

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		/* Total amount of successful callbacks must be 26 */
		EXPECT_EQ((int)success_callbacks, (int)26);
	}
#endif /* OPTION_BUILD_LOADERS_NODE */
}
