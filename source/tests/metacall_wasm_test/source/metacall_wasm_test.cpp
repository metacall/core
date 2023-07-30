/*
 *	WebAssembly Loader Tests by Parra Studios
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
#include <metacall/metacall_value.h>

class metacall_wasm_test : public testing::Test
{
protected:
	void SetUp() override
	{
		metacall_initialize();
	}

	void TearDown() override
	{
		metacall_destroy();
	}
};

TEST_F(metacall_wasm_test, InitializeAndDestroy)
{
	// This is extremely hacky and causes an error when loading the buffer,
	// since it is invalid. However, it is currently impossible to initialize a
	// loader without attempting to load a handle with it. Ideally, there would
	// be a `metacall_initialize_loader` function or similar.
	const char dummy_buffer[1] = { 0 };
	metacall_load_from_memory("wasm", dummy_buffer, 1, NULL);

	ASSERT_EQ((int)0, (int)metacall_is_initialized("wasm"));

	// `metacall_destroy` does nothing if Metacall is not initialized, so we
	// can safely call it here even though we also call it during tear-down.
	ASSERT_EQ((int)0, (int)metacall_destroy());
}

/* TODO: Address sanitizer seems to break with WASM loading, we should either review this or instrument properly wasmtime library */
#if !defined(__ADDRESS_SANITIZER__)
TEST_F(metacall_wasm_test, LoadBinaryFromMemory)
{
	// See https://webassembly.github.io/spec/core/binary/modules.html#binary-module
	const char empty_module[] = {
		0x00, 0x61, 0x73, 0x6d, // Magic bytes
		0x01, 0x00, 0x00, 0x00	// Version
	};
	ASSERT_EQ((int)0, (int)metacall_load_from_memory("wasm", empty_module, sizeof(empty_module), NULL));

	const char invalid_module[] = { 0 };
	ASSERT_NE((int)0, (int)metacall_load_from_memory("wasm", invalid_module, sizeof(invalid_module), NULL));
}

TEST_F(metacall_wasm_test, LoadTextFromMemory)
{
	const char *empty_module = "(module)";
	ASSERT_EQ((int)0, (int)metacall_load_from_memory("wasm", empty_module, strlen(empty_module), NULL));

	const char *invalid_module = "(invalid)";
	ASSERT_NE((int)0, (int)metacall_load_from_memory("wasm", invalid_module, strlen(invalid_module), NULL));
}

TEST_F(metacall_wasm_test, LoadFromFile)
{
	const char *empty_module_filename = "empty_module.wat";
	const char *invalid_module_filename = "invalid_module.wat";

	ASSERT_EQ((int)0, (int)metacall_load_from_file("wasm", &empty_module_filename, 1, NULL));
	ASSERT_NE((int)0, (int)metacall_load_from_file("wasm", &invalid_module_filename, 1, NULL));
}

TEST_F(metacall_wasm_test, LoadFromPackage)
{
	ASSERT_EQ((int)0, (int)metacall_load_from_package("wasm", "empty_module.wasm", NULL));
	ASSERT_NE((int)0, (int)metacall_load_from_package("wasm", "invalid_module.wasm", NULL));
}

void TestFunction(void *handle, const char *name, const std::vector<enum metacall_value_id> &expected_param_types, enum metacall_value_id expected_return_type)
{
	void *func = metacall_handle_function(handle, name);

	ASSERT_NE((void *)NULL, (void *)func);
	ASSERT_EQ((size_t)expected_param_types.size(), (size_t)metacall_function_size(func));

	for (size_t i = 0; i < expected_param_types.size(); ++i)
	{
		enum metacall_value_id param_type;
		ASSERT_EQ((int)0, (int)metacall_function_parameter_type(func, i, &param_type));
		ASSERT_EQ((enum metacall_value_id)expected_param_types[i], (enum metacall_value_id)param_type);
	}

	enum metacall_value_id return_type;
	ASSERT_EQ((int)0, (int)metacall_function_return_type(func, &return_type));
	ASSERT_EQ((enum metacall_value_id)expected_return_type, (enum metacall_value_id)return_type);
}

TEST_F(metacall_wasm_test, DiscoverFunctions)
{
	const char *functions_module_filename = "functions.wat";
	void *handle = NULL;

	ASSERT_EQ((int)0, (int)metacall_load_from_file("wasm", &functions_module_filename, 1, &handle));

	ASSERT_EQ((void *)NULL, (void *)metacall_handle_function(handle, "does_not_exist"));

	TestFunction(handle, "none_ret_none", {}, METACALL_INVALID);
	TestFunction(handle, "i64_ret_none", { METACALL_LONG }, METACALL_INVALID);
	TestFunction(handle, "i32_f32_i64_f64_ret_none", { METACALL_INT, METACALL_FLOAT, METACALL_LONG, METACALL_DOUBLE }, METACALL_INVALID);
	TestFunction(handle, "none_ret_i32", {}, METACALL_INT);

	TestFunction(handle, "none_ret_i32_f32_i64_f64", {}, METACALL_ARRAY);
	TestFunction(handle, "i32_f32_i64_f64_ret_i32_f32_i64_f64", { METACALL_INT, METACALL_FLOAT, METACALL_LONG, METACALL_DOUBLE }, METACALL_ARRAY);
}

TEST_F(metacall_wasm_test, CallFunctions)
{
	const char *functions_module_filename = "functions.wat";

	ASSERT_EQ((int)0, (int)metacall_load_from_file("wasm", &functions_module_filename, 1, NULL));

	void *ret = metacall("none_ret_none");
	ASSERT_NE((void *)NULL, (void *)ret);
	ASSERT_EQ((enum metacall_value_id)METACALL_NULL, (enum metacall_value_id)metacall_value_id(ret));
	metacall_value_destroy(ret);

	ret = metacall("i64_ret_none", 0L);
	ASSERT_NE((void *)NULL, (void *)ret);
	ASSERT_EQ((enum metacall_value_id)METACALL_NULL, (enum metacall_value_id)metacall_value_id(ret));
	metacall_value_destroy(ret);

	ret = metacall("i32_f32_i64_f64_ret_none", 0, 0.0f, 0L, 0.0);
	ASSERT_NE((void *)NULL, (void *)ret);
	ASSERT_EQ((enum metacall_value_id)METACALL_NULL, (enum metacall_value_id)metacall_value_id(ret));
	metacall_value_destroy(ret);

	ret = metacall("none_ret_i32");
	ASSERT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(ret));
	ASSERT_EQ((int)1, (int)metacall_value_to_int(ret));
	metacall_value_destroy(ret);

	ret = metacall("none_ret_i32_f32_i64_f64");
	ASSERT_EQ((enum metacall_value_id)METACALL_ARRAY, (enum metacall_value_id)metacall_value_id(ret));

	void **values = metacall_value_to_array(ret);
	ASSERT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(values[0]));
	ASSERT_EQ((enum metacall_value_id)METACALL_FLOAT, (enum metacall_value_id)metacall_value_id(values[1]));
	ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(values[2]));
	ASSERT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(values[3]));
	ASSERT_EQ((int)1, (int)metacall_value_to_int(values[0]));
	ASSERT_EQ((float)2.0f, (float)metacall_value_to_float(values[1]));
	ASSERT_EQ((long)3, (long)metacall_value_to_long(values[2]));
	ASSERT_EQ((double)4.0, (double)metacall_value_to_double(values[3]));
	metacall_value_destroy(ret);

	ret = metacall("i32_f32_i64_f64_ret_i32_f32_i64_f64", 0, 0, 0, 0);
	ASSERT_EQ((enum metacall_value_id)METACALL_ARRAY, (enum metacall_value_id)metacall_value_id(ret));

	values = metacall_value_to_array(ret);
	ASSERT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(values[0]));
	ASSERT_EQ((enum metacall_value_id)METACALL_FLOAT, (enum metacall_value_id)metacall_value_id(values[1]));
	ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(values[2]));
	ASSERT_EQ((enum metacall_value_id)METACALL_DOUBLE, (enum metacall_value_id)metacall_value_id(values[3]));
	ASSERT_EQ((int)1, (int)metacall_value_to_int(values[0]));
	ASSERT_EQ((float)2.0f, (float)metacall_value_to_float(values[1]));
	ASSERT_EQ((long)3, (long)metacall_value_to_long(values[2]));
	ASSERT_EQ((double)4.0, (double)metacall_value_to_double(values[3]));
	metacall_value_destroy(ret);

	// It should exit with illegal instruction
	#if defined(unix) || defined(__unix__) || defined(__unix) || \
		defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux)
	ASSERT_EXIT((metacall("trap"), exit(0)), ::testing::KilledBySignal(SIGILL), ".*");
	#endif
}

TEST_F(metacall_wasm_test, LinkModules)
{
	const char *modules[] = {
		"exports1.wat",
		"exports2.wat",
		"imports.wat"
	};

	ASSERT_EQ((int)0, (int)metacall_load_from_file("wasm", modules, sizeof(modules) / sizeof(modules[0]), NULL));

	void *ret = metacall("duplicate_func_i32");
	ASSERT_EQ((enum metacall_value_id)METACALL_INT, (enum metacall_value_id)metacall_value_id(ret));
	ASSERT_EQ((int)1, (int)metacall_value_to_int(ret));
	metacall_value_destroy(ret);

	ret = metacall("duplicate_func_i64");
	ASSERT_EQ((enum metacall_value_id)METACALL_LONG, (enum metacall_value_id)metacall_value_id(ret));
	ASSERT_EQ((long)2, (long)metacall_value_to_long(ret));
	metacall_value_destroy(ret);
}

TEST_F(metacall_wasm_test, InvalidLinkModules)
{
	const char *modules[] = {
		"exports1.wat",
		"imports.wat"
	};

	ASSERT_EQ((int)1, (int)metacall_load_from_file("wasm", modules, sizeof(modules) / sizeof(modules[0]), NULL));
}
#endif
