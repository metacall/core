/*
 *	MetaCall Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
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

#include <dynlink/dynlink.h>

#include <environment/environment_variable_path.h>

#include <log/log.h>

#define DYNLINK_TEST_LIBRARY_PATH "DYNLINK_TEST_LIBRARY_PATH"

typedef const char *(*mock_loader_print_func)(void);

class dynlink_test : public testing::Test
{
protected:
};

#ifdef _WIN32
	#define EXPORT_SYMBOL __declspec(dllexport)
#else
	#define EXPORT_SYMBOL __attribute__((visibility("default")))
#endif

extern "C" EXPORT_SYMBOL int function_from_current_executable(void)
{
	log_write("metacall", LOG_LEVEL_INFO, "function_from_current_executable");

	return 48;
}

TEST_F(dynlink_test, DefaultConstructor)
{
	EXPECT_EQ((int)0, (int)log_configure("metacall",
						  log_policy_format_text(),
						  log_policy_schedule_sync(),
						  log_policy_storage_sequential(),
						  log_policy_stream_stdio(stdout)));

	log_write("metacall", LOG_LEVEL_DEBUG, "dynlink_test: calling dynlink_print_info");
	dynlink_print_info();

	const char *ext = dynlink_extension();
	log_write("metacall", LOG_LEVEL_DEBUG, "Dynamic linked shared object extension: %s", ext != NULL ? ext : "(null)");

	/* Test loading symbols from current process */
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "dynlink_test: calling dynlink_load_self");
		dynlink proc = dynlink_load_self(DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);

		if (proc == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "dynlink_test: dynlink_load_self returned NULL");
		}
		ASSERT_NE((dynlink)proc, (dynlink)(NULL));

		log_write("metacall", LOG_LEVEL_DEBUG, "dynlink_test: dynlink_load_self succeeded (name: %s, path: %s)",
			dynlink_get_name(proc) ? dynlink_get_name(proc) : "(null)",
			dynlink_get_path(proc) ? dynlink_get_path(proc) : "(null)");

		dynlink_symbol_addr addr = nullptr;

		log_write("metacall", LOG_LEVEL_DEBUG, "dynlink_test: calling dynlink_symbol for function_from_current_executable");
		int sym_ret = dynlink_symbol(proc, "function_from_current_executable", &addr);
		log_write("metacall", LOG_LEVEL_DEBUG, "dynlink_test: dynlink_symbol returned %d, addr %p", sym_ret, (void *)addr);

		if (sym_ret != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "dynlink_symbol failed for function_from_current_executable (return %d, addr %p)", sym_ret, (void *)addr);
		}
		EXPECT_EQ((int)0, sym_ret);
		ASSERT_NE((dynlink_symbol_addr)addr, (dynlink_symbol_addr)NULL);

		int (*fn_ptr)(void) = (int (*)(void))addr;
		ASSERT_NE((void *)fn_ptr, (void *)NULL);

		log_write("metacall", LOG_LEVEL_DEBUG, "dynlink_test: calling fn_ptr at %p (expected %p)",
			(void *)fn_ptr, (void *)&function_from_current_executable);
		EXPECT_EQ((int)48, fn_ptr());

		EXPECT_EQ((int (*)(void))(&function_from_current_executable), (int (*)(void))fn_ptr);

		dynlink_unload(proc);
		proc = nullptr;
	}

#ifdef DYNLINK_TEST_MOCK_LOADER
	{
	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		const char library_name[] = "mock_loaderd";
	#else
		const char library_name[] = "mock_loader";
	#endif

		char *path = environment_variable_path_create(DYNLINK_TEST_LIBRARY_PATH, NULL, 0, NULL);

		ASSERT_NE((char *)path, (char *)NULL);

		/* Test library loading */
		{
			dynlink handle = dynlink_load(path, library_name, DYNLINK_FLAGS_BIND_NOW | DYNLINK_FLAGS_BIND_GLOBAL);

			ASSERT_NE(handle, (dynlink)NULL);

			log_write("metacall", LOG_LEVEL_DEBUG, "Dynamic linked shared object file: %s", dynlink_get_path(handle));

			EXPECT_STREQ(library_name, dynlink_get_name(handle));

			if (handle != NULL)
			{
				dynlink_symbol_addr mock_loader_print_info_addr = nullptr;

				int sym_ret = dynlink_symbol(handle, "mock_loader_print_info", &mock_loader_print_info_addr);
				if (sym_ret != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "dynlink_symbol failed for mock_loader_print_info (return %d)", sym_ret);
				}
				EXPECT_EQ((int)0, sym_ret);

				if (mock_loader_print_info_addr != NULL)
				{
					mock_loader_print_func print = (mock_loader_print_func)mock_loader_print_info_addr;
					log_write("metacall", LOG_LEVEL_DEBUG, "Print function: %p", (void *)print);
					log_write("metacall", LOG_LEVEL_DEBUG, "Print: %s", print());
				}

				dynlink_unload(handle);
				handle = nullptr;
			}
		}

		/* Test loading symbols from absolute path */
		{
			char library_name_platform[PORTABILITY_PATH_SIZE];
			char absolute_path[PORTABILITY_PATH_SIZE];

			dynlink_platform_name(library_name, library_name_platform);

			portability_path_join(path, strlen(path) + 1, library_name_platform, strlen(library_name_platform) + 1, absolute_path, PORTABILITY_PATH_SIZE);

			dynlink handle = dynlink_load_absolute(absolute_path, DYNLINK_FLAGS_BIND_NOW | DYNLINK_FLAGS_BIND_GLOBAL);

			ASSERT_NE(handle, (dynlink)NULL);

			log_write("metacall", LOG_LEVEL_DEBUG, "Dynamic linked shared object absolute path: %s", absolute_path);
			log_write("metacall", LOG_LEVEL_DEBUG, "Dynamic linked shared object file name:     %s", dynlink_get_path(handle));
			log_write("metacall", LOG_LEVEL_DEBUG, "Dynamic linked shared object file:          %s", dynlink_get_name(handle));

			EXPECT_STREQ(absolute_path, dynlink_get_path(handle));
			EXPECT_STREQ(library_name, dynlink_get_name(handle));

			if (handle != NULL)
			{
				dynlink_symbol_addr mock_loader_print_info_addr = nullptr;

				int sym_ret = dynlink_symbol(handle, "mock_loader_print_info", &mock_loader_print_info_addr);
				if (sym_ret != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "dynlink_symbol failed for mock_loader_print_info (absolute path, return %d)", sym_ret);
				}
				EXPECT_EQ((int)0, sym_ret);

				if (mock_loader_print_info_addr != NULL)
				{
					mock_loader_print_func print = (mock_loader_print_func)mock_loader_print_info_addr;
					log_write("metacall", LOG_LEVEL_DEBUG, "Print function: %p", (void *)print);
					log_write("metacall", LOG_LEVEL_DEBUG, "Print: %s", print());
				}

				dynlink_unload(handle);
				handle = nullptr;
			}
		}

		environment_variable_path_destroy(path);
		path = nullptr; /* do not use after destroy */
	}
#endif
}
