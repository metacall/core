/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <gmock/gmock.h>

#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_cast.h>

class value_cast_short_test : public testing::Test
{
  public:
};

TEST_F(value_cast_short_test, short_to_bool_cast)
{
	value v = value_create_short(1923);

	boolean b = 1L; /* Because C compiler cast doesn't convert 1923 to 1L */

	v = value_type_cast(v, TYPE_BOOL);

	EXPECT_EQ((boolean) b, (boolean) value_to_bool(v));

	value_destroy(v);
}

TEST_F(value_cast_short_test, short_to_char_cast)
{
	value v = value_create_short(1923);

	short s = value_to_short(v);

	v = value_type_cast(v, TYPE_CHAR);

	EXPECT_EQ((char) s, (char) value_to_char(v));

	value_destroy(v);
}

TEST_F(value_cast_short_test, short_to_short_cast)
{
	value v = value_create_short(1923);

	short s = value_to_short(v);

	v = value_type_cast(v, TYPE_SHORT);

	EXPECT_EQ((short) s, (short) value_to_short(v));

	value_destroy(v);
}

TEST_F(value_cast_short_test, short_to_int_cast)
{
	value v = value_create_short(1923);

	short s = value_to_short(v);

	v = value_type_cast(v, TYPE_INT);

	EXPECT_EQ((int) s, (int) value_to_int(v));

	value_destroy(v);
}

TEST_F(value_cast_short_test, short_to_long_cast)
{
	value v = value_create_short(1923);

	short s = value_to_short(v);

	v = value_type_cast(v, TYPE_LONG);

	EXPECT_EQ((long) s, (long) value_to_long(v));

	value_destroy(v);
}

TEST_F(value_cast_short_test, short_to_float_cast)
{
	value v = value_create_short(1923);

	short s = value_to_short(v);

	v = value_type_cast(v, TYPE_FLOAT);

	EXPECT_EQ((float) s, (float) value_to_float(v));

	value_destroy(v);
}

TEST_F(value_cast_short_test, short_to_double_cast)
{
	value v = value_create_short(1923);

	short s = value_to_short(v);

	v = value_type_cast(v, TYPE_DOUBLE);

	EXPECT_EQ((double) s, (double) value_to_double(v));

	value_destroy(v);
}
