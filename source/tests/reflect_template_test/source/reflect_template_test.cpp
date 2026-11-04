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
#include <reflect/reflect_value_type.h>

#include <string>

class reflect_template_test : public testing::Test
{
public:
};

/* Mock of the interface implemented by a loader */
typedef struct template_test_impl_type
{
	int create;
	int destroy;
} template_test_impl;

static std::string template_test_instance_name(reflect_template tpl, template_argument args[], size_t size)
{
	std::string name(template_name(tpl));

	name += "<";

	for (size_t iterator = 0; iterator < size; ++iterator)
	{
		if (iterator > 0)
		{
			name += ", ";
		}

		if (args[iterator].kind == TEMPLATE_ARGUMENT_TYPE)
		{
			name += type_name(args[iterator].data.t);
		}
		else
		{
			name += template_name(args[iterator].data.tpl);
		}
	}

	name += ">";

	return name;
}

int template_test_interface_create(reflect_template tpl, template_impl impl)
{
	(void)tpl;

	if (impl != NULL)
	{
		++static_cast<template_test_impl *>(impl)->create;
	}

	return 0;
}

function template_test_interface_instantiate_function(reflect_template tpl, template_impl impl, template_argument args[], size_t size)
{
	std::string name = template_test_instance_name(tpl, args, size);

	(void)impl;

	/* The name is copied by function_create */
	return function_create(name.c_str(), size, NULL, NULL);
}

klass template_test_interface_instantiate_class(reflect_template tpl, template_impl impl, template_argument args[], size_t size)
{
	std::string name = template_test_instance_name(tpl, args, size);

	(void)impl;

	/* The name is copied by class_create */
	return class_create(name.c_str(), ACCESSOR_TYPE_STATIC, NULL, NULL);
}

void template_test_interface_destroy(reflect_template tpl, template_impl impl)
{
	(void)tpl;

	if (impl != NULL)
	{
		++static_cast<template_test_impl *>(impl)->destroy;
	}
}

template_interface template_test_singleton()
{
	static struct template_interface_type test_interface = {
		&template_test_interface_create,
		&template_test_interface_instantiate_function,
		&template_test_interface_instantiate_class,
		&template_test_interface_destroy
	};

	return &test_interface;
}

TEST_F(reflect_template_test, DefaultConstructor)
{
	template_test_impl impl = { 0, 0 };

	reflect_template tpl = template_create("identity", TEMPLATE_TYPE_FUNCTION, &impl, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(tpl));

	EXPECT_EQ(1, impl.create);

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));

	EXPECT_EQ((size_t)1, template_parameter_count(tpl));

	EXPECT_STREQ("T", template_parameter(tpl, 0));

	EXPECT_STREQ("identity", template_name(tpl));

	EXPECT_EQ(TEMPLATE_TYPE_FUNCTION, template_type(tpl));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);

	template_argument args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { int_type } }
	};

	function f = template_instantiate_function(tpl, args, sizeof(args) / sizeof(args[0]));

	ASSERT_NE((function)NULL, f);

	/* The caller holds a reference, released by function_destroy */
	EXPECT_EQ(0, function_increment_reference(f));

	EXPECT_STREQ("identity<int>", function_name(f));

	EXPECT_EQ(tpl, function_template(f));

	function_destroy(f);

	type_destroy(int_type);

	template_destroy(tpl);

	EXPECT_EQ(1, impl.destroy);
}

TEST_F(reflect_template_test, MultipleParameters)
{
	reflect_template tpl = template_create("pair", TEMPLATE_TYPE_FUNCTION, NULL, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(tpl));

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));
	EXPECT_EQ(0, template_add_parameter(tpl, "U"));

	EXPECT_EQ((size_t)2, template_parameter_count(tpl));

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

	function f = template_instantiate_function(tpl, args, sizeof(args) / sizeof(args[0]));

	ASSERT_NE((function)NULL, f);

	/* The caller holds a reference, released by function_destroy */
	EXPECT_EQ(0, function_increment_reference(f));

	EXPECT_STREQ("pair<int, float>", function_name(f));

	function_destroy(f);

	type_destroy(int_type);
	type_destroy(float_type);

	template_destroy(tpl);
}

TEST_F(reflect_template_test, DuplicatedParameter)
{
	reflect_template tpl = template_create("pair", TEMPLATE_TYPE_FUNCTION, NULL, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(tpl));

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));
	EXPECT_EQ(1, template_add_parameter(tpl, "T"));
	EXPECT_EQ(0, template_add_parameter(tpl, "U"));
	EXPECT_EQ(1, template_add_parameter(tpl, "U"));

	EXPECT_EQ((size_t)2, template_parameter_count(tpl));

	EXPECT_STREQ("T", template_parameter(tpl, 0));
	EXPECT_STREQ("U", template_parameter(tpl, 1));

	template_destroy(tpl);
}

TEST_F(reflect_template_test, InvalidArguments)
{
	reflect_template tpl = template_create("pair", TEMPLATE_TYPE_FUNCTION, NULL, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(tpl));

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));
	EXPECT_EQ(0, template_add_parameter(tpl, "U"));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);

	template_argument args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { int_type } },
		{ "V", TEMPLATE_ARGUMENT_TYPE, { int_type } }
	};

	/* Wrong number of arguments */
	EXPECT_EQ((function)NULL, template_instantiate_function(tpl, args, 1));

	/* Wrong parameter name */
	EXPECT_EQ((function)NULL, template_instantiate_function(tpl, args, sizeof(args) / sizeof(args[0])));

	/* Wrong template type */
	EXPECT_EQ((klass)NULL, template_instantiate_class(tpl, args, sizeof(args) / sizeof(args[0])));

	/* Invalid template type */
	EXPECT_EQ((reflect_template)NULL, template_create("invalid", TEMPLATE_TYPE_INVALID, NULL, &template_test_singleton));

	type_destroy(int_type);

	template_destroy(tpl);
}

TEST_F(reflect_template_test, ClassTemplate)
{
	reflect_template tpl = template_create("box", TEMPLATE_TYPE_CLASS, NULL, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(tpl));

	EXPECT_EQ(TEMPLATE_TYPE_CLASS, template_type(tpl));

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);

	template_argument args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { int_type } }
	};

	EXPECT_EQ((function)NULL, template_instantiate_function(tpl, args, sizeof(args) / sizeof(args[0])));

	klass cls = template_instantiate_class(tpl, args, sizeof(args) / sizeof(args[0]));

	ASSERT_NE((klass)NULL, cls);

	/* The caller holds a reference, released by class_destroy */
	EXPECT_EQ(0, class_increment_reference(cls));

	EXPECT_STREQ("box<int>", class_name(cls));

	EXPECT_EQ(tpl, class_template(cls));

	class_destroy(cls);

	type_destroy(int_type);

	template_destroy(tpl);
}

TEST_F(reflect_template_test, InstanceKeepsTemplateAlive)
{
	template_test_impl impl = { 0, 0 };

	reflect_template tpl = template_create("identity", TEMPLATE_TYPE_FUNCTION, &impl, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, tpl);

	EXPECT_EQ(0, template_increment_reference(tpl));

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);

	template_argument args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { int_type } }
	};

	function f = template_instantiate_function(tpl, args, sizeof(args) / sizeof(args[0]));

	ASSERT_NE((function)NULL, f);

	/* The caller holds a reference, released by function_destroy */
	EXPECT_EQ(0, function_increment_reference(f));

	/* Release the reference of the creator, the function still holds one */
	template_destroy(tpl);

	EXPECT_EQ(0, impl.destroy);

	EXPECT_EQ(tpl, function_template(f));

	EXPECT_STREQ("identity", template_name(function_template(f)));

	/* The last reference is released when the function is destroyed */
	function_destroy(f);

	EXPECT_EQ(1, impl.destroy);

	type_destroy(int_type);
}

TEST_F(reflect_template_test, ValueTemplate)
{
	template_test_impl impl = { 0, 0 };

	reflect_template tpl = template_create("identity", TEMPLATE_TYPE_FUNCTION, &impl, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, tpl);

	/* The value holds the reference instead of the creator */
	value v = value_create_template(tpl);

	ASSERT_NE((value)NULL, v);

	EXPECT_EQ((type_id)TYPE_TEMPLATE, value_type_id(v));

	EXPECT_EQ(tpl, value_to_template(v));

	value cpy = value_type_copy(v);

	ASSERT_NE((value)NULL, cpy);

	EXPECT_EQ(tpl, value_to_template(cpy));

	value_type_destroy(v);

	EXPECT_EQ(0, impl.destroy);

	/* The last reference is released when the copy is destroyed */
	value_type_destroy(cpy);

	EXPECT_EQ(1, impl.destroy);
}

TEST_F(reflect_template_test, NestedTemplate)
{
	/* For pair<T, U> */
	reflect_template pair_tpl = template_create("pair", TEMPLATE_TYPE_FUNCTION, NULL, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, pair_tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(pair_tpl));

	EXPECT_EQ(0, template_add_parameter(pair_tpl, "T"));
	EXPECT_EQ(0, template_add_parameter(pair_tpl, "U"));

	/* For vector<T> */
	reflect_template vector_tpl = template_create("vector", TEMPLATE_TYPE_FUNCTION, NULL, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, vector_tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(vector_tpl));

	EXPECT_EQ(0, template_add_parameter(vector_tpl, "T"));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	type float_type = type_create(TYPE_FLOAT, "float", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);
	ASSERT_NE((type)NULL, float_type);

	template_argument pair_args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { int_type } },
		{ "U", TEMPLATE_ARGUMENT_TYPE, { float_type } }
	};

	function pair = template_instantiate_function(pair_tpl, pair_args, sizeof(pair_args) / sizeof(pair_args[0]));

	ASSERT_NE((function)NULL, pair);

	/* The caller holds a reference, released by function_destroy */
	EXPECT_EQ(0, function_increment_reference(pair));

	EXPECT_STREQ("pair<int, float>", function_name(pair));

	/* Designated initializers are not available in C++17 (MSVC), so the union member is assigned explicitly */
	template_argument vector_args[1];

	vector_args[0].name = "T";
	vector_args[0].kind = TEMPLATE_ARGUMENT_TEMPLATE;
	vector_args[0].data.tpl = pair_tpl;

	function vector = template_instantiate_function(vector_tpl, vector_args, sizeof(vector_args) / sizeof(vector_args[0]));

	ASSERT_NE((function)NULL, vector);

	/* The caller holds a reference, released by function_destroy */
	EXPECT_EQ(0, function_increment_reference(vector));

	/* A template argument only carries the template, not its instantiation, so the arguments of pair are not available */
	EXPECT_STREQ("vector<pair>", function_name(vector));

	function_destroy(vector);
	function_destroy(pair);

	type_destroy(int_type);
	type_destroy(float_type);

	template_destroy(vector_tpl);
	template_destroy(pair_tpl);
}
