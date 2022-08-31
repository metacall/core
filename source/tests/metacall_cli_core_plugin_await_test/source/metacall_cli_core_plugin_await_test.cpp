/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <filesystem>

namespace fs = std::filesystem;

class metacall_cli_core_plugin_await_test : public testing::Test
{
public:
};

TEST_F(metacall_cli_core_plugin_await_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* Extension */
	void *handle = metacall_plugin_extension();

	ASSERT_NE((void *)NULL, (void *)handle);

	void *args[] = {
		metacall_value_create_string(METACALL_PLUGIN_PATH, sizeof(METACALL_PLUGIN_PATH) - 1),
		metacall_value_create_ptr(&handle)
	};

	void *result = metacallhv_s(handle, "plugin_load_from_path", args, sizeof(args) / sizeof(args[0]));

	ASSERT_NE((void *)NULL, (void *)result);

	EXPECT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(result));

	EXPECT_EQ((int)0, (int)metacall_value_to_int(result));

	metacall_value_destroy(args[0]);
	metacall_value_destroy(args[1]);
	metacall_value_destroy(result);

	/* Get core plugin path and handle in order to load cli plugins */
	const char *plugin_path = metacall_plugin_path();
	void *plugin_extension_handle = metacall_plugin_extension();
	void *cli_plugin_handle = NULL;

	ASSERT_NE((const char *)plugin_path, (const char *)NULL);
	ASSERT_NE((void *)plugin_extension_handle, (void *)NULL);

	/* Define the cli plugin path as string (core plugin path plus cli) */
	fs::path plugin_cli_path(plugin_path);
	plugin_cli_path /= "cli";
	std::string plugin_cli_path_str(plugin_cli_path.string());

	/* Load cli plugins into plugin cli handle */
	void *args_cli[] = {
		metacall_value_create_string(plugin_cli_path_str.c_str(), plugin_cli_path_str.length()),
		metacall_value_create_ptr(&cli_plugin_handle)
	};

	result = metacallhv_s(plugin_extension_handle, "plugin_load_from_path", args_cli, sizeof(args_cli) / sizeof(args_cli[0]));

	ASSERT_NE((void *)result, (void *)NULL);
	ASSERT_EQ((int)0, (int)metacall_value_to_int(result));

	metacall_value_destroy(args_cli[0]);
	metacall_value_destroy(args_cli[1]);
	metacall_value_destroy(result);

	/* Test eval */
	void *func = metacall_handle_function(cli_plugin_handle, "eval");

	ASSERT_NE((void *)func, (void *)NULL);

	static const char eval_loader_str[] = "node";
	static const char eval_str[] = "console.log('hello world')";

	void *args_eval[] = {
		metacall_value_create_string(eval_loader_str, sizeof(eval_loader_str) - 1),
		metacall_value_create_string(eval_str, sizeof(eval_str) - 1)
	};

	result = metacallfv_s(func, args_eval, sizeof(args_eval) / sizeof(args_eval[0]));

	EXPECT_NE((void *)NULL, (void *)result);

	EXPECT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(result));

	EXPECT_EQ((int)0, (int)metacall_value_to_int(result));

	metacall_value_destroy(args_eval[0]);
	metacall_value_destroy(args_eval[1]);
	metacall_value_destroy(result);

	/* Test await */
	func = metacall_handle_function(cli_plugin_handle, "await");

	ASSERT_NE((void *)func, (void *)NULL);

	void *args_test[] = {
		metacall_value_create_function(func)
	};

	result = metacallhv_s(handle, "await__test", args_test, sizeof(args_test) / sizeof(args_test[0]));

	EXPECT_NE((void *)NULL, (void *)result);

	EXPECT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(result));

	EXPECT_EQ((double)22.0, (double)metacall_value_to_double(result));

	metacall_value_destroy(args_test[0]);
	metacall_value_destroy(result);

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
