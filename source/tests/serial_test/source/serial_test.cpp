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
	void create_serial(const char * name, const char * extension)
	{
		serial s = serial_create(name);

		ASSERT_NE((serial) NULL, (serial) s);

		EXPECT_EQ((int) 0, (int) strcmp(name, serial_name(s)));

		EXPECT_EQ((int) 0, (int) strcmp(extension, serial_extension(s)));
	}

	const char * rapid_json_name()
	{
		return "rapid_json";
	}
	const char * rapid_json_extension()
	{
		return "json";
	}
	const char * metacall_name()
	{
		return "metacall";
	}
	const char * metacall_extension()
	{
		return "meta";
	}
};

TEST_F(serial_test, DefaultConstructor)
{
	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	// Initialize serial
	EXPECT_EQ((int) 0, (int) serial_initialize());

	// Create RapidJSON serial
	create_serial(rapid_json_name(), rapid_json_extension());

	// Create MetaCall serial
	create_serial(metacall_name(), metacall_extension());

	// RapidJSON
	{
		static const char hello_world[] = "hello world";

		static const value value_list[] =
		{
			value_create_int(244),
			value_create_double(6.8),
			value_create_string(hello_world, sizeof(hello_world) - 1)
		};

		static const char map_index_a[] = "aaa";
		static const char map_index_b[] = "bbb";

		static const value value_map_a[] =
		{
			value_create_string(map_index_a, sizeof(map_index_a) - 1),
			value_create_double(3.333)
		};

		static const value value_map_b[] =
		{
			value_create_string(map_index_b, sizeof(map_index_b) - 1),
			value_create_double(4.5)
		};

		static const value value_map[] =
		{
			value_create_array(value_map_a, sizeof(value_map_a)),
			value_create_array(value_map_b, sizeof(value_map_b))
		};

		static const size_t value_list_size = sizeof(value_list) / sizeof(value_list[0]);
		static const char value_list_str[] = "[244,6.8,\"hello world\"]";
		static const size_t value_map_size = sizeof(value_map) / sizeof(value_map[0]);
		static const char value_map_str[] = "{\"aaa\":3.333,\"bbb\":4.5}";

		static const char json_buffer_array[] = "[\"asdf\",443,3.2]";
		static const size_t json_buffer_array_size = 3;

		static const char json_buffer_map[] = "{\"abc\":9.9,\"cde\":1.5}";
		static const size_t json_buffer_map_size = 2;

		size_t serialize_size = 0;

		serial s = serial_create(rapid_json_name());

		// Create value array from value list
		value v = value_create_array(value_list, value_list_size);

		EXPECT_NE((value) NULL, (value) v);

		// Serialize value array into buffer
		char * buffer = serial_serialize(s, v, &serialize_size);

		EXPECT_EQ((size_t) sizeof(value_list_str), (size_t) serialize_size);
		EXPECT_NE((char *) NULL, (char *) buffer);
		EXPECT_EQ((int) 0, (int) strcmp(buffer, value_list_str));

		value_destroy(v);

		for (size_t iterator = 0; iterator < value_list_size; ++iterator)
		{
			value_destroy(value_list[iterator]);
		}

		free(buffer);

		// Create value map from value list map
		v = value_create_map(value_map, value_map_size);

		EXPECT_NE((value) NULL, (value) v);

		// Serialize value map into buffer
		buffer = serial_serialize(s, v, &serialize_size);

		EXPECT_EQ((size_t) sizeof(value_map_str), (size_t) serialize_size);
		EXPECT_NE((value) NULL, (value) v);
		EXPECT_EQ((int) 0, (int) strcmp(buffer, value_map_str));

		value * v_map = value_to_map(v);

		for (size_t iterator = 0; iterator < value_map_size; ++iterator)
		{
			value * tupla = value_to_array(v_map[iterator]);

			value_destroy(tupla[0]);
			value_destroy(tupla[1]);

			value_destroy(v_map[iterator]);
		}

		value_destroy(v);

		free(buffer);
	
		// Deserialize json buffer array into value
		v = serial_deserialize(s, json_buffer_array, sizeof(json_buffer_array));

		EXPECT_EQ((type_id) TYPE_ARRAY, (type_id) value_type_id(v));
		EXPECT_EQ((size_t) json_buffer_array_size, (size_t) value_type_size(v) / sizeof(const value));

		value * v_array = value_to_array(v);

		EXPECT_NE((value *) NULL, (value *) v_array);

		EXPECT_EQ((type_id) TYPE_STRING, (type_id) value_type_id(v_array[0]));
		EXPECT_EQ((int) 0, (int) strcmp(value_to_string(v_array[0]), "asdf"));

		EXPECT_EQ((type_id) TYPE_INT, (type_id) value_type_id(v_array[1]));
		EXPECT_EQ((int) 443, (int) value_to_int(v_array[1]));

		EXPECT_EQ((type_id) TYPE_FLOAT, (type_id) value_type_id(v_array[2]));
		EXPECT_EQ((float) 3.2f, (float) value_to_float(v_array[2]));

		for (size_t iterator = 0; iterator < value_list_size; ++iterator)
		{
			value_destroy(v_array[iterator]);
		}

		value_destroy(v);

		// Deserialize json buffer map into value
		v = serial_deserialize(s, json_buffer_map, sizeof(json_buffer_map));

		EXPECT_EQ((type_id) TYPE_MAP, (type_id) value_type_id(v));
		EXPECT_EQ((size_t) json_buffer_map_size, (size_t) value_type_size(v) / sizeof(const value));

		v_map = value_to_map(v);

		EXPECT_NE((value *) NULL, (value *) v_map);

		EXPECT_EQ((type_id) TYPE_ARRAY, (type_id) value_type_id(v_map[0]));
		EXPECT_EQ((type_id) TYPE_ARRAY, (type_id) value_type_id(v_map[1]));

		value * tupla = value_to_array(v_map[0]);

		EXPECT_EQ((type_id) TYPE_STRING, (type_id) value_type_id(tupla[0]));
		EXPECT_EQ((int) 0, (int) strcmp(value_to_string(tupla[0]), "abc"));

		EXPECT_EQ((type_id) TYPE_FLOAT, (type_id) value_type_id(tupla[1]));
		EXPECT_EQ((float) 9.9f, (float) value_to_float(tupla[1]));

		value_destroy(tupla[0]);
		value_destroy(tupla[1]);

		value_destroy(v_map[0]);

		tupla = value_to_array(v_map[1]);

		EXPECT_EQ((type_id) TYPE_STRING, (type_id) value_type_id(tupla[0]));
		EXPECT_EQ((int) 0, (int) strcmp(value_to_string(tupla[0]), "cde"));

		EXPECT_EQ((type_id) TYPE_FLOAT, (type_id) value_type_id(tupla[1]));
		EXPECT_EQ((float) 1.5f, (float) value_to_float(tupla[1]));

		value_destroy(tupla[0]);
		value_destroy(tupla[1]);

		value_destroy(v_map[1]);

		value_destroy(v);
	}

	// MetaCall
	{
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
			NULL, /* TODO */
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

		const size_t value_names_size = sizeof(value_names) / sizeof(value_names[0]);

		serial s = serial_create(metacall_name());

		for (size_t iterator = 0; iterator < value_names_size; ++iterator)
		{
			/* TODO: Remove this workaround when type map stringification is implemented */
			if (value_type_id(value_array[iterator]) != TYPE_MAP)
			{
				size_t size;

				char * buffer = serial_serialize(s, value_array[iterator], &size);

				log_write("metacall", LOG_LEVEL_DEBUG, "(%s == %s)", buffer, value_names[iterator]);

				EXPECT_GT((size_t) size, (size_t) 0);

				EXPECT_EQ((int) 0, (int) strncmp(buffer, value_names[iterator], size - 1));

				value_destroy(value_array[iterator]);
			}
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

	// Clear RapidJSON serial
	EXPECT_EQ((int) 0, (int) serial_clear(serial_create(rapid_json_name())));

	// Clear MetaCall serial
	EXPECT_EQ((int) 0, (int) serial_clear(serial_create(metacall_name())));

	// Destroy serial
	serial_destroy();
}
