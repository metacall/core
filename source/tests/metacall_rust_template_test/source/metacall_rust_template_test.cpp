/*
 *	Loader Library by Parra Studios
 *	A plugin for loading rust code at run-time into a process.
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

#include <cstdlib>
#include <iostream>
#include <string>

using namespace metacall;

/* Templates to instantiate at load time, each element is { "<template>": [ "<type>", ... ] } */
using template_map = metacall::map_typed<std::string, metacall::array>;

/* The instance is a value owned by the template, so it is only unwrapped (not destroyed) */
static void *metacall_rust_template_function(void *v)
{
	if (v == NULL)
	{
		return NULL;
	}

	EXPECT_EQ((enum metacall_value_id)METACALL_FUNCTION, metacall_value_id(v));

	return metacall_value_to_function(v);
}

class metacall_rust_template_test : public testing::Test
{
protected:
	static void SetUpTestSuite()
	{
		metacall_print_info();

		ASSERT_EQ(0, metacall_initialize());

		/* Reproduces a bug when loading multiple files: each path was searched in the execution paths
		 * from where the previous path was found, instead of in all of them, so a second execution path
		 * after the scripts folder is needed (see FileWithoutLoadData) */
		const char *script_path = std::getenv("LOADER_SCRIPT_PATH");

		ASSERT_NE(script_path, nullptr);

		ASSERT_EQ(0, metacall_execution_path("rs", (std::string(script_path) + "/template_path").c_str()));
	}

	static void TearDownTestSuite()
	{
		metacall_destroy();
	}
};

TEST_F(metacall_rust_template_test, DefaultConstructor)
{
	const char script[] =
		"pub fn identity<T>(x: T) -> T {"
		"    x"
		"}"
		"pub fn pair<T, U>(a: T, b: U) -> U {"
		"    b"
		"}"
		"pub fn add<T: std::ops::Add<Output = T>>(a: T, b: T) -> T {"
		"    a + b"
		"}";

	template_map data{ { "templates", metacall::array(
										  template_map{ { "identity", metacall::array("f64") } },
										  template_map{ { "pair", metacall::array("i32", "f64") } }) } };

	int result = metacall_load_from_memory_ex(
		"rs",
		script,
		sizeof(script) - 1,
		NULL,
		data.to_raw());

	ASSERT_EQ(0, result);

	void *identity = metacall_function("identity::<f64>");
	void *pair = metacall_function("pair::<i32, f64>");

	std::cout << "identity::<f64>: " << identity << std::endl;
	std::cout << "pair::<i32, f64>: " << pair << std::endl;

	ASSERT_NE(identity, nullptr);
	ASSERT_NE(pair, nullptr);

	void *identity_ret = metacall::metacall("identity::<f64>", 42.5);

	ASSERT_NE(identity_ret, nullptr);
	EXPECT_EQ((double)metacall_value_to_double(identity_ret), 42.5);

	void *pair_ret = metacall::metacall("pair::<i32, f64>", 123, 3.14);

	ASSERT_NE(pair_ret, nullptr);
	EXPECT_EQ((double)metacall_value_to_double(pair_ret), 3.14);

	/* Templates in the global scope */
	void *identity_tpl = metacall_template("identity");
	void *pair_tpl = metacall_template("pair");

	ASSERT_NE(identity_tpl, nullptr);
	ASSERT_NE(pair_tpl, nullptr);

	EXPECT_EQ((size_t)1, metacall_template_size(identity_tpl));
	EXPECT_STREQ("T", metacall_template_parameter(identity_tpl, 0));
	EXPECT_EQ(nullptr, metacall_template_parameter(identity_tpl, 1));

	EXPECT_EQ((size_t)2, metacall_template_size(pair_tpl));
	EXPECT_STREQ("T", metacall_template_parameter(pair_tpl, 0));
	EXPECT_STREQ("U", metacall_template_parameter(pair_tpl, 1));
	EXPECT_EQ(nullptr, metacall_template_parameter(pair_tpl, 2));

	/* Templates and instantiations are not mixed */
	EXPECT_EQ(nullptr, metacall_function("identity"));
	EXPECT_EQ(nullptr, metacall_template("identity::<f64>"));
	EXPECT_EQ(nullptr, metacall_template("does_not_exist"));

	/* Invalid arguments */
	EXPECT_EQ((size_t)0, metacall_template_size(NULL));
	EXPECT_EQ(nullptr, metacall_template_parameter(NULL, 0));

	/* Instantiation through the API */
	struct metacall_loader_type f64_type, i32_type, f32_type;

	ASSERT_EQ(0, metacall_loader_type("rs", "f64", &f64_type));
	ASSERT_EQ(0, metacall_loader_type("rs", "i32", &i32_type));
	ASSERT_EQ(0, metacall_loader_type("rs", "f32", &f32_type));

	struct metacall_template_argument identity_args[] = {
		{ METACALL_TEMPLATE_ARGUMENT_TYPE, f64_type.t }
	};

	struct metacall_template_argument pair_args[] = {
		{ METACALL_TEMPLATE_ARGUMENT_TYPE, i32_type.t },
		{ METACALL_TEMPLATE_ARGUMENT_TYPE, f64_type.t }
	};

	/* The instantiations done at load time are cached in the template, so the same function is returned */
	EXPECT_EQ(identity, metacall_rust_template_function(metacall_template_instantiate(identity_tpl, identity_args, sizeof(identity_args) / sizeof(identity_args[0]))));
	EXPECT_EQ(identity, metacall_rust_template_function(metacall_template_instantiate(identity_tpl, identity_args, sizeof(identity_args) / sizeof(identity_args[0]))));
	EXPECT_EQ(pair, metacall_rust_template_function(metacall_template_instantiate(pair_tpl, pair_args, sizeof(pair_args) / sizeof(pair_args[0]))));

	{
		void *args[] = {
			metacall_value_create_double(7.5)
		};

		void *ret = metacallfv_s(metacall_rust_template_function(metacall_template_instantiate(identity_tpl, identity_args, sizeof(identity_args) / sizeof(identity_args[0]))), args, sizeof(args) / sizeof(args[0]));

		ASSERT_NE(ret, nullptr);
		EXPECT_EQ((double)metacall_value_to_double(ret), 7.5);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* The instantiations not compiled at load time are compiled into a new crate */
	struct metacall_template_argument f32_args[] = {
		{ METACALL_TEMPLATE_ARGUMENT_TYPE, f32_type.t }
	};

	void *identity_f32 = metacall_rust_template_function(metacall_template_instantiate(identity_tpl, f32_args, sizeof(f32_args) / sizeof(f32_args[0])));

	ASSERT_NE(identity_f32, nullptr);

	/* The instantiation is cached in the template, it is not compiled again */
	EXPECT_EQ(identity_f32, metacall_rust_template_function(metacall_template_instantiate(identity_tpl, f32_args, sizeof(f32_args) / sizeof(f32_args[0]))));

	{
		void *args[] = {
			metacall_value_create_float(2.5f)
		};

		void *ret = metacallfv_s(identity_f32, args, sizeof(args) / sizeof(args[0]));

		ASSERT_NE(ret, nullptr);
		EXPECT_EQ((float)metacall_value_to_float(ret), 2.5f);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* Templates with trait bounds */
	void *add_tpl = metacall_template("add");

	ASSERT_NE(add_tpl, nullptr);

	struct metacall_template_argument add_i32_args[] = {
		{ METACALL_TEMPLATE_ARGUMENT_TYPE, i32_type.t }
	};

	void *add_i32 = metacall_rust_template_function(metacall_template_instantiate(add_tpl, add_i32_args, sizeof(add_i32_args) / sizeof(add_i32_args[0])));

	ASSERT_NE(add_i32, nullptr);

	{
		void *args[] = {
			metacall_value_create_int(3),
			metacall_value_create_int(4)
		};

		void *ret = metacallfv_s(add_i32, args, sizeof(args) / sizeof(args[0]));

		ASSERT_NE(ret, nullptr);
		EXPECT_EQ((int)metacall_value_to_int(ret), 7);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
		metacall_value_destroy(args[1]);
	}

	/* String does not implement Add<String>, so the instantiation does not compile */
	struct metacall_loader_type string_type;

	ASSERT_EQ(0, metacall_loader_type("rs", "String", &string_type));

	struct metacall_template_argument add_string_args[] = {
		{ METACALL_TEMPLATE_ARGUMENT_TYPE, string_type.t }
	};

	EXPECT_EQ(nullptr, metacall_template_instantiate(add_tpl, add_string_args, sizeof(add_string_args) / sizeof(add_string_args[0])));

	/* The failed instantiation is not cached, the valid ones still work */
	EXPECT_EQ(add_i32, metacall_rust_template_function(metacall_template_instantiate(add_tpl, add_i32_args, sizeof(add_i32_args) / sizeof(add_i32_args[0]))));

	/* TODO: The Rust Loader registers type names that are not Rust types (Array, Map, Ptr and Null),
	* so an instantiation like identity::<Array> cannot be compiled and it fails (returns NULL),
	* they should be mapped to real Rust types (i.e: Array to Vec<...>), review it later
	*/
	{
		static const char *non_rust_types[] = {
			"Array",
			"Map",
			"Ptr",
			"Null"
		};

		for (const char *name : non_rust_types)
		{
			struct metacall_loader_type non_rust_type;

			ASSERT_EQ(0, metacall_loader_type("rs", name, &non_rust_type));

			struct metacall_template_argument non_rust_args[] = {
				{ METACALL_TEMPLATE_ARGUMENT_TYPE, non_rust_type.t }
			};

			EXPECT_EQ(nullptr, metacall_template_instantiate(identity_tpl, non_rust_args, sizeof(non_rust_args) / sizeof(non_rust_args[0])));
		}
	}

	/* Invalid instantiations */
	EXPECT_EQ(nullptr, metacall_template_instantiate(identity_tpl, pair_args, sizeof(pair_args) / sizeof(pair_args[0])));
	EXPECT_EQ(nullptr, metacall_template_instantiate(identity_tpl, NULL, 0));
	EXPECT_EQ(nullptr, metacall_template_instantiate(NULL, identity_args, sizeof(identity_args) / sizeof(identity_args[0])));

	/* Templates in a private handle */
	const char handle_script[] =
		"pub fn wrap<V>(x: V) -> V {"
		"    x"
		"}";

	void *handle = NULL;

	ASSERT_EQ(0, metacall_load_from_memory_ex("rs", handle_script, sizeof(handle_script) - 1, &handle, NULL));

	ASSERT_NE(handle, nullptr);

	void *wrap_tpl = metacall_handle_template(handle, "wrap");

	ASSERT_NE(wrap_tpl, nullptr);

	EXPECT_EQ((size_t)1, metacall_template_size(wrap_tpl));
	EXPECT_STREQ("V", metacall_template_parameter(wrap_tpl, 0));

	/* Instantiation of a template of a private handle, without instantiations at load time */
	struct metacall_loader_type i64_type;

	ASSERT_EQ(0, metacall_loader_type("rs", "i64", &i64_type));

	struct metacall_template_argument wrap_args[] = {
		{ METACALL_TEMPLATE_ARGUMENT_TYPE, i64_type.t }
	};

	void *wrap_i64 = metacall_rust_template_function(metacall_template_instantiate(wrap_tpl, wrap_args, sizeof(wrap_args) / sizeof(wrap_args[0])));

	ASSERT_NE(wrap_i64, nullptr);

	{
		void *args[] = {
			metacall_value_create_long(1234567890123L)
		};

		void *ret = metacallfv_s(wrap_i64, args, sizeof(args) / sizeof(args[0]));

		ASSERT_NE(ret, nullptr);
		EXPECT_EQ((long)metacall_value_to_long(ret), 1234567890123L);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	/* The handle is private, so its templates are not in the global scope */
	EXPECT_EQ(nullptr, metacall_template("wrap"));

	/* The templates of the global scope are not in the private handle */
	EXPECT_EQ(nullptr, metacall_handle_template(handle, "identity"));

	EXPECT_EQ(0, metacall_clear(handle));
}

TEST_F(metacall_rust_template_test, MemoryLoadData)
{
	const char script[] =
		"pub fn identity<T>(x: T) -> T { x }"
		"pub fn pair<T, U>(_a: T, b: U) -> U { b }";

	/* A successful load is cleared, so the same script can be loaded again */
	auto load = [&script](void *data) {
		void *handle = NULL;
		int result = metacall_load_from_memory_ex("rs", script, sizeof(script) - 1, &handle, data);

		if (result == 0)
		{
			EXPECT_EQ(0, metacall_clear(handle));
		}

		return result;
	};

	/* The load data is not a map */
	EXPECT_NE(0, load(metacall::array("identity").to_raw()));

	/* The templates are not an array */
	EXPECT_NE(0, load(metacall::map_typed<std::string, std::string>{ { "templates", "identity" } }.to_raw()));

	/* An element of the templates is not a map */
	EXPECT_NE(0, load(template_map{ { "templates", metacall::array("identity") } }.to_raw()));

	/* An element of the templates has more than one name */
	EXPECT_NE(0, load(template_map{ { "templates", metacall::array(template_map{ { "identity", metacall::array("i64") }, { "pair", metacall::array("i32", "i64") } }) } }.to_raw()));

	/* The types of a template are not an array */
	EXPECT_NE(0, load(template_map{ { "templates", metacall::array(metacall::map_typed<std::string, std::string>{ { "identity", "i64" } }) } }.to_raw()));

	/* A type is not a string */
	EXPECT_NE(0, load(template_map{ { "templates", metacall::array(template_map{ { "identity", metacall::array(64) } }) } }.to_raw()));

	/* TODO: Nested template arguments need class templates, they fail until they are implemented */
	EXPECT_NE(0, load(template_map{ { "templates", metacall::array(template_map{ { "identity", metacall::array(template_map{ { "Inner", metacall::array("i64", "i64") } }) } }) } }.to_raw()));

	/* The template does not exist */
	EXPECT_NE(0, load(template_map{ { "templates", metacall::array(template_map{ { "does_not_exist", metacall::array("i64") } }) } }.to_raw()));

	/* The template expects a different number of types */
	EXPECT_NE(0, load(template_map{ { "templates", metacall::array(template_map{ { "pair", metacall::array("i64") } }) } }.to_raw()));

	/* Without templates in the load data nothing is instantiated */
	{
		metacall::map_typed<std::string, std::string> data{ { "other", "value" } };
		void *handle = NULL;

		ASSERT_EQ(0, metacall_load_from_memory_ex("rs", script, sizeof(script) - 1, &handle, data.to_raw()));

		EXPECT_NE(nullptr, metacall_handle_template(handle, "identity"));
		EXPECT_EQ(nullptr, metacall_handle_function(handle, "identity::<i64>"));

		EXPECT_EQ(0, metacall_clear(handle));
	}
}

/* Two files where only one declares the template, in both orders, it is instantiated
 * at load time in the file which declares it, and at run time through the handle */
TEST_F(metacall_rust_template_test, FileTemplateInOneFile)
{
	const char *orders[][2] = {
		{ "template_identity.rs", "template_twice.rs" },
		{ "template_twice.rs", "template_identity.rs" }
	};

	for (auto &scripts : orders)
	{
		template_map data{ { "templates", metacall::array(template_map{ { "identity", metacall::array("i64") } }) } };
		void *handle = NULL;

		ASSERT_EQ(0, metacall_load_from_file_ex("rs", scripts, 2, &handle, data.to_raw()));

		{
			metacall::function<long, long> identity(handle, "identity::<i64>");
			metacall::function<int, int> twice(handle, "twice");

			EXPECT_EQ(1234567890123L, identity(1234567890123L).to_value());
			EXPECT_EQ(42, twice(21).to_value());

			/* Instantiation at run time, the instance is owned by the template */
			struct metacall_loader_type f64_type;

			ASSERT_EQ(0, metacall_loader_type("rs", "f64", &f64_type));

			struct metacall_template_argument args[] = {
				{ METACALL_TEMPLATE_ARGUMENT_TYPE, f64_type.t }
			};

			metacall::function<double, double> identity_f64("identity::<f64>", metacall_template_instantiate(metacall_handle_template(handle, "identity"), args, sizeof(args) / sizeof(args[0])));

			EXPECT_EQ(2.5, identity_f64(2.5).to_value());
		}

		EXPECT_EQ(0, metacall_clear(handle));
	}
}

/* Two files where none declares the template, the load fails */
TEST_F(metacall_rust_template_test, FileTemplateNotFound)
{
	const char *scripts[] = { "template_twice.rs", "template_triple.rs" };
	template_map data{ { "templates", metacall::array(template_map{ { "identity", metacall::array("i64") } }) } };
	void *handle = NULL;

	EXPECT_NE(0, metacall_load_from_file_ex("rs", scripts, sizeof(scripts) / sizeof(scripts[0]), &handle, data.to_raw()));
}

/* Two files which declare the same template, the load fails */
TEST_F(metacall_rust_template_test, FileTemplateInMoreThanOneFile)
{
	const char *scripts[] = { "template_identity.rs", "template_identity_other.rs" };
	template_map data{ { "templates", metacall::array(template_map{ { "identity", metacall::array("i64") } }) } };
	void *handle = NULL;

	EXPECT_NE(0, metacall_load_from_file_ex("rs", scripts, sizeof(scripts) / sizeof(scripts[0]), &handle, data.to_raw()));
}

/* Two files without load data, the first one is found in the second execution path and the
 * second one in the first, so each path must be searched in all the execution paths */
TEST_F(metacall_rust_template_test, FileWithoutLoadData)
{
	const char *scripts[] = { "template_quadruple.rs", "template_twice.rs" };
	void *handle = NULL;

	ASSERT_EQ(0, metacall_load_from_file_ex("rs", scripts, sizeof(scripts) / sizeof(scripts[0]), &handle, NULL));

	{
		metacall::function<int, int> quadruple(handle, "quadruple");
		metacall::function<int, int> twice(handle, "twice");

		EXPECT_EQ(20, quadruple(5).to_value());
		EXPECT_EQ(42, twice(21).to_value());
	}

	EXPECT_EQ(0, metacall_clear(handle));
}

/* TODO: Class templates are not implemented yet (see TemplateInstantiation in the compiler),
 * enable this test once they are, there is no metacall_handle_class for checking the instance */
TEST_F(metacall_rust_template_test, DISABLED_Recursive)
{
	const char script[] =
		"pub fn identity<T>(x: T) -> T { x }"
		"pub fn pair<T, U>(_a: T, b: U) -> U { b }"
		"pub struct Inner<T, U> { pub first: T, pub second: U }"
		"pub struct Recursive<T, U> { pub value: T, pub inner: U }";

	template_map data{ { "templates", metacall::array(
										  template_map{ { "identity", metacall::array("i64") } },
										  template_map{ { "identity", metacall::array("i32") } },
										  template_map{ { "pair", metacall::array("i32", "i64") } },
										  template_map{ { "Recursive", metacall::array("i32", template_map{ { "Inner", metacall::array("i64", "i64") } }) } }) } };

	void *handle = NULL;

	ASSERT_EQ(0, metacall_load_from_memory_ex("rs", script, sizeof(script) - 1, &handle, data.to_raw()));

	EXPECT_NE(nullptr, metacall_handle_function(handle, "identity::<i64>"));
	EXPECT_NE(nullptr, metacall_handle_function(handle, "identity::<i32>"));
	EXPECT_NE(nullptr, metacall_handle_function(handle, "pair::<i32, i64>"));
	EXPECT_NE(nullptr, metacall_handle_template(handle, "Recursive"));
	EXPECT_NE(nullptr, metacall_handle_template(handle, "Inner"));

	EXPECT_EQ(0, metacall_clear(handle));
}
