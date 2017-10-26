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

	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	// Initialize serial
	EXPECT_EQ((int) 0, (int) serial_initialize());

	// Create serial
	serial s = serial_create(rapid_json_name);

	ASSERT_NE((serial) NULL, (serial) s);

	EXPECT_EQ((int) 0, (int) strcmp(rapid_json_name, serial_name(s)));

	EXPECT_EQ((int) 0, (int) strcmp(rapid_json_extension, serial_extension(s)));

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

	// Clear serial
	EXPECT_EQ((int) 0, (int) serial_clear(s));

	// Destroy serial
	serial_destroy();
}
