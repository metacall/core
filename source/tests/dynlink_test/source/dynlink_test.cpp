/*
 *	MetaCall Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <dynlink/dynlink.h>

#include <environment/environment_variable_path.h>

#include <log/log.h>

#define DYNLINK_TEST_LIBRARY_PATH "DYNLINK_TEST_LIBRARY_PATH"

typedef void (*mock_loader_print_func)(void);

class dynlink_test : public testing::Test
{
protected:
};

TEST_F(dynlink_test, DefaultConstructor)
{
	EXPECT_EQ((int)0, (int)log_configure("metacall",
						  log_policy_format_text(),
						  log_policy_schedule_sync(),
						  log_policy_storage_sequential(),
						  log_policy_stream_stdio(stdout)));

	dynlink_print_info();

	log_write("metacall", LOG_LEVEL_DEBUG, "Dynamic linked shared object extension: %s", dynlink_extension());

	{
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		const char library_name[] = "mock_loaderd";
#else
		const char library_name[] = "mock_loader";
#endif

		char *path = environment_variable_path_create(DYNLINK_TEST_LIBRARY_PATH, NULL, 0, NULL);

		dynlink handle = dynlink_load(path, library_name, DYNLINK_FLAGS_BIND_NOW | DYNLINK_FLAGS_BIND_GLOBAL);

		environment_variable_path_destroy(path);

		EXPECT_NE(handle, (dynlink)NULL);

		log_write("metacall", LOG_LEVEL_DEBUG, "Dynamic linked shared object file: %s", dynlink_get_name_impl(handle));

		if (handle != NULL)
		{
			static dynlink_symbol_addr mock_loader_print_info_addr;

			EXPECT_EQ((int)0, dynlink_symbol(handle, DYNLINK_SYMBOL_STR("mock_loader_print_info"), &mock_loader_print_info_addr));

			if (mock_loader_print_info_addr != NULL)
			{
				mock_loader_print_func print = DYNLINK_SYMBOL_GET(mock_loader_print_info_addr);

				log_write("metacall", LOG_LEVEL_DEBUG, "Print function: %p", (void *)print);

				log_write("metacall", LOG_LEVEL_DEBUG, "Symbol pointer: %p", (void *)mock_loader_print_info_addr);

				if (DYNLINK_SYMBOL_GET(mock_loader_print_info_addr) != NULL)
				{
					log_write("metacall", LOG_LEVEL_DEBUG, "Pointer is valid");
				}

				print();
			}

			dynlink_unload(handle);
		}
	}
}
