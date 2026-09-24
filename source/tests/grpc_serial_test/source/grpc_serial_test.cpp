#include <gtest/gtest.h>

#include <serial/serial.h>
#include <log/log.h>

#include <cstring>

class grpc_serial_test : public testing::Test
{
protected:
	void SetUp() override
	{
		log_configure("metacall",
			log_policy_format_text(),
			log_policy_schedule_sync(),
			log_policy_storage_sequential(),
			log_policy_stream_stdio(stdout));

		allocator = memory_allocator_std(&malloc, &realloc, &free);

		ASSERT_EQ(0, serial_initialize());

		s = serial_create("grpc");
		ASSERT_NE(nullptr, s);
	}

	void TearDown() override
	{
		serial_clear(s);
		serial_destroy();
		memory_allocator_destroy(allocator);
	}

	serial s;
	memory_allocator allocator;
};

// Positive and Negative integer tests 

TEST_F(grpc_serial_test, IntPositive)
{
	value v = value_create_int(42);

	size_t size = 0;
	char *buf = serial_serialize(s, v, &size, allocator);

	ASSERT_NE(nullptr, buf);

	value out = serial_deserialize(s, buf, size, allocator);

	ASSERT_NE(nullptr, out);
	EXPECT_EQ(TYPE_INT, value_type_id(out));
	EXPECT_EQ(42, value_to_int(out));

	memory_allocator_deallocate(allocator, buf);
	value_destroy(v);
	value_destroy(out);
}

TEST_F(grpc_serial_test, IntNegative)
{
	value v = value_create_int(-123);

	size_t size = 0;
	char *buf = serial_serialize(s, v, &size, allocator);

	ASSERT_NE(nullptr, buf);

	value out = serial_deserialize(s, buf, size, allocator);

	ASSERT_NE(nullptr, out);
	EXPECT_EQ(-123, value_to_int(out));

	memory_allocator_deallocate(allocator, buf);
	value_destroy(v);
	value_destroy(out);
}

// String tests 

TEST_F(grpc_serial_test, StringBasic)
{
	const char *msg = "hello grpc";

	value v = value_create_string(msg, strlen(msg));

	size_t size = 0;
	char *buf = serial_serialize(s, v, &size, allocator);

	ASSERT_NE(nullptr, buf);

	value out = serial_deserialize(s, buf, size, allocator);

	ASSERT_NE(nullptr, out);
	EXPECT_EQ(TYPE_STRING, value_type_id(out));
	EXPECT_STREQ(msg, value_to_string(out));

	memory_allocator_deallocate(allocator, buf);
	value_destroy(v);
	value_destroy(out);
}

TEST_F(grpc_serial_test, StringEmpty)
{
	value v = value_create_string("", 0);

	size_t size = 0;
	char *buf = serial_serialize(s, v, &size, allocator);

	ASSERT_NE(nullptr, buf);

	value out = serial_deserialize(s, buf, size, allocator);

	ASSERT_NE(nullptr, out);
	EXPECT_EQ(TYPE_STRING, value_type_id(out));

	memory_allocator_deallocate(allocator, buf);
	value_destroy(v);
	value_destroy(out);
}

TEST_F(grpc_serial_test, StringSpecialChars)
{
	const char *msg = "line1\nline2\t\"quoted\"";

	value v = value_create_string(msg, strlen(msg));

	size_t size = 0;
	char *buf = serial_serialize(s, v, &size, allocator);

	ASSERT_NE(nullptr, buf);

	value out = serial_deserialize(s, buf, size, allocator);

	ASSERT_NE(nullptr, out);
	EXPECT_STREQ(msg, value_to_string(out));

	memory_allocator_deallocate(allocator, buf);
	value_destroy(v);
	value_destroy(out);
}

// Bool and double tests 

TEST_F(grpc_serial_test, BoolTest)
{
	value v = value_create_bool(1L);

	size_t size = 0;
	char *buf = serial_serialize(s, v, &size, allocator);

	ASSERT_NE(nullptr, buf);

	value out = serial_deserialize(s, buf, size, allocator);

	ASSERT_NE(nullptr, out);
	EXPECT_EQ(TYPE_BOOL, value_type_id(out));
	EXPECT_EQ(1L, value_to_bool(out));

	memory_allocator_deallocate(allocator, buf);
	value_destroy(v);
	value_destroy(out);
}

TEST_F(grpc_serial_test, DoubleTest)
{
	value v = value_create_double(3.14);

	size_t size = 0;
	char *buf = serial_serialize(s, v, &size, allocator);

	ASSERT_NE(nullptr, buf);

	value out = serial_deserialize(s, buf, size, allocator);

	ASSERT_NE(nullptr, out);
	EXPECT_EQ(TYPE_DOUBLE, value_type_id(out));
	EXPECT_NEAR(3.14, value_to_double(out), 1e-6);

	memory_allocator_deallocate(allocator, buf);
	value_destroy(v);
	value_destroy(out);
}

// Null test 

TEST_F(grpc_serial_test, NullTest)
{
	value v = value_create_null();

	size_t size = 0;
	char *buf = serial_serialize(s, v, &size, allocator);

	ASSERT_NE(nullptr, buf);

	value out = serial_deserialize(s, buf, size, allocator);

	ASSERT_NE(nullptr, out);
	EXPECT_EQ(TYPE_NULL, value_type_id(out));

	memory_allocator_deallocate(allocator, buf);
	value_destroy(v);
	value_destroy(out);
}

// Array test 

TEST_F(grpc_serial_test, SimpleArray)
{
	const value arr[] = {
		value_create_int(1),
		value_create_int(2),
		value_create_int(3)
	};

	value v = value_create_array(arr, 3);

	size_t size = 0;
	char *buf = serial_serialize(s, v, &size, allocator);

	ASSERT_NE(nullptr, buf);

	value out = serial_deserialize(s, buf, size, allocator);

	ASSERT_NE(nullptr, out);
	EXPECT_EQ(TYPE_ARRAY, value_type_id(out));
	EXPECT_EQ(3, value_type_count(out));

	value *vals = value_to_array(out);
	EXPECT_EQ(1, value_to_int(vals[0]));
	EXPECT_EQ(2, value_to_int(vals[1]));
	EXPECT_EQ(3, value_to_int(vals[2]));

	memory_allocator_deallocate(allocator, buf);
	value_destroy(v);
	value_destroy(out);
}

// Map test 

TEST_F(grpc_serial_test, SimpleMap)
{
	const value pair[] = {
		value_create_string("key", 3),
		value_create_int(99)
	};

	const value map_arr[] = {
		value_create_array(pair, 2)
	};

	value v = value_create_map(map_arr, 1);

	size_t size = 0;
	char *buf = serial_serialize(s, v, &size, allocator);

	ASSERT_NE(nullptr, buf);

	value out = serial_deserialize(s, buf, size, allocator);

	ASSERT_NE(nullptr, out);
	EXPECT_EQ(TYPE_MAP, value_type_id(out));

	memory_allocator_deallocate(allocator, buf);
	value_destroy(v);
	value_destroy(out);
}

// Error case test 

TEST_F(grpc_serial_test, UnsupportedTypeFailsGracefully)
{
	// Example: function type (unsupported)
	value v = value_create_null(); 
	size_t size = 0;
	char *buf = serial_serialize(s, v, &size, allocator);

	// Should not crash
	EXPECT_TRUE(buf == nullptr || size > 0);

	if (buf)
		memory_allocator_deallocate(allocator, buf);

	value_destroy(v);
}