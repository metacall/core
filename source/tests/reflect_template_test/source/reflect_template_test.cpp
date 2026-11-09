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
	int instantiate;
	int instance_destroy;
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
		else if (args[iterator].kind == TEMPLATE_ARGUMENT_CLASS)
		{
			name += class_name(args[iterator].data.cls);
		}
		else
		{
			name += template_name(args[iterator].data.tpl);
		}
	}

	name += ">";

	return name;
}

/* Mock of the interface of the instances, it counts the destroyed instances */
void template_test_function_interface_destroy(function func, function_impl impl)
{
	(void)func;

	if (impl != NULL)
	{
		++static_cast<template_test_impl *>(impl)->instance_destroy;
	}
}

function_interface template_test_function_singleton()
{
	static struct function_interface_type test_interface = {
		NULL,
		NULL,
		NULL,
		&template_test_function_interface_destroy
	};

	return &test_interface;
}

void template_test_class_interface_destroy(klass cls, class_impl impl)
{
	(void)cls;

	if (impl != NULL)
	{
		++static_cast<template_test_impl *>(impl)->instance_destroy;
	}
}

class_interface template_test_class_singleton()
{
	static struct class_interface_type test_interface = {
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&template_test_class_interface_destroy
	};

	return &test_interface;
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

value template_test_interface_instantiate(reflect_template tpl, template_impl impl, template_argument args[], size_t size)
{
	std::string name = template_test_instance_name(tpl, args, size);

	if (impl != NULL)
	{
		++static_cast<template_test_impl *>(impl)->instantiate;
	}

	/* The name is copied by function_create and class_create, the value holds the reference to the instance */
	if (template_type(tpl) == TYPE_FUNCTION)
	{
		return value_create_function(function_create(name.c_str(), size, impl, &template_test_function_singleton));
	}

	return value_create_class(class_create(name.c_str(), ACCESSOR_TYPE_STATIC, impl, &template_test_class_singleton));
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
		&template_test_interface_instantiate,
		&template_test_interface_destroy
	};

	return &test_interface;
}

/* Unwrap the value returned by template_instantiate, checking that it has the type of the template */
static function template_test_function(value v)
{
	if (v == NULL)
	{
		return NULL;
	}

	EXPECT_EQ((type_id)TYPE_FUNCTION, value_type_id(v));

	return value_to_function(v);
}

static klass template_test_class(value v)
{
	if (v == NULL)
	{
		return NULL;
	}

	EXPECT_EQ((type_id)TYPE_CLASS, value_type_id(v));

	return value_to_class(v);
}

/* Mock of an interface which returns a value of the wrong type (a class for a function template) */
value template_test_wrong_interface_instantiate(reflect_template tpl, template_impl impl, template_argument args[], size_t size)
{
	std::string name = template_test_instance_name(tpl, args, size);

	if (impl != NULL)
	{
		++static_cast<template_test_impl *>(impl)->instantiate;
	}

	return value_create_class(class_create(name.c_str(), ACCESSOR_TYPE_STATIC, impl, &template_test_class_singleton));
}

template_interface template_test_wrong_singleton()
{
	static struct template_interface_type test_interface = {
		&template_test_interface_create,
		&template_test_wrong_interface_instantiate,
		&template_test_interface_destroy
	};

	return &test_interface;
}

TEST_F(reflect_template_test, DefaultConstructor)
{
	template_test_impl impl = { 0, 0, 0, 0 };

	reflect_template tpl = template_create("identity", TYPE_FUNCTION, &impl, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(tpl));

	EXPECT_EQ(1, impl.create);

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));

	EXPECT_EQ((size_t)1, template_parameter_count(tpl));

	EXPECT_STREQ("T", template_parameter(tpl, 0));

	EXPECT_STREQ("identity", template_name(tpl));

	EXPECT_EQ(TYPE_FUNCTION, template_type(tpl));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);

	template_argument args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { int_type } }
	};

	/* The function is owned by the template */
	function f = template_test_function(template_instantiate(tpl, args, sizeof(args) / sizeof(args[0])));

	ASSERT_NE((function)NULL, f);

	EXPECT_EQ(1, impl.instantiate);

	EXPECT_STREQ("identity<int>", function_name(f));

	template_destroy(tpl);

	EXPECT_EQ(1, impl.instance_destroy);
	EXPECT_EQ(1, impl.destroy);

	type_destroy(int_type);
}

TEST_F(reflect_template_test, MultipleParameters)
{
	reflect_template tpl = template_create("pair", TYPE_FUNCTION, NULL, &template_test_singleton);

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

	function f = template_test_function(template_instantiate(tpl, args, sizeof(args) / sizeof(args[0])));

	ASSERT_NE((function)NULL, f);

	EXPECT_STREQ("pair<int, float>", function_name(f));

	template_destroy(tpl);

	type_destroy(int_type);
	type_destroy(float_type);
}

TEST_F(reflect_template_test, DuplicatedParameter)
{
	reflect_template tpl = template_create("pair", TYPE_FUNCTION, NULL, &template_test_singleton);

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
	template_test_impl impl = { 0, 0, 0, 0 };

	reflect_template tpl = template_create("pair", TYPE_FUNCTION, &impl, &template_test_singleton);

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
	EXPECT_EQ((value)NULL, template_instantiate(tpl, args, 1));

	/* Wrong parameter name */
	EXPECT_EQ((value)NULL, template_instantiate(tpl, args, sizeof(args) / sizeof(args[0])));

	/* Invalid template type */
	EXPECT_EQ((reflect_template)NULL, template_create("invalid", TYPE_INVALID, NULL, &template_test_singleton));

	/* The loader is not called with invalid arguments */
	EXPECT_EQ(0, impl.instantiate);

	type_destroy(int_type);

	template_destroy(tpl);
}

TEST_F(reflect_template_test, ClassTemplate)
{
	template_test_impl impl = { 0, 0, 0, 0 };

	reflect_template tpl = template_create("box", TYPE_CLASS, &impl, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(tpl));

	EXPECT_EQ(TYPE_CLASS, template_type(tpl));

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);

	template_argument args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { int_type } }
	};

	/* The class is owned by the template */
	klass cls = template_test_class(template_instantiate(tpl, args, sizeof(args) / sizeof(args[0])));

	ASSERT_NE((klass)NULL, cls);

	EXPECT_STREQ("box<int>", class_name(cls));

	/* Same arguments, same class */
	EXPECT_EQ(cls, template_test_class(template_instantiate(tpl, args, sizeof(args) / sizeof(args[0]))));

	EXPECT_EQ(1, impl.instantiate);

	template_destroy(tpl);

	EXPECT_EQ(1, impl.instance_destroy);
	EXPECT_EQ(1, impl.destroy);

	type_destroy(int_type);
}

TEST_F(reflect_template_test, InstanceCache)
{
	template_test_impl impl = { 0, 0, 0, 0 };

	reflect_template tpl = template_create("identity", TYPE_FUNCTION, &impl, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(tpl));

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	type float_type = type_create(TYPE_FLOAT, "float", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);
	ASSERT_NE((type)NULL, float_type);

	template_argument int_args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { int_type } }
	};

	template_argument float_args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { float_type } }
	};

	function int_f = template_test_function(template_instantiate(tpl, int_args, sizeof(int_args) / sizeof(int_args[0])));

	ASSERT_NE((function)NULL, int_f);

	EXPECT_EQ(1, impl.instantiate);

	/* Same arguments, the loader is not called again */
	EXPECT_EQ(int_f, template_test_function(template_instantiate(tpl, int_args, sizeof(int_args) / sizeof(int_args[0]))));

	EXPECT_EQ(1, impl.instantiate);

	/* Different arguments, new instance */
	function float_f = template_test_function(template_instantiate(tpl, float_args, sizeof(float_args) / sizeof(float_args[0])));

	ASSERT_NE((function)NULL, float_f);

	EXPECT_NE(int_f, float_f);

	EXPECT_EQ(2, impl.instantiate);

	EXPECT_STREQ("identity<float>", function_name(float_f));

	/* Same pointer with a different kind is a different argument */
	klass cls = class_create("Point", ACCESSOR_TYPE_STATIC, NULL, NULL);

	ASSERT_NE((klass)NULL, cls);

	/* The caller holds a reference, released by class_destroy */
	EXPECT_EQ(0, class_increment_reference(cls));

	template_argument class_args[1];

	class_args[0].name = "T";
	class_args[0].kind = TEMPLATE_ARGUMENT_CLASS;
	class_args[0].data.cls = cls;

	function class_f = template_test_function(template_instantiate(tpl, class_args, sizeof(class_args) / sizeof(class_args[0])));

	ASSERT_NE((function)NULL, class_f);

	EXPECT_EQ(3, impl.instantiate);

	EXPECT_EQ(class_f, template_test_function(template_instantiate(tpl, class_args, sizeof(class_args) / sizeof(class_args[0]))));

	EXPECT_EQ(3, impl.instantiate);

	/* All the instances are released with the template */
	template_destroy(tpl);

	EXPECT_EQ(3, impl.instance_destroy);
	EXPECT_EQ(1, impl.destroy);

	class_destroy(cls);

	type_destroy(int_type);
	type_destroy(float_type);
}

TEST_F(reflect_template_test, InstanceOutlivesTemplate)
{
	template_test_impl impl = { 0, 0, 0, 0 };

	reflect_template tpl = template_create("identity", TYPE_FUNCTION, &impl, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(tpl));

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);

	template_argument args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { int_type } }
	};

	function f = template_test_function(template_instantiate(tpl, args, sizeof(args) / sizeof(args[0])));

	ASSERT_NE((function)NULL, f);

	/* The caller holds its own reference, released by function_destroy */
	EXPECT_EQ(0, function_increment_reference(f));

	/* The template releases its reference, the function survives */
	template_destroy(tpl);

	EXPECT_EQ(1, impl.destroy);
	EXPECT_EQ(0, impl.instance_destroy);

	EXPECT_STREQ("identity<int>", function_name(f));

	/* The last reference is released by the caller */
	function_destroy(f);

	EXPECT_EQ(1, impl.instance_destroy);

	type_destroy(int_type);
}

TEST_F(reflect_template_test, ValueTemplate)
{
	template_test_impl impl = { 0, 0, 0, 0 };

	reflect_template tpl = template_create("identity", TYPE_FUNCTION, &impl, &template_test_singleton);

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
	reflect_template pair_tpl = template_create("pair", TYPE_FUNCTION, NULL, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, pair_tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(pair_tpl));

	EXPECT_EQ(0, template_add_parameter(pair_tpl, "T"));
	EXPECT_EQ(0, template_add_parameter(pair_tpl, "U"));

	/* For vector<T> */
	reflect_template vector_tpl = template_create("vector", TYPE_FUNCTION, NULL, &template_test_singleton);

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

	function pair = template_test_function(template_instantiate(pair_tpl, pair_args, sizeof(pair_args) / sizeof(pair_args[0])));

	ASSERT_NE((function)NULL, pair);

	EXPECT_STREQ("pair<int, float>", function_name(pair));

	template_argument vector_args[1];

	vector_args[0].name = "T";
	vector_args[0].kind = TEMPLATE_ARGUMENT_TEMPLATE;
	vector_args[0].data.tpl = pair_tpl;

	function vector = template_test_function(template_instantiate(vector_tpl, vector_args, sizeof(vector_args) / sizeof(vector_args[0])));

	ASSERT_NE((function)NULL, vector);

	/* A template argument only carries the template, not its instantiation, so the arguments of pair are not available */
	EXPECT_STREQ("vector<pair>", function_name(vector));

	template_destroy(vector_tpl);
	template_destroy(pair_tpl);

	type_destroy(int_type);
	type_destroy(float_type);
}

TEST_F(reflect_template_test, ClassArgument)
{
	reflect_template tpl = template_create("pair", TYPE_FUNCTION, NULL, &template_test_singleton);

	ASSERT_NE((reflect_template)NULL, tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(tpl));

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));
	EXPECT_EQ(0, template_add_parameter(tpl, "U"));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);

	klass point_cls = class_create("Point", ACCESSOR_TYPE_STATIC, NULL, NULL);

	ASSERT_NE((klass)NULL, point_cls);

	/* The caller holds a reference, released by class_destroy */
	EXPECT_EQ(0, class_increment_reference(point_cls));

	template_argument args[2];

	/* A class and a type mixed */
	args[0].name = "T";
	args[0].kind = TEMPLATE_ARGUMENT_CLASS;
	args[0].data.cls = point_cls;

	args[1].name = "U";
	args[1].kind = TEMPLATE_ARGUMENT_TYPE;
	args[1].data.t = int_type;

	function mixed = template_test_function(template_instantiate(tpl, args, sizeof(args) / sizeof(args[0])));

	ASSERT_NE((function)NULL, mixed);

	EXPECT_STREQ("pair<Point, int>", function_name(mixed));

	/* Only classes */
	args[1].kind = TEMPLATE_ARGUMENT_CLASS;
	args[1].data.cls = point_cls;

	function classes = template_test_function(template_instantiate(tpl, args, sizeof(args) / sizeof(args[0])));

	ASSERT_NE((function)NULL, classes);

	EXPECT_STREQ("pair<Point, Point>", function_name(classes));

	/* NULL class */
	args[1].data.cls = NULL;

	EXPECT_EQ((value)NULL, template_instantiate(tpl, args, sizeof(args) / sizeof(args[0])));

	/* Invalid kind (out of the range of template_argument_id) */
	args[1].kind = (enum template_argument_id)0xFF;
	args[1].data.cls = point_cls;

	EXPECT_EQ((value)NULL, template_instantiate(tpl, args, sizeof(args) / sizeof(args[0])));

	template_destroy(tpl);

	class_destroy(point_cls);

	type_destroy(int_type);
}

TEST_F(reflect_template_test, WrongInstanceType)
{
	template_test_impl impl = { 0, 0, 0, 0 };

	reflect_template tpl = template_create("identity", TYPE_FUNCTION, &impl, &template_test_wrong_singleton);

	ASSERT_NE((reflect_template)NULL, tpl);

	/* The creator holds a reference, released by template_destroy */
	EXPECT_EQ(0, template_increment_reference(tpl));

	EXPECT_EQ(0, template_add_parameter(tpl, "T"));

	type int_type = type_create(TYPE_INT, "int", NULL, NULL);

	ASSERT_NE((type)NULL, int_type);

	template_argument args[] = {
		{ "T", TEMPLATE_ARGUMENT_TYPE, { int_type } }
	};

	/* The loader returns a class for a function template, so the value is destroyed */
	EXPECT_EQ((value)NULL, template_instantiate(tpl, args, sizeof(args) / sizeof(args[0])));

	EXPECT_EQ(1, impl.instantiate);
	EXPECT_EQ(1, impl.instance_destroy);

	/* It is not cached, so the loader is called again */
	EXPECT_EQ((value)NULL, template_instantiate(tpl, args, sizeof(args) / sizeof(args[0])));

	EXPECT_EQ(2, impl.instantiate);
	EXPECT_EQ(2, impl.instance_destroy);

	template_destroy(tpl);

	EXPECT_EQ(1, impl.destroy);

	type_destroy(int_type);
}
