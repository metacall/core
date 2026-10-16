/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

/* SimdJSON must be included before MetaCall headers because reflect defines boolean as a macro */
#include <simdjson.h>

#include <simd_json_serial/simd_json_serial_impl.h>

#include <metacall/metacall_error.h>

#include <log/log.h>

#include <climits>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

/* -- Definitions -- */

/* Minimum page size of the supported platforms, used for checking if the padding can be read safely */
#define SIMD_JSON_SERIAL_PAGE_SIZE 4096

/* -- Type Definitions -- */

typedef struct simd_json_document_type
{
	simdjson::dom::parser parser;
	simdjson::builder::string_builder builder;
	memory_allocator allocator;

} * simd_json_document;

/* -- Private Methods -- */

static void simd_json_serial_impl_serialize_value(value v, simdjson::builder::string_builder &builder);

static char *simd_json_serial_impl_document_stringify(simd_json_document document, size_t *size);

static value simd_json_serial_impl_deserialize_value(simdjson::dom::element v);

/* -- Methods -- */

const char *simd_json_serial_impl_extension()
{
	static const char extension[] = "json";

	return extension;
}

serial_handle simd_json_serial_impl_initialize(memory_allocator allocator)
{
	simd_json_document document = new simd_json_document_type();

	if (document == nullptr)
	{
		return NULL;
	}

	document->allocator = allocator;

	/* Big integers (exceeding uint64) are stored as strings and converted into double, like RapidJSON does */
	document->parser.number_as_string(true);

	return (serial_handle)document;
}

void simd_json_serial_impl_serialize_value(value v, simdjson::builder::string_builder &builder)
{
	type_id id = value_type_id(v);

	if (id == TYPE_BOOL)
	{
		boolean b = value_to_bool(v);

		builder.append(b == 1L ? true : false);
	}
	else if (id == TYPE_CHAR)
	{
		char c = value_to_char(v);

		builder.escape_and_append_with_quotes(std::string_view(&c, 1));
	}
	else if (id == TYPE_SHORT)
	{
		short s = value_to_short(v);

		builder.append((int)s);
	}
	else if (id == TYPE_INT)
	{
		int i = value_to_int(v);

		builder.append(i);
	}
	else if (id == TYPE_LONG)
	{
		long l = value_to_long(v);

		builder.append((int64_t)l);
	}
	else if (id == TYPE_FLOAT)
	{
		float f = value_to_float(v);

		builder.append((double)f);
	}
	else if (id == TYPE_DOUBLE)
	{
		double d = value_to_double(v);

		builder.append(d);
	}
	else if (id == TYPE_STRING)
	{
		const char *str = value_to_string(v);

		size_t size = value_type_size(v);

		size_t length = size > 0 ? size - 1 : 0;

		builder.escape_and_append_with_quotes(std::string_view(str, length));
	}
	else if (id == TYPE_BUFFER)
	{
		static const char data_str[] = "\"data\":[";
		static const char length_str[] = "],\"length\":";

		void *buffer = value_to_buffer(v);

		size_t size = value_type_size(v);

		builder.start_object();
		builder.append_raw(data_str, sizeof(data_str) - 1);

		for (size_t iterator = 0; iterator < size; ++iterator)
		{
			const unsigned char *data = (const unsigned char *)(((uintptr_t)buffer) + iterator);

			if (iterator > 0)
			{
				builder.append_comma();
			}

			builder.append((unsigned int)*data);
		}

		builder.append_raw(length_str, sizeof(length_str) - 1);
		builder.append((uint64_t)size);
		builder.end_object();
	}
	else if (id == TYPE_ARRAY)
	{
		value *value_array = value_to_array(v);

		size_t array_size = value_type_count(v);

		builder.start_array();

		for (size_t iterator = 0; iterator < array_size; ++iterator)
		{
			if (iterator > 0)
			{
				builder.append_comma();
			}

			simd_json_serial_impl_serialize_value(value_array[iterator], builder);
		}

		builder.end_array();
	}
	else if (id == TYPE_MAP)
	{
		value *value_map = value_to_map(v);

		size_t map_size = value_type_count(v);

		builder.start_object();

		for (size_t iterator = 0; iterator < map_size; ++iterator)
		{
			value tupla = value_map[iterator];

			value *tupla_array = value_to_array(tupla);

			if (iterator > 0)
			{
				builder.append_comma();
			}

			simd_json_serial_impl_serialize_value(tupla_array[0], builder);

			builder.append_colon();

			simd_json_serial_impl_serialize_value(tupla_array[1], builder);
		}

		builder.end_object();
	}
	else if (id == TYPE_FUTURE)
	{
		/* TODO: Improve future serialization */
		static const char str[] = "\"[Future]\"";

		builder.append_raw(str, sizeof(str) - 1);
	}
	else if (id == TYPE_FUNCTION)
	{
		/* TODO: Improve function serialization */
		static const char str[] = "\"[Function]\"";

		builder.append_raw(str, sizeof(str) - 1);
	}
	else if (id == TYPE_CLASS)
	{
		/* TODO: Improve class serialization */
		static const char str[] = "\"[Class]\"";

		builder.append_raw(str, sizeof(str) - 1);
	}
	else if (id == TYPE_OBJECT)
	{
		/* TODO: Improve object serialization */
		static const char str[] = "\"[Object]\"";

		builder.append_raw(str, sizeof(str) - 1);
	}
	else if (id == TYPE_EXCEPTION)
	{
		static const char message_str[] = "\"message\":";
		static const char label_str[] = ",\"label\":";
		static const char code_str[] = ",\"code\":";
		static const char stacktrace_str[] = ",\"stacktrace\":";

		exception ex = value_to_exception(v);

		builder.start_object();

		builder.append_raw(message_str, sizeof(message_str) - 1);
		builder.escape_and_append_with_quotes(std::string_view(exception_message(ex)));

		builder.append_raw(label_str, sizeof(label_str) - 1);
		builder.escape_and_append_with_quotes(std::string_view(exception_label(ex)));

		builder.append_raw(code_str, sizeof(code_str) - 1);
		builder.append((int64_t)exception_error_code(ex));

		builder.append_raw(stacktrace_str, sizeof(stacktrace_str) - 1);
		builder.escape_and_append_with_quotes(std::string_view(exception_stacktrace(ex)));

		builder.end_object();
	}
	else if (id == TYPE_THROWABLE)
	{
		static const char str[] = "\"ExceptionThrown\":";

		throwable th = value_to_throwable(v);

		builder.start_object();

		builder.append_raw(str, sizeof(str) - 1);

		simd_json_serial_impl_serialize_value(throwable_value(th), builder);

		builder.end_object();
	}
	else if (id == TYPE_PTR)
	{
		std::ostringstream ostream;

		ostream << value_to_ptr(v);

		std::string s = ostream.str();

		builder.escape_and_append_with_quotes(std::string_view(s));
	}
	else if (id == TYPE_NULL)
	{
		builder.append_null();
	}
}

char *simd_json_serial_impl_document_stringify(simd_json_document document, size_t *size)
{
	std::string_view view;

	if (document->builder.view().get(view) != simdjson::SUCCESS)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid string builder state for document stringifycation in SimdJSON implementation");
		return NULL;
	}

	size_t buffer_size = view.size();
	size_t buffer_str_size = buffer_size + 1;
	char *buffer_str = static_cast<char *>(memory_allocator_allocate(document->allocator, sizeof(char) * buffer_str_size));

	if (buffer_str == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid string allocation for document stringifycation in SimdJSON implementation");
		return NULL;
	}

	memcpy(buffer_str, view.data(), buffer_size);

	buffer_str[buffer_size] = '\0';

	*size = buffer_str_size;

	return buffer_str;
}

char *simd_json_serial_impl_serialize(serial_handle handle, value v, size_t *size)
{
	simd_json_document document = static_cast<simd_json_document>(handle);

	if (handle == NULL || v == NULL || size == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Serialization called with wrong arguments in SimdJSON implementation");

		return NULL;
	}

	/* Reuse the builder buffer between serializations */
	document->builder.clear();

	simd_json_serial_impl_serialize_value(v, document->builder);

	return simd_json_serial_impl_document_stringify(document, size);
}

value simd_json_serial_impl_deserialize_value(simdjson::dom::element v)
{
	switch (v.type())
	{
		case simdjson::dom::element_type::NULL_VALUE: {
			return value_create_null();
		}

		case simdjson::dom::element_type::BOOL: {
			return value_create_bool(v.get_bool().value_unsafe() == true ? 1L : 0L);
		}

		case simdjson::dom::element_type::INT64: {
			int64_t i = v.get_int64().value_unsafe();

			/* Happy path: value fits in a signed int */
			if (i >= (int64_t)INT_MIN && i <= (int64_t)INT_MAX)
			{
				return value_create_int((int)i);
			}

			/* On 32-bit, long is 32 bits so int64_t values outside [LONG_MIN, LONG_MAX]
			 * would be silently truncated. Guard with a compile-time check to avoid a
			 * -Wtype-limits warning on 64-bit where long == int64_t and the comparison
			 * is trivially false. */
#if LONG_MAX < INT64_MAX
			if (i < (int64_t)LONG_MIN || i > (int64_t)LONG_MAX)
			{
				return (value)metacall_error_throw("SimdJSON", -1, NULL, "64-bit signed integer value overflows long in SimdJSON implementation");
			}
#endif

			return value_create_long((long)i);
		}

		case simdjson::dom::element_type::UINT64: {
			/* SimdJSON only uses this type for integers that do not fit in int64_t */
			uint64_t ui = v.get_uint64().value_unsafe();

			if (ui > (uint64_t)LONG_MAX)
			{
				return (value)metacall_error_throw("SimdJSON", -1, NULL, "64-bit unsigned integer value overflows long in SimdJSON implementation");
			}

			return value_create_long((long)ui);
		}

		case simdjson::dom::element_type::DOUBLE: {
			double d = v.get_double().value_unsafe();

			return value_create_double(d);
		}

		case simdjson::dom::element_type::BIGINT: {
			/* Integers exceeding 64 bits are converted into double, like RapidJSON does */
			std::string str(v.get_bigint().value_unsafe());

			return value_create_double(strtod(str.c_str(), NULL));
		}

		case simdjson::dom::element_type::STRING: {
			std::string_view str = v.get_string().value_unsafe();

			return value_create_string(str.data(), str.size());
		}

		case simdjson::dom::element_type::ARRAY: {
			simdjson::dom::array array = v.get_array().value_unsafe();

			size_t size = array.size();

			value v_array = value_create_array(NULL, size);

			value *values;

			size_t index = 0;

			if (size == 0 || v_array == NULL)
			{
				return v_array;
			}

			values = static_cast<value *>(value_to_array(v_array));

			for (simdjson::dom::element element : array)
			{
				values[index] = simd_json_serial_impl_deserialize_value(element);

				if (values[index] == NULL)
				{
					for (size_t iterator = 0; iterator < index; ++iterator)
					{
						value_type_destroy(values[iterator]);
					}

					value_destroy(v_array);

					return NULL;
				}

				++index;
			}

			return v_array;
		}

		case simdjson::dom::element_type::OBJECT: {
			simdjson::dom::object object = v.get_object().value_unsafe();

			const size_t size = object.size();

			value v_map = value_create_map(NULL, size);

			value *tuples;

			size_t index = 0;

			if (size == 0 || v_map == NULL)
			{
				return v_map;
			}

			tuples = static_cast<value *>(value_to_map(v_map));

			for (simdjson::dom::key_value_pair field : object)
			{
				const value tupla[] = {
					value_create_string(field.key.data(), field.key.size()),
					simd_json_serial_impl_deserialize_value(field.value)
				};

				if (tupla[0] == NULL || tupla[1] == NULL)
				{
					if (tupla[0] != NULL)
					{
						value_type_destroy(tupla[0]);
					}

					if (tupla[1] != NULL)
					{
						value_type_destroy(tupla[1]);
					}

					value_type_destroy(v_map);

					return NULL;
				}

				tuples[index++] = value_create_array(tupla, sizeof(tupla) / sizeof(tupla[0]));
			}

			return v_map;
		}
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Unsupported value type in SimdJSON implementation");

	return NULL;
}

value simd_json_serial_impl_deserialize(serial_handle handle, const char *buffer, size_t size)
{
	simd_json_document document = static_cast<simd_json_document>(handle);

	if (handle == NULL || buffer == NULL || size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Deserialization called with wrong arguments in SimdJSON implementation");

		return NULL;
	}

	size_t length = size - 1;

	/* SimdJSON requires SIMDJSON_PADDING readable bytes after the document, if the padding
	 * does not cross a page boundary it can be read safely without copying the buffer,
	 * otherwise the parser copies it into its own (reused) padded buffer */
	bool realloc_if_needed = ((uintptr_t)(buffer + length) % SIMD_JSON_SERIAL_PAGE_SIZE) + simdjson::SIMDJSON_PADDING > SIMD_JSON_SERIAL_PAGE_SIZE;

	simdjson::dom::element element;

	simdjson::error_code error = document->parser.parse(buffer, length, realloc_if_needed).get(element);

	if (error != simdjson::SUCCESS)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid parsing of document (%s) in SimdJSON implementation: %s",
			buffer, simdjson::error_message(error));

		return NULL;
	}

	return simd_json_serial_impl_deserialize_value(element);
}

int simd_json_serial_impl_destroy(serial_handle handle)
{
	simd_json_document document = static_cast<simd_json_document>(handle);

	if (document != NULL)
	{
		delete document;
	}

	return 0;
}
