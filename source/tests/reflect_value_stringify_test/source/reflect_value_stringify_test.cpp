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
	static const char good_bye[] = "good bye";

	static const size_t hello_world_length = sizeof(hello_world) - 1;
	static const size_t good_bye_length = sizeof(good_bye) - 1;

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
		"[244,6.800000,hello world]",
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
		value_create_string(hello_world, hello_world_length)
	};

	static const size_t value_list_size = sizeof(value_list) / sizeof(value_list[0]);

	static const value value_map_tupla_a[] =
	{
		value_create_string(hello_world, hello_world_length),
		value_create_int(9874),
	};

	static const value value_map_tupla_b[] =
	{
		value_create_string(good_bye, good_bye_length),
		value_create_int(1111),
	};

	static const value value_map[] =
	{
		value_create_array(value_map_tupla_a, sizeof(value_map_tupla_a) / sizeof(value_map_tupla_a[0])),
		value_create_array(value_map_tupla_b, sizeof(value_map_tupla_b) / sizeof(value_map_tupla_b[0])),
	};

	static const size_t value_map_size = sizeof(value_map) / sizeof(value_map[0]);

	value value_array[TYPE_SIZE] =
	{
		value_create_bool(1),
		value_create_char('A'),
		value_create_short(123),
		value_create_int(56464),
		value_create_long(251251251L),
		value_create_float(13.545f),
		value_create_double(545.3453),
		value_create_string(hello_world, hello_world_length),
		value_create_buffer(char_array, sizeof(char_array)),
		value_create_array(value_list, value_list_size),
		value_create_map(value_map, value_map_size),
		value_create_ptr((void *)0x000A7EF2)
	};

	const size_t size = sizeof(value_names) / sizeof(value_names[0]);

	for (size_t iterator = 0; iterator < size; ++iterator)
	{
		#define BUFFER_SIZE 100

		char buffer[BUFFER_SIZE];

		const size_t buffer_size = BUFFER_SIZE;

		size_t length = 0;

		/* TODO: Remove this workaround when type map stringification is implemented */
		if (value_type_id(value_array[iterator]) != TYPE_MAP)
		{
			value_stringify(value_array[iterator], buffer, buffer_size, &length);

			log_write("metacall", LOG_LEVEL_DEBUG, "(%s == %s)", buffer, value_names[iterator]);

			EXPECT_EQ((int)0, (int)strncmp(buffer, value_names[iterator], length));

			EXPECT_LT((size_t)length, (size_t)buffer_size);

			value_destroy(value_array[iterator]);
		}

		#undef BUFFER_SIZE
	}

	for (size_t index = 0; index < value_list_size; ++index)
	{
		value_destroy(value_list[index]);
	}

	for (size_t index = 0; index < value_map_size; ++index)
	{
		value iterator = value_map[index];

		value * tupla = value_to_array(iterator);

		value_destroy(tupla[0]);
		value_destroy(tupla[1]);

		value_destroy(iterator);
	}
}
