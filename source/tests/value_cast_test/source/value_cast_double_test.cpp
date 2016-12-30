/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <gmock/gmock.h>

#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_cast.h>

class value_cast_double_test : public testing::Test
{
  public:
};

TEST_F(value_cast_double_test, double_to_bool_cast)
{
	value v = value_create_double(78.31245);

	boolean b = 1L; /* Because C compiler cast doesn't convert 78.31245 to 1L */

	v = value_type_cast(v, TYPE_BOOL);

	EXPECT_EQ((boolean) b, (boolean) value_to_bool(v));

	value_destroy(v);
}

TEST_F(value_cast_double_test, double_to_char_cast)
{
	value v = value_create_double(78.31245);

	double d = value_to_double(v);

	v = value_type_cast(v, TYPE_CHAR);

	EXPECT_EQ((char) d, (char) value_to_char(v));

	value_destroy(v);
}

TEST_F(value_cast_double_test, double_to_short_cast)
{
	value v = value_create_double(78.31245);

	double d = value_to_double(v);

	v = value_type_cast(v, TYPE_SHORT);

	EXPECT_EQ((short) d, (short) value_to_short(v));

	value_destroy(v);
}

TEST_F(value_cast_double_test, double_to_int_cast)
{
	value v = value_create_double(78.31245);

	double d = value_to_double(v);

	v = value_type_cast(v, TYPE_INT);

	EXPECT_EQ((int) d, (int) value_to_int(v));

	value_destroy(v);
}

TEST_F(value_cast_double_test, double_to_long_cast)
{
	value v = value_create_double(78.31245);

	double d = value_to_double(v);

	v = value_type_cast(v, TYPE_LONG);

	EXPECT_EQ((long) d, (long) value_to_long(v));

	value_destroy(v);
}

TEST_F(value_cast_double_test, double_to_float_cast)
{
	value v = value_create_double(78.31245);

	double d = value_to_double(v);

	v = value_type_cast(v, TYPE_FLOAT);

	EXPECT_EQ((float) d, (float) value_to_float(v));

	value_destroy(v);
}

TEST_F(value_cast_double_test, double_to_double_cast)
{
	value v = value_create_double(78.31245);

	double d = value_to_double(v);

	v = value_type_cast(v, TYPE_DOUBLE);

	EXPECT_EQ((double) d, (double) value_to_double(v));

	value_destroy(v);
}
