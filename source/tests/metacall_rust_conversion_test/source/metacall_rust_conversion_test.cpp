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

#include <sstream>
#include <string>

class metacall_rust_conversion_test : public testing::Test
{
public:
};

TEST_F(metacall_rust_conversion_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	// Initialize the script
	{
		std::ostringstream oss;

		oss << "use std::collections::HashMap;" << std::endl;

		// TODO: We can infer this with metacall_loader_types(...)
		// but right now templates are not supported, and the types
		// are registered in a way that cannot be converted into code
		// porperly, you can check pub unsafe fn define_type(...)
		// in rs_loader_impl_initialize for understanding it
		const char *rust_types[] = {
			"bool",
			"i8",
			"i16",
			"i32",
			"i64",
			"f32",
			"f64",
			"String",
			"Vec<i32>",
			"HashMap<String, String>",
		};

		size_t id = 0;

		for (const auto &t : rust_types)
		{
			oss << "fn identity_" << metacall_value_id_name((enum metacall_value_id)id) << "(x: " << t << ") -> " << t << " { x }";
			++id;
		}

		std::string identity_script = oss.str();

		std::cout << identity_script << std::endl;

		ASSERT_EQ((int)0, (int)metacall_load_from_memory("rs", identity_script.c_str(), identity_script.length(), NULL));
	}

	// Test identity
	{
		for (size_t id = 0; id < METACALL_SIZE; ++id)
		{
			const char *type_id_name = metacall_value_id_name((enum metacall_value_id)id);
			std::string func_name = std::string("identity_") + type_id_name;
			void *f = metacall_function(func_name.c_str());

			if (f == NULL)
			{
				std::cout << type_id_name << " => NULL (unsupported)" << std::endl;
				continue;
			}

			void *args[1] = { metacall_value_create((enum metacall_value_id)id) };
			void *ret = metacallfv(f, args);

			ASSERT_NE(ret, nullptr);
			ASSERT_EQ(metacall_value_id(ret), (enum metacall_value_id)id);

			std::cout << type_id_name << " => " << metacall_value_id_name(metacall_value_id(ret)) << std::endl;

			metacall_value_destroy(ret);
			metacall_value_destroy(args[0]);
		}
	}

	metacall_destroy();
}
