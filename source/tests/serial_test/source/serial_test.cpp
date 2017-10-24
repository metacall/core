/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <gmock/gmock.h>

#include <serial/serial.h>

#include <log/log.h>

class serial_test : public testing::Test
{
  public:
};

TEST_F(serial_test, DefaultConstructor)
{
	static const char rapid_json_name[] = "rapid_json";
	static const char rapid_json_extension[] = "json";

	static const char hello_world[] = "hello world";

	static const value value_list[] =
	{
		value_create_int(244),
		value_create_double(6.8),
		value_create_string(hello_world, sizeof(hello_world))
	};

	size_t serialize_size = 0;

	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	EXPECT_EQ((int) 0, (int) serial_initialize());

	serial s = serial_create(rapid_json_name);

	EXPECT_NE((serial) NULL, (serial) s);

	EXPECT_EQ((int) 0, (int) strcmp(rapid_json_name, serial_name(s)));

	EXPECT_EQ((int) 0, (int) strcmp(rapid_json_extension, serial_extension(s)));

	value v = value_create_array(value_list, sizeof(value_list) / sizeof(value_list[0]));

	EXPECT_NE((value) NULL, (value) v);

	char * buffer = serial_serialize(s, v, &serialize_size);

	EXPECT_NE((char *) NULL, (char *) buffer);

	log_write("metacall", LOG_LEVEL_DEBUG, "Serialized buffer: %s", buffer);

	free(buffer);
	
	EXPECT_EQ((int) 0, (int) serial_clear(s));

	serial_destroy();

	const size_t value_list_size = sizeof(value_list) / sizeof(value_list[0]);

	for (size_t iterator = 0; iterator < value_list_size; ++iterator)
	{
		value_destroy(value_list[iterator]);
	}
}
