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

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

class metacall_rust_conversion_test : public testing::Test
{
public:
};

static bool is_synthetic_rust_type(const char *name)
{
	return std::strcmp(name, "Array") == 0 ||
		   std::strcmp(name, "Map") == 0 ||
		   std::strcmp(name, "Ptr") == 0 ||
		   std::strcmp(name, "Null") == 0;
}

TEST_F(metacall_rust_conversion_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	const char identity_script[] =
		"pub fn identity<T>(x: T) -> T {\n"
		"    x\n"
		"}\n";

	ASSERT_EQ((int)0, (int)metacall_load_from_memory("rs", identity_script, sizeof(identity_script) - 1, NULL));

	void *identity_tpl = metacall_template("identity");

	ASSERT_NE(identity_tpl, nullptr);

	struct metacall_loader_type *types = NULL;
	size_t types_size = 0;

	ASSERT_EQ(0, metacall_loader_types("rs", &types, &types_size));

	/* The array is malloc'd for the caller, name and t are owned by the loader */
	std::unique_ptr<struct metacall_loader_type, void (*)(void *)> types_guard(types, std::free);

	ASSERT_GT(types_size, (size_t)0);

	for (size_t i = 0; i < types_size; ++i)
	{
		SCOPED_TRACE(types[i].name);

		struct metacall_template_argument args[] = {
			{ METACALL_TEMPLATE_ARGUMENT_TYPE, types[i].t }
		};

		void *fn_val = metacall_template_instantiate(identity_tpl, args, sizeof(args) / sizeof(args[0]));

		/* Non-Rust synthetic type names registered in initialize.rs (Array, Map, Ptr, Null)
		 * fail rustc compilation because they are not valid Rust types */
		if (is_synthetic_rust_type(types[i].name))
		{
			EXPECT_EQ(fn_val, nullptr);
			std::cout << types[i].name << " (" << metacall_value_id_name(types[i].id) << ") => NULL (instantiation failed as expected)" << std::endl;
			continue;
		}

		if (fn_val == nullptr || metacall_value_id(fn_val) != METACALL_FUNCTION)
		{
			ADD_FAILURE() << "Failed to instantiate identity::<" << types[i].name << ">";
			continue;
		}

		void *f = metacall_value_to_function(fn_val);
		void *call_args[1] = { metacall_value_create(types[i].id) };
		void *ret = metacallfv_s(f, call_args, 1);

		/* metacallfv_s may replace the slot with a cast value, so always release call_args[0] itself */
		metacall_value_destroy(call_args[0]);

		if (ret == nullptr)
		{
			ADD_FAILURE() << "identity::<" << types[i].name << "> returned NULL";
			continue;
		}

		EXPECT_EQ(metacall_value_id(ret), types[i].id);

		std::cout << types[i].name << " (" << metacall_value_id_name(types[i].id) << ") => "
				  << metacall_value_id_name(metacall_value_id(ret)) << std::endl;

		metacall_value_destroy(ret);
	}

	metacall_destroy();
}
