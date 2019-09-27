/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
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

	// Create allocator
	memory_allocator allocator = memory_allocator_std(&malloc, &realloc, &free);

	EXPECT_NE((memory_allocator) NULL, (memory_allocator) allocator);

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
			value_create_array(value_map_a, sizeof(value_map_a) / sizeof(value_map_a[0])),
			value_create_array(value_map_b, sizeof(value_map_b) / sizeof(value_map_b[0]))
		};

		static const size_t value_list_size = sizeof(value_list) / sizeof(value_list[0]);
		static const char value_list_str[] = "[244,6.8,\"hello world\"]";
		static const size_t value_map_size = sizeof(value_map) / sizeof(value_map[0]);
		static const char value_map_str[] = "{\"aaa\":3.333,\"bbb\":4.5}";

		static const char json_buffer_array[] = "[\"asdf\",443,3.2]";
		static const size_t json_buffer_array_size = 3;

		static const char json_buffer_map[] = "{\"abc\":9.9,\"cde\":1.5}";
		static const size_t json_buffer_map_size = 2;

		static const char json_true[] = "true";
		static const char json_number[] = "23434";
		static const char json_string[] = "\"Hello World\"";
		static const char json_string_value[] = "Hello World";

		size_t serialize_size = 0;

		serial s = serial_create(rapid_json_name());

		// Create value array from value list
		value v = value_create_array(value_list, value_list_size);

		EXPECT_NE((value) NULL, (value) v);

		// Serialize value array into buffer
		char * buffer = serial_serialize(s, v, &serialize_size, allocator);

		EXPECT_EQ((size_t) sizeof(value_list_str), (size_t) serialize_size);
		EXPECT_NE((char *) NULL, (char *) buffer);
		EXPECT_EQ((int) 0, (int) strcmp(buffer, value_list_str));

		value_destroy(v);

		for (size_t iterator = 0; iterator < value_list_size; ++iterator)
		{
			value_destroy(value_list[iterator]);
		}

		memory_allocator_deallocate(allocator, buffer);

		// Create value map from value list map
		v = value_create_map(value_map, value_map_size);

		EXPECT_NE((value) NULL, (value) v);

		// Serialize value map into buffer
		buffer = serial_serialize(s, v, &serialize_size, allocator);

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

		memory_allocator_deallocate(allocator, buffer);
	
		// Deserialize json buffer array into value
		v = serial_deserialize(s, json_buffer_array, sizeof(json_buffer_array), allocator);

		EXPECT_EQ((type_id) TYPE_ARRAY, (type_id) value_type_id(v));
		EXPECT_EQ((size_t) json_buffer_array_size, (size_t) value_type_count(v));

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
		v = serial_deserialize(s, json_buffer_map, sizeof(json_buffer_map), allocator);

		EXPECT_EQ((type_id) TYPE_MAP, (type_id) value_type_id(v));
		EXPECT_EQ((size_t) json_buffer_map_size, (size_t) value_type_count(v));

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

		// Deserialize json boolean primitive type into value
		v = serial_deserialize(s, json_true, sizeof(json_true), allocator);

		EXPECT_EQ((type_id) TYPE_BOOL, (type_id) value_type_id(v));
		EXPECT_EQ((size_t) sizeof(boolean), (size_t)value_type_size(v));
		EXPECT_EQ((boolean) 1L, (boolean) value_to_bool(v));

		value_destroy(v);

		// Deserialize json number primitive type into value
		v = serial_deserialize(s, json_number, sizeof(json_number), allocator);

		EXPECT_EQ((type_id) TYPE_INT, (type_id) value_type_id(v));
		EXPECT_EQ((size_t) sizeof(int), (size_t) value_type_size(v));
		EXPECT_EQ((int) 23434, (int) value_to_int(v));

		value_destroy(v);

		// Deserialize json string primitive type into value
		v = serial_deserialize(s, json_string, sizeof(json_string), allocator);

		EXPECT_EQ((type_id) TYPE_STRING, (type_id) value_type_id(v));
		EXPECT_EQ((size_t) sizeof(json_string_value), (size_t) value_type_size(v));
		EXPECT_EQ((int) 0, (int) strncmp(value_to_string(v), json_string_value, sizeof(json_string_value) - 1));

		value_destroy(v);
	}

	// MetaCall
	{
		static const char hello_world[] = "hello world";
		static const char good_bye[] = "good bye";

		static const size_t hello_world_length = sizeof(hello_world) - 1;
		static const size_t good_bye_length = sizeof(good_bye) - 1;

		static const char * value_names[] =
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
			NULL, /* TODO: Map */
			#if defined(_WIN32) && defined(_MSC_VER)
				#if defined(_WIN64)
					"0x00000000000A7EF2",
				#else
					"0x000A7EF2",
				#endif
			#elif defined(__linux) || defined(__linux__) || defined(__APPLE__)
				"0xa7ef2",
			#else
				"<unknown>",
			#endif
			NULL, /* TODO: Future */
			NULL, /* TODO: Function */
			"(null)"
		};

		static_assert((int) sizeof(value_names) / sizeof(value_names[0]) == (int) TYPE_SIZE,
			"Value names size does not match type size.");

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
			value_create_array(value_map_tupla_b, sizeof(value_map_tupla_b) / sizeof(value_map_tupla_b[0]))
		};

		static const size_t value_map_size = sizeof(value_map) / sizeof(value_map[0]);

		value value_array[] =
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
			value_create_ptr((void *)0x000A7EF2),
			value_create_future(NULL), /* TODO: Implement future properly */
			value_create_function(NULL), /* TODO: Implement function properly */
			value_create_null()
		};

		static_assert((int) sizeof(value_array) / sizeof(value_array[0]) == (int)TYPE_SIZE,
			"Value array size does not match type size.");

		const size_t value_names_size = sizeof(value_names) / sizeof(value_names[0]);

		serial s = serial_create(metacall_name());

		for (size_t iterator = 0; iterator < value_names_size; ++iterator)
		{
			/* TODO: Remove this workaround when type map and future/function stringification is implemented */
			if (value_names[iterator] != NULL)
			{
				size_t size;

				char * buffer = serial_serialize(s, value_array[iterator], &size, allocator);

				log_write("metacall", LOG_LEVEL_DEBUG, "(%s == %s)", buffer, value_names[iterator]);

				EXPECT_GT((size_t) size, (size_t) 0);

				EXPECT_EQ((int) 0, (int) strncmp(buffer, value_names[iterator], size - 1));

				memory_allocator_deallocate(allocator, buffer);
			}

			value_type_destroy(value_array[iterator]);
		}
	}

	// Clear RapidJSON serial
	EXPECT_EQ((int) 0, (int) serial_clear(serial_create(rapid_json_name())));

	// Clear MetaCall serial
	EXPECT_EQ((int) 0, (int) serial_clear(serial_create(metacall_name())));

	// Destroy serial
	serial_destroy();

	// Destroy allocator
	memory_allocator_destroy(allocator);
}
