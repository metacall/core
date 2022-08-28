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

class await_test : public testing::Test
{
public:
};

TEST_F(await_test, DefaultConstructor)
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

/* NodeJS */
#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		/* Get core plugin path and handle in order to load cli plugins */
		const char *plugin_path = metacall_plugin_path();
		void *plugin_extension_handle = metacall_plugin_extension();
		void *cli_plugin_handle = NULL;

		if (plugin_path != NULL && plugin_extension_handle != NULL)
		{
			/* Define the cli plugin path as string (core plugin path plus cli) */
			fs::path plugin_cli_path(plugin_path);
			plugin_cli_path /= "cli";
			std::string plugin_cli_path_str(plugin_cli_path.string());

			/* Load cli plugins into plugin cli handle */
			void *args[] = {
				metacall_value_create_string(plugin_cli_path_str.c_str(), plugin_cli_path_str.length()),
				metacall_value_create_ptr(&cli_plugin_handle)
			};

			void *ret = metacallhv_s(plugin_extension_handle, "plugin_load_from_path", args, sizeof(args) / sizeof(args[0]));

			if (ret == NULL || (ret != NULL && metacall_value_to_int(ret) != 0))
			{
				std::cerr << "Failed to load CLI plugins from folder: " << plugin_cli_path_str << std::endl;
			}

			metacall_value_destroy(args[0]);
			metacall_value_destroy(args[1]);
			metacall_value_destroy(ret);
		}

		void *func = metacall_handle_function(cli_plugin_handle, "await");
		if (func == NULL)
			std::cerr << "function not in handle\n " << METACALL_PLUGIN_PATH << '\n';
		void *args[] = {
			metacall_value_create_function(func)
		};
		void *ret = metacallhv_s(handle, "await__test", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);

		EXPECT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(ret));

		EXPECT_EQ((double)22, (long)metacall_value_to_double(ret));

		metacall_value_destroy(ret);
	}
#endif /* OPTION_BUILD_LOADERS_NODE */

	/* Print inspect information */
	{
		size_t size = 0;

		struct metacall_allocator_std_type std_ctx = { &std::malloc, &std::realloc, &std::free };

		void *allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

		char *inspect_str = metacall_inspect(&size, allocator);

		EXPECT_NE((char *)NULL, (char *)inspect_str);

		EXPECT_GT((size_t)size, (size_t)0);

		//std::cout << inspect_str << std::endl;

		metacall_allocator_free(allocator, inspect_str);

		metacall_allocator_destroy(allocator);
	}

	EXPECT_EQ((int)0, (int)metacall_destroy());
}
