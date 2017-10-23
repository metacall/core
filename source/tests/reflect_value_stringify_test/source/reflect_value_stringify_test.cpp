/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <gmock/gmock.h>

#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_stringify.h>

#include <log/log.h>

class reflect_value_stringify_test : public testing::Test
{
  public:
};

TEST_F(reflect_value_stringify_test, DefaultConstructor)
{
	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	static const char hello_world[] = "hello world";

	static const char * value_names[TYPE_SIZE] =
	{
		"true",
		"A",
		"123",
		"56464",
		"251251251",
		"13.545000f",
		"545.345300",
		hello_world,
		"05060708",
		"[244,6.800000,\"hello world\"]",
		#if defined(_WIN32) && defined(_MSC_VER)
			#if defined(_WIN64)
				"0x00000000000A7EF2"
			#else
				"0x000A7EF2"
			#endif
		#elif defined(__linux) || defined(__linux__)
			"0xa7ef2"
		#else
			"<unknown>"
		#endif
	};

	static const char char_array[] =
	{
		0x05, 0x06, 0x07, 0x08
	};

	static const value value_list[] =
	{
		value_create_int(244),
		value_create_double(6.8),
		value_create_string(hello_world, sizeof(hello_world))
	};

	value value_array[TYPE_SIZE] =
	{
		value_create_bool(1),
		value_create_char('A'),
		value_create_short(123),
		value_create_int(56464),
		value_create_long(251251251L),
		value_create_float(13.545f),
		value_create_double(545.3453),
		value_create_string(hello_world, sizeof(hello_world)),
		value_create_buffer(char_array, sizeof(char_array)),
		value_create_array(value_list, sizeof(value_list) / sizeof(value_list[0])),
		value_create_ptr((void *)0x000A7EF2)
	};

	const size_t size = sizeof(value_names) / sizeof(value_names[0]);

	for (size_t iterator = 0; iterator < size; ++iterator)
	{
		#define BUFFER_SIZE 100

		char buffer[BUFFER_SIZE];

		const size_t buffer_size = BUFFER_SIZE;

		size_t length;

		/* TODO: Remove this workaround when implementing array stringify */
		if (value_type_id(value_array[iterator]) != TYPE_ARRAY)
		{
			value_stringify(value_array[iterator], buffer, buffer_size, &length);

			log_write("metacall", LOG_LEVEL_DEBUG, "(%s == %s)", buffer, value_names[iterator]);

			EXPECT_EQ((int) 0, (int) strncmp(buffer, value_names[iterator], length));

			EXPECT_LT((size_t) length, (size_t)buffer_size);
		}
		else
		{
			log_write("metacall", LOG_LEVEL_WARNING, "WARNING: Avoiding test for TYPE_ARRAY");
		}

		value_destroy(value_array[iterator]);

		#undef BUFFER_SIZE
	}

	const size_t value_list_size = sizeof(value_list) / sizeof(value_list[0]);

	for (size_t iterator = 0; iterator < value_list_size; ++iterator)
	{
		value_destroy(value_list[iterator]);
	}
}
