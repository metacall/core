#include <gtest/gtest.h>

#include <reflect/reflect_template.h>

class reflect_template_test : public testing::Test
{
public:
};

TEST_F(reflect_template_test, DefaultConstructor)
{
	reflect_template tpl = template_create(
		"identity",
		TEMPLATE_TYPE_FUNCTION);

	ASSERT_NE((reflect_template)NULL, tpl);

	EXPECT_EQ(
		0,
		template_add_parameter(tpl, "T"));

	EXPECT_EQ(
		1,
		template_parameter_count(tpl));

	EXPECT_STREQ(
		"T",
		template_parameter(tpl, 0));

	EXPECT_STREQ(
		"identity",
		template_name(tpl));

	EXPECT_EQ(
		TEMPLATE_TYPE_FUNCTION,
		template_type(tpl));

	type int_type = type_create(
		TYPE_INT,
		"int",
		NULL,
		NULL);

	ASSERT_NE((type)NULL, int_type);

	template_argument args[] = {
		{
			"T",
			int_type
		}
	};

	function f = template_instantiate_function(
		tpl,
		args,
		1);

	ASSERT_NE((function)NULL, f);

	EXPECT_STREQ(
		"identity<int>",
		function_name(f));

	function_destroy(f);

	type_destroy(int_type);

	template_destroy(tpl);
}