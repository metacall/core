#include <gtest/gtest.h>

#include <reflect/reflect_template.h>

class reflect_template_test : public testing::Test
{
public:
};

TEST_F(reflect_template_test, DefaultConstructor)
{
	reflect_template tpl = reflect_template_create(
		"identity",
		TEMPLATE_TYPE_FUNCTION
	);

	EXPECT_NE((reflect_template)NULL, tpl);

	EXPECT_EQ(
		0,
		reflect_template_add_parameter(tpl, "T")
	);

	EXPECT_EQ(
		1,
		reflect_template_parameter_count(tpl)
	);

	EXPECT_STREQ(
		"T",
		reflect_template_parameter(tpl, 0)
	);

	EXPECT_STREQ(
		"identity",
		reflect_template_name(tpl)
	);

	EXPECT_EQ(
		TEMPLATE_TYPE_FUNCTION,
		reflect_template_type(tpl)
	);

type int_type = type_create(
    TYPE_INT,
    "int",
    NULL,
    NULL
);

EXPECT_NE((type)NULL, int_type);

template_argument args[] =
{
    {
        "T",
        int_type
    }
};

function f =
    reflect_template_instantiate_function(
        tpl,
        args,
        1
    );

	EXPECT_NE((function)NULL, f);

	EXPECT_STREQ(
    "identity<int>",
    function_name(f)
);

	function_destroy(f);

	reflect_template_destroy(tpl);

	type_destroy(int_type);
}