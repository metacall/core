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

class value_cast_char_test : public testing::Test
{
  public:
};

TEST_F(value_cast_char_test, char_to_bool_cast)
{
	value v = value_create_char('C');

	boolean b = 1L; /* Because C compiler cast doesn't convert 'C' to 1L */

	v = value_type_cast(v, TYPE_BOOL);

	EXPECT_EQ((boolean) b, (boolean) value_to_bool(v));

	value_destroy(v);
}

TEST_F(value_cast_char_test, char_to_char_cast)
{
	value v = value_create_char('C');

	char c = value_to_char(v);

	v = value_type_cast(v, TYPE_CHAR);

	EXPECT_EQ((char) c, (char) value_to_char(v));

	value_destroy(v);
}

TEST_F(value_cast_char_test, char_to_short_cast)
{
	value v = value_create_char('C');

	char c = value_to_char(v);

	v = value_type_cast(v, TYPE_SHORT);

	EXPECT_EQ((short) c, (short) value_to_short(v));

	value_destroy(v);
}

TEST_F(value_cast_char_test, char_to_int_cast)
{
	value v = value_create_char('C');

	char c = value_to_char(v);

	v = value_type_cast(v, TYPE_INT);

	EXPECT_EQ((int) c, (int) value_to_int(v));

	value_destroy(v);
}

TEST_F(value_cast_char_test, char_to_long_cast)
{
	value v = value_create_char('C');

	char c = value_to_char(v);

	v = value_type_cast(v, TYPE_LONG);

	EXPECT_EQ((long) c, (long) value_to_long(v));

	value_destroy(v);
}

TEST_F(value_cast_char_test, char_to_float_cast)
{
	value v = value_create_char('C');

	char c = value_to_char(v);

	v = value_type_cast(v, TYPE_FLOAT);

	EXPECT_EQ((float) c, (float) value_to_float(v));

	value_destroy(v);
}

TEST_F(value_cast_char_test, char_to_double_cast)
{
	value v = value_create_char('C');

	char c = value_to_char(v);

	v = value_type_cast(v, TYPE_DOUBLE);

	EXPECT_EQ((double) c, (double) value_to_double(v));

	value_destroy(v);
}

