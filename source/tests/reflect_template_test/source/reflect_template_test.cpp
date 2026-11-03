/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#include <reflect/reflect_template.h>

class reflect_template_test : public testing::Test
{
public:
};

TEST_F(reflect_template_test, DefaultConstructor)
{
	reflect_template tpl = template_create("identity", TEMPLATE_TYPE_FUNCTION);

	ASSERT_NE((reflect_template)NULL, tpl);

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));

	EXPECT_EQ(1, template_parameter_count(tpl));

	EXPECT_STREQ("T", template_parameter(tpl, 0));

	EXPECT_STREQ("identity", template_name(tpl));

	EXPECT_EQ(TEMPLATE_TYPE_FUNCTION, template_type(tpl));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);

	template_argument args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { int_type } }
	};

	function f = template_instantiate_function(tpl, args, sizeof(args) / sizeof(args[0]), NULL, NULL);

	ASSERT_NE((function)NULL, f);

	EXPECT_STREQ("identity<int>", function_name(f));

	function_destroy(f);

	type_destroy(int_type);

	template_destroy(tpl);
}

TEST_F(reflect_template_test, MultipleParameters)
{
	reflect_template tpl = template_create("pair", TEMPLATE_TYPE_FUNCTION);

	ASSERT_NE((reflect_template)NULL, tpl);

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));
	EXPECT_EQ(0, template_add_parameter(tpl, "U"));

	EXPECT_EQ(2, template_parameter_count(tpl));

	EXPECT_STREQ("T", template_parameter(tpl, 0));
	EXPECT_STREQ("U", template_parameter(tpl, 1));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	type float_type = type_create(TYPE_FLOAT, "float", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);
	ASSERT_NE((type)NULL, float_type);

	template_argument args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { int_type } },
		{ "U", TEMPLATE_ARGUMENT_TYPE, { float_type } }
	};

	function f = template_instantiate_function(tpl, args, sizeof(args) / sizeof(args[0]), NULL, NULL);

	ASSERT_NE((function)NULL, f);

	EXPECT_STREQ("pair<int, float>", function_name(f));

	function_destroy(f);

	type_destroy(int_type);
	type_destroy(float_type);

	template_destroy(tpl);
}

TEST_F(reflect_template_test, NestedTemplate)
{
	/* For pair<T, U> */
	reflect_template pair_tpl = template_create("pair", TEMPLATE_TYPE_FUNCTION);

	ASSERT_NE((reflect_template)NULL, pair_tpl);

	EXPECT_EQ(0, template_add_parameter(pair_tpl, "T"));
	EXPECT_EQ(0, template_add_parameter(pair_tpl, "U"));

	/* For vector<T> */
	reflect_template vector_tpl = template_create("vector", TEMPLATE_TYPE_FUNCTION);

	ASSERT_NE((reflect_template)NULL, vector_tpl);

	EXPECT_EQ(0, template_add_parameter(vector_tpl, "T"));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	type float_type = type_create(TYPE_FLOAT, "float", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);
	ASSERT_NE((type)NULL, float_type);

	template_argument pair_args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { int_type } },
		{ "U", TEMPLATE_ARGUMENT_TYPE, { float_type } }
	};

	function pair = template_instantiate_function(pair_tpl, pair_args, sizeof(pair_args) / sizeof(pair_args[0]), NULL, NULL);

	ASSERT_NE((function)NULL, pair);

	/* Designated initializers are not available in C++17 (MSVC), so the union member is assigned explicitly */
	template_argument vector_args[1];

	vector_args[0].name = "T";
	vector_args[0].kind = TEMPLATE_ARGUMENT_TEMPLATE;
	vector_args[0].value_template = pair_tpl;

	function vector = template_instantiate_function(vector_tpl, vector_args, sizeof(vector_args) / sizeof(vector_args[0]), NULL, NULL);

	ASSERT_NE((function)NULL, vector);

	EXPECT_STREQ("vector<pair<int, float>>", function_name(vector));

	function_destroy(vector);
	function_destroy(pair);

	type_destroy(int_type);
	type_destroy(float_type);

	template_destroy(vector_tpl);
	template_destroy(pair_tpl);
}
