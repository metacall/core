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
#include <string>

class metacall_csharp_function_test : public testing::Test
{
public:
};

TEST_F(metacall_csharp_function_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

/* C# Netcore */
#if defined(OPTION_BUILD_LOADERS_CS)
	{
		const char *cs_scripts[] = {
			"function.cs"
		};

		EXPECT_EQ((int)0, (int)metacall_load_from_file("cs", cs_scripts, sizeof(cs_scripts) / sizeof(cs_scripts[0]), NULL));

		void *ret = metacall("TopLevelFunc");

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((int)8, (int)metacall_value_to_int(ret));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_CS */

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

#if defined(OPTION_BUILD_LOADERS_CS)
TEST_F(metacall_csharp_function_test, MoreThan100Functions)
{
	ASSERT_EQ(0, metacall_initialize());

	/* Ensure teardown also runs after a fatal test assertion. */
	struct metacall_cleanup
	{
		~metacall_cleanup()
		{
			metacall_destroy();
		}
	} cleanup;

	const int function_count = 101;

	/* A top-level statement selects the existing top-level function path. */
	std::string source =
		"System.Console.WriteLine(\"Metadata buffer regression\");\n";

	for (int i = 0; i < function_count; ++i)
	{
		source += "static int MetadataBufferFunction" +
				  std::to_string(i) +
				  "() { return " +
				  std::to_string(i) +
				  "; }\n";
	}

	ASSERT_EQ(0, metacall_load_from_memory(
					 "cs",
					 source.c_str(),
					 source.size() + 1,
					 nullptr));

	/* Check every exported function, including the record beyond index 99. */
	for (int i = 0; i < function_count; ++i)
	{
		SCOPED_TRACE(i);

		const std::string name =
			"MetadataBufferFunction" + std::to_string(i);

		void *ret = metacall(name.c_str());

		ASSERT_NE(nullptr, ret);
		EXPECT_EQ(i, metacall_value_to_int(ret));

		metacall_value_destroy(ret);
	}
}

TEST_F(metacall_csharp_function_test, FunctionMetadataSurvivesRepeatedLoad)
{
	ASSERT_EQ(0, metacall_initialize());

	struct metacall_cleanup
	{
		~metacall_cleanup()
		{
			metacall_destroy();
		}
	} cleanup;

	auto expect_integer_result = [](void *func, int expected) {
		/* Zero parameters, but the invocation API needs a non-null array. */
		void *args[] = { nullptr };
		void *ret = metacallfv(func, args);

		ASSERT_NE(nullptr, ret);
		EXPECT_EQ(METACALL_INT, metacall_value_id(ret));

		if (metacall_value_id(ret) == METACALL_INT)
		{
			EXPECT_EQ(expected, metacall_value_to_int(ret));
		}

		metacall_value_destroy(ret);
	};

	/* Load A and retain its original native function reference. */
	const std::string first_source =
		"System.Console.WriteLine(\"Metadata lifetime fixture A\");\n"
		"static int MetadataLifetimeOriginal() { return 7331; }\n";

	void *first_handle = nullptr;

	ASSERT_EQ(0, metacall_load_from_memory(
					 "cs",
					 first_source.c_str(),
					 first_source.size() + 1,
					 &first_handle));

	ASSERT_NE(nullptr, first_handle);

	void *original_function =
		metacall_handle_function(first_handle, "MetadataLifetimeOriginal");

	ASSERT_NE(nullptr, original_function);

	{
		SCOPED_TRACE("Original function before second load");
		expect_integer_result(original_function, 7331);
	}

	/* Load B with enough metadata to replace and enlarge the native buffer. */
	const int second_function_count = 250;

	std::string second_source =
		"System.Console.WriteLine(\"Metadata lifetime fixture B\");\n";

	for (int i = 0; i < second_function_count; ++i)
	{
		second_source += "static int MetadataLifetimeAdded" +
						 std::to_string(i) +
						 "() { return " +
						 std::to_string(10000 + i) +
						 "; }\n";
	}

	void *second_handle = nullptr;

	ASSERT_EQ(0, metacall_load_from_memory(
					 "cs",
					 second_source.c_str(),
					 second_source.size() + 1,
					 &second_handle));

	ASSERT_NE(nullptr, second_handle);

	/* Use the original reference, not a fresh lookup in the second handle. */
	{
		SCOPED_TRACE("Original function after second load");
		expect_integer_result(original_function, 7331);
	}

	for (int i = 0; i < second_function_count; ++i)
	{
		SCOPED_TRACE(i);

		const std::string name =
			"MetadataLifetimeAdded" + std::to_string(i);

		void *func = metacall_handle_function(second_handle, name.c_str());

		ASSERT_NE(nullptr, func);
		expect_integer_result(func, 10000 + i);
	}

	{
		SCOPED_TRACE("Original function after invoking all new functions");
		expect_integer_result(original_function, 7331);
	}
}
#endif /* OPTION_BUILD_LOADERS_CS */
