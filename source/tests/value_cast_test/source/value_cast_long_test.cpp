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

class value_cast_long_test : public testing::Test
{
  public:
};

TEST_F(value_cast_long_test, long_to_bool_cast)
{
	value v = value_create_long(931324L);

	boolean b = 1L; /* Because C compiler cast doesn't convert 931324L to 1L */

	v = value_type_cast(v, TYPE_BOOL);

	EXPECT_EQ((boolean) b, (boolean) value_to_bool(v));

	value_destroy(v);
}

TEST_F(value_cast_long_test, long_to_char_cast)
{
	value v = value_create_long(931324L);

	long l = value_to_long(v);

	v = value_type_cast(v, TYPE_CHAR);

	EXPECT_EQ((char) l, (char) value_to_char(v));

	value_destroy(v);
}

TEST_F(value_cast_long_test, long_to_short_cast)
{
	value v = value_create_long(931324L);

	long l = value_to_long(v);

	v = value_type_cast(v, TYPE_SHORT);

	EXPECT_EQ((short) l, (short) value_to_short(v));

	value_destroy(v);
}

TEST_F(value_cast_long_test, long_to_int_cast)
{
	value v = value_create_long(931324L);

	long l = value_to_long(v);

	v = value_type_cast(v, TYPE_INT);

	EXPECT_EQ((int) l, (int) value_to_int(v));

	value_destroy(v);
}

TEST_F(value_cast_long_test, long_to_long_cast)
{
	value v = value_create_long(931324L);

	long l = value_to_long(v);

	v = value_type_cast(v, TYPE_LONG);

	EXPECT_EQ((long) l, (long) value_to_long(v));

	value_destroy(v);
}

TEST_F(value_cast_long_test, long_to_float_cast)
{
	value v = value_create_long(931324L);

	long l = value_to_long(v);

	v = value_type_cast(v, TYPE_FLOAT);

	EXPECT_EQ((float) l, (float) value_to_float(v));

	value_destroy(v);
}

TEST_F(value_cast_long_test, long_to_double_cast)
{
	value v = value_create_long(931324L);

	long l = value_to_long(v);

	v = value_type_cast(v, TYPE_DOUBLE);

	EXPECT_EQ((double) l, (double) value_to_double(v));

	value_destroy(v);
}
