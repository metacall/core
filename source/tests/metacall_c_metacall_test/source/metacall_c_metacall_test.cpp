/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
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
#include <metacall/metacall.hpp>
#include <stdlib.h>
#include <string.h>

using namespace metacall;
using namespace metacall::detail;

static void unset_self_load_env(void)
{
#if defined(WIN32) || defined(_WIN32)
	_putenv_s("METACALL_SELF_LOAD", "");
#else
	unsetenv("METACALL_SELF_LOAD");
#endif
}

class metacall_c_metacall_test : public testing::Test
{
protected:
	void TearDown() override
	{
		unset_self_load_env();
	}
};

/* Test 1: Verifies that without METACALL_SELF_LOAD, startup is vanilla and self-load does not run */
TEST_F(metacall_c_metacall_test, DisabledByDefault)
{
	unset_self_load_env();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	EXPECT_EQ((void *)NULL, metacall_handle("c", "metacall"));
	EXPECT_EQ((void *)NULL, metacall_handle_self());

	metacall_destroy();
}

/* Test 2: Verifies that with METACALL_SELF_LOAD=1, symbols are loaded into a handle and retrievable */
TEST_F(metacall_c_metacall_test, OptInSelfLoad)
{
#if defined(WIN32) || defined(_WIN32)
	_putenv_s("METACALL_SELF_LOAD", "1");
#else
	setenv("METACALL_SELF_LOAD", "1", 1);
#endif

	ASSERT_EQ((int)0, (int)metacall_initialize());

	/* 1. Verify symbols are PRIVATE to the handle (NOT accessible globally) */
	{
		void *global_ret = metacallv_s("metacall_print_info", metacall_null_args, 0);
		EXPECT_EQ((void *)NULL, global_ret);
	}

	/* 2. Verify handle retrieval and handle invocation */
	void *handle = metacall_handle("c", "metacall");
	ASSERT_NE((void *)NULL, handle);
	EXPECT_EQ(handle, metacall_handle_self());

	void *ret = metacallhv_s(handle, "metacall_print_info", metacall_null_args, 0);
	ASSERT_NE((void *)NULL, ret);
	EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_STRING);
	EXPECT_EQ((int)0, strncmp(metacall_value_to_string(ret), "MetaCall", 8));
	metacall_value_destroy(ret);

	/* 3. Export handle symbols and verify functions are intact */
	{
		void *export_val = metacall_handle_export(handle);
		ASSERT_NE((void *)NULL, export_val);
		ASSERT_EQ(METACALL_MAP, (enum metacall_value_id)metacall_value_id(export_val));

		metacall::map_typed<std::string, metacall::value> exported_map(export_val);
		size_t metacall_funcs_count = 0;
		size_t query_failures = 0;

		for (const auto &pair : exported_map)
		{
			const std::string &func_name = pair.first;
			if (func_name.rfind("metacall", 0) != 0)
			{
				continue;
			}

			void *func_ptr = metacall_handle_function(handle, func_name.c_str());
			if (func_ptr == NULL)
			{
				continue;
			}

			enum metacall_value_id ret_id = METACALL_INVALID;
			if (metacall_function_return_type(func_ptr, &ret_id) != 0)
			{
				++query_failures;
				continue;
			}

			size_t args_count = metacall_function_size(func_ptr);
			for (size_t i = 0; i < args_count; ++i)
			{
				enum metacall_value_id arg_id = METACALL_INVALID;
				if (metacall_function_parameter_type(func_ptr, i, &arg_id) != 0)
				{
					++query_failures;
				}
			}
			++metacall_funcs_count;
		}

		EXPECT_NE((size_t)0, metacall_funcs_count);
		EXPECT_EQ((size_t)0, query_failures);

		metacall_value_destroy(export_val);
	}

	metacall_destroy();
}
