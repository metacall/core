/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_node_port_c_test : public testing::Test
{
public:
};

TEST_F(metacall_node_port_c_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int)0, (int)metacall_initialize());

	static const char buffer[] =
		"const assert = require('assert');\n"
		"const { metacall_load_from_file_export, metacall_value_create_ptr, metacall_value_reference, metacall_value_dereference } = require('" METACALL_NODE_PORT_PATH "');\n"
		"const { return_text, process_text, modify_double_ptr, modify_str_ptr, compare_data_value, alloc_data, alloc_data_args, set_data_value, get_data_value, free_data } = metacall_load_from_file_export('c', ['compiled.c']);\n"
		// Test strings
		"const result = return_text();\n"
		"console.log(`'${result}'`);\n"
		"assert(result == 'hello');\n"
		"console.log(result);\n"
		"process_text('test_test');\n"
		// Test return pointers
		"data_ptr = alloc_data();\n"
		"console.log(data_ptr);\n"
		"set_data_value(data_ptr, 12);\n"
		"assert(get_data_value(data_ptr) == 12);\n"
		"free_data(data_ptr);\n"
		// Test passing reference by arguments
		"double_val = 324444.0;\n"
		"double_val_ref = metacall_value_reference(double_val);\n"
		"modify_double_ptr(double_val_ref);\n"
		"double_val_deref = metacall_value_dereference(double_val_ref);\n"
		"assert(double_val_deref == 111.0);\n"
		// Test passing reference by arguments string
		"str_val = 'asd';\n"
		"str_val_ref = metacall_value_reference(str_val);\n"
		"console.log(str_val);\n"
		"console.log(str_val_ref);\n"
		"modify_str_ptr(str_val_ref);\n"
		"str_val_deref = metacall_value_dereference(str_val_ref);\n"
		"assert(str_val_deref == 'yeet');\n"
		// Test passing reference of structs by arguments (with no args on create ptr)
		"data_ptr = metacall_value_create_ptr();\n"
		"data_ptr_ref = metacall_value_reference(data_ptr);\n"
		"console.log(data_ptr);\n"
		"console.log(data_ptr_ref);\n"
		"alloc_data_args(data_ptr_ref);\n"
		"alloc_data_ptr = metacall_value_dereference(data_ptr_ref);\n"
		"console.log(alloc_data_ptr);\n"
		"set_data_value(alloc_data_ptr, 12);\n"
		"assert(get_data_value(alloc_data_ptr) == 12);\n"
		"free_data(alloc_data_ptr);\n"
		// Test passing reference of structs by arguments (with undefined arg on create ptr)
		"data_ptr = metacall_value_create_ptr(undefined);\n"
		"data_ptr_ref = metacall_value_reference(data_ptr);\n"
		"console.log(data_ptr);\n"
		"console.log(data_ptr_ref);\n"
		"alloc_data_args(data_ptr_ref);\n"
		"alloc_data_ptr = metacall_value_dereference(data_ptr_ref);\n"
		"console.log(alloc_data_ptr);\n"
		"set_data_value(alloc_data_ptr, 12);\n"
		"assert(get_data_value(alloc_data_ptr) == 12);\n"
		"free_data(alloc_data_ptr);\n"
		// Test passing reference of structs by arguments (with another pointer arg on create ptr)
		"data_ptr = metacall_value_create_ptr(undefined);\n"
		"copy_data_ptr = metacall_value_create_ptr(data_ptr);\n"
		"console.log(data_ptr);\n"
		"console.log(copy_data_ptr);\n"
		"assert(compare_data_value(data_ptr, copy_data_ptr));\n"
		"data_ptr_ref = metacall_value_reference(copy_data_ptr);\n"
		"console.log(data_ptr);\n"
		"console.log(data_ptr_ref);\n"
		"alloc_data_args(data_ptr_ref);\n"
		"alloc_data_ptr = metacall_value_dereference(data_ptr_ref);\n"
		"console.log(alloc_data_ptr);\n"
		"set_data_value(alloc_data_ptr, 12);\n"
		"assert(get_data_value(alloc_data_ptr) == 12);\n"
		"free_data(alloc_data_ptr);\n"
		"\n";

	ASSERT_EQ((int)0, (int)metacall_load_from_memory("node", buffer, sizeof(buffer), NULL));

	metacall_destroy();
}
