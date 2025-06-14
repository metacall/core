/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <rapid_json_serial/rapid_json_serial_impl.h>

#include <log/log.h>

/* Disable warnings from RapidJSON */
#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wstrict-overflow"
#elif defined(__GNUC__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wstrict-overflow"
#endif

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

/* Disable warnings from RapidJSON */
#if defined(__clang__)
	#pragma clang diagnostic pop
#elif defined(__GNUC__)
	#pragma GCC diagnostic pop
#endif

#include <sstream>

/* -- Type Definitions -- */

typedef struct rapid_json_document_type
{
	rapidjson::Document impl;
	memory_allocator allocator;

} * rapid_json_document;

/* -- Private Methods -- */

static void rapid_json_serial_impl_serialize_value(value v, rapidjson::Value *json_v);

static char *rapid_json_serial_impl_document_stringify(rapid_json_document document, size_t *size);

static value rapid_json_serial_impl_deserialize_value(const rapidjson::Value *v);

/* -- Classes -- */

rapidjson::MemoryPoolAllocator<> rapid_json_allocator;

/* -- Methods -- */

const char *rapid_json_serial_impl_extension()
{
	static const char extension[] = "json";

	return extension;
}

serial_handle rapid_json_serial_impl_initialize(memory_allocator allocator)
{
	rapid_json_document document = new rapid_json_document_type();

	if (document == nullptr)
	{
		return NULL;
	}

	document->allocator = allocator;

	return (serial_handle)document;
}

void rapid_json_serial_impl_serialize_value(value v, rapidjson::Value *json_v)
{
	type_id id = value_type_id(v);

	if (id == TYPE_BOOL)
	{
		boolean b = value_to_bool(v);

		json_v->SetBool(b == 1L ? true : false);
	}
	else if (id == TYPE_CHAR)
	{
		char str[1];

		rapidjson::SizeType length = 1;

		str[0] = value_to_char(v);

		json_v->SetString(str, length);
	}
	else if (id == TYPE_SHORT)
	{
		short s = value_to_short(v);

		json_v->SetInt((int)s);
	}
	else if (id == TYPE_INT)
	{
		int i = value_to_int(v);

		json_v->SetInt(i);
	}
	else if (id == TYPE_LONG)
	{
		long l = value_to_long(v);

		json_v->SetInt64((int64_t)l);
	}
	else if (id == TYPE_FLOAT)
	{
		float f = value_to_float(v);

		json_v->SetFloat(f);
	}
	else if (id == TYPE_DOUBLE)
	{
		double d = value_to_double(v);

		json_v->SetDouble(d);
	}
	else if (id == TYPE_STRING)
	{
		const char *str = value_to_string(v);

		size_t size = value_type_size(v);

		rapidjson::SizeType length = size > 0 ? static_cast<rapidjson::SizeType>(size - 1) : 0;

		json_v->SetString(str, length);
	}
	else if (id == TYPE_BUFFER)
	{
		rapidjson::Value &json_map = json_v->SetObject();

		rapidjson::Value json_array(rapidjson::kArrayType);

		void *buffer = value_to_buffer(v);

		size_t size = value_type_size(v);

		for (size_t iterator = 0; iterator < size; ++iterator)
		{
			const char *data = (const char *)(((uintptr_t)buffer) + iterator);

			rapidjson::Value json_inner_value;

			json_inner_value.SetUint((unsigned int)*data);

			json_array.PushBack(json_inner_value, rapid_json_allocator);
		}

		// Set data
		{
			rapidjson::Value json_member;

			json_member.SetString("data");

			json_map.AddMember(json_member, json_array, rapid_json_allocator);
		}

		// Set length
		{
			rapidjson::Value json_member, json_inner_value;

			json_member.SetString("length");

			json_inner_value.SetUint64((uint64_t)size);

			json_map.AddMember(json_member, json_inner_value, rapid_json_allocator);
		}
	}
	else if (id == TYPE_ARRAY)
	{
		rapidjson::Value &json_array = json_v->SetArray();

		value *value_array = value_to_array(v);

		size_t array_size = value_type_count(v);

		for (size_t iterator = 0; iterator < array_size; ++iterator)
		{
			value current_value = value_array[iterator];

			rapidjson::Value json_inner_value;

			rapid_json_serial_impl_serialize_value(current_value, &json_inner_value);

			json_array.PushBack(json_inner_value, rapid_json_allocator);
		}
	}
	else if (id == TYPE_MAP)
	{
		rapidjson::Value &json_map = json_v->SetObject();

		value *value_map = value_to_map(v);

		size_t map_size = value_type_count(v);

		for (size_t iterator = 0; iterator < map_size; ++iterator)
		{
			value tupla = value_map[iterator];

			value *tupla_array = value_to_array(tupla);

			rapidjson::Value json_member, json_inner_value;

			rapid_json_serial_impl_serialize_value(tupla_array[0], &json_member);

			rapid_json_serial_impl_serialize_value(tupla_array[1], &json_inner_value);

			json_map.AddMember(json_member, json_inner_value, rapid_json_allocator);
		}
	}
	else if (id == TYPE_FUTURE)
	{
		/* TODO: Improve future serialization */
		static const char str[] = "[Future]";

		size_t size = sizeof(str);

		rapidjson::SizeType length = size > 0 ? static_cast<rapidjson::SizeType>(size - 1) : 0;

		json_v->SetString(str, length);
	}
	else if (id == TYPE_FUNCTION)
	{
		/* TODO: Improve function serialization */
		static const char str[] = "[Function]";

		size_t size = sizeof(str);

		rapidjson::SizeType length = size > 0 ? static_cast<rapidjson::SizeType>(size - 1) : 0;

		json_v->SetString(str, length);
	}
	else if (id == TYPE_CLASS)
	{
		/* TODO: Improve class serialization */
		static const char str[] = "[Class]";

		size_t size = sizeof(str);

		rapidjson::SizeType length = size > 0 ? static_cast<rapidjson::SizeType>(size - 1) : 0;

		json_v->SetString(str, length);
	}
	else if (id == TYPE_OBJECT)
	{
		/* TODO: Improve object serialization */
		static const char str[] = "[Object]";

		size_t size = sizeof(str);

		rapidjson::SizeType length = size > 0 ? static_cast<rapidjson::SizeType>(size - 1) : 0;

		json_v->SetString(str, length);
	}
	else if (id == TYPE_EXCEPTION)
	{
		rapidjson::Value &json_map = json_v->SetObject();

		exception ex = value_to_exception(v);

		rapidjson::Value message_member, message_value;
		static const char message_str[] = "message";

		message_member.SetString(message_str, static_cast<rapidjson::SizeType>(sizeof(message_str) - 1));
		message_value.SetString(exception_message(ex), static_cast<rapidjson::SizeType>(strlen(exception_message(ex))));
		json_map.AddMember(message_member, message_value, rapid_json_allocator);

		rapidjson::Value label_member, label_value;
		static const char label_str[] = "label";

		label_member.SetString(label_str, static_cast<rapidjson::SizeType>(sizeof(label_str) - 1));
		label_value.SetString(exception_label(ex), static_cast<rapidjson::SizeType>(strlen(exception_label(ex))));
		json_map.AddMember(label_member, label_value, rapid_json_allocator);

		rapidjson::Value code_member, code_value;
		static const char code_str[] = "code";

		code_member.SetString(code_str, static_cast<rapidjson::SizeType>(sizeof(code_str) - 1));
		code_value.SetInt64(exception_error_code(ex));
		json_map.AddMember(code_member, code_value, rapid_json_allocator);

		rapidjson::Value stacktrace_member, stacktrace_value;
		static const char stacktrace_str[] = "stacktrace";

		stacktrace_member.SetString(stacktrace_str, static_cast<rapidjson::SizeType>(sizeof(stacktrace_str) - 1));
		stacktrace_value.SetString(exception_stacktrace(ex), static_cast<rapidjson::SizeType>(strlen(exception_stacktrace(ex))));
		json_map.AddMember(stacktrace_member, stacktrace_value, rapid_json_allocator);
	}
	else if (id == TYPE_THROWABLE)
	{
		rapidjson::Value &json_map = json_v->SetObject();

		throwable th = value_to_throwable(v);

		static const char str[] = "ExceptionThrown";

		size_t size = sizeof(str);

		rapidjson::SizeType length = static_cast<rapidjson::SizeType>(size - 1);

		rapidjson::Value json_member, json_inner_value;

		json_member.SetString(str, length);

		rapid_json_serial_impl_serialize_value(throwable_value(th), &json_inner_value);

		json_map.AddMember(json_member, json_inner_value, rapid_json_allocator);
	}
	else if (id == TYPE_PTR)
	{
		std::ostringstream ostream;

		ostream << value_to_ptr(v);

		std::string s = ostream.str();

		json_v->SetString(s.c_str(), static_cast<rapidjson::SizeType>(s.length()));
	}
	else if (id == TYPE_NULL)
	{
		json_v->SetNull();
	}
}

char *rapid_json_serial_impl_document_stringify(rapid_json_document document, size_t *size)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	document->impl.Accept(writer);
	size_t buffer_size = buffer.GetSize();
	size_t buffer_str_size = buffer_size + 1;
	char *buffer_str = static_cast<char *>(memory_allocator_allocate(document->allocator, sizeof(char) * buffer_str_size));

	if (buffer_str == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid string allocation for document stringifycation in RapidJSON implementation");
		return NULL;
	}

	strncpy(buffer_str, buffer.GetString(), buffer_size);

	buffer_str[buffer_size] = '\0';

	*size = buffer_str_size;

	return buffer_str;
}

char *rapid_json_serial_impl_serialize(serial_handle handle, value v, size_t *size)
{
	rapid_json_document document = static_cast<rapid_json_document>(handle);

	if (handle == NULL || v == NULL || size == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Serialization called with wrong arguments in RapidJSON implementation");

		return NULL;
	}

	rapid_json_serial_impl_serialize_value(v, &document->impl);

	return rapid_json_serial_impl_document_stringify(document, size);
}

value rapid_json_serial_impl_deserialize_value(const rapidjson::Value *v)
{
	if (v->IsNull())
	{
		return value_create_null();
	}
	else if (v->IsBool() == true)
	{
		return value_create_bool(v->GetBool() == true ? 1L : 0L);
	}
	/*else if (v->IsString() == true && v->GetStringLength() == 1)
	{
		const char * str = v->GetString();

		return value_create_char(str[0]);
	}*/
	else if (v->IsInt() == true)
	{
		int i = v->GetInt();

		return value_create_int(i);
	}
	else if (v->IsUint() == true)
	{
		unsigned int ui = v->GetUint();

		/* TODO: Review this, in case of underflow/overflow store it in a bigger type? */
		log_write("metacall", LOG_LEVEL_WARNING, "Casting unsigned integer to integer (posible overflow) in RapidJSON implementation");

		return value_create_int((int)ui);
	}
	else if (v->IsInt64() == true)
	{
		int64_t i = v->GetInt64();

		/* TODO: Review this, in case of underflow/overflow store it in a bigger type? */
#if LONG_MAX < INT64_MAX
		log_write("metacall", LOG_LEVEL_WARNING, "Casting long to int (posible overflow) in RapidJSON implementation");
#endif

		return value_create_long((long)i);
	}
	else if (v->IsUint64() == true)
	{
		uint64_t ui = v->GetUint64();

		/* TODO: Review this, in case of underflow/overflow store it in a bigger type? */
#if LONG_MAX < UINT64_MAX
		log_write("metacall", LOG_LEVEL_WARNING, "Casting unsigned long to int (posible overflow) in RapidJSON implementation");
#endif

		return value_create_long((long)ui);
	}
	else if (v->IsFloat() == true || v->IsLosslessFloat() == true)
	{
		float f = v->GetFloat();

		return value_create_float(f);
	}
	else if (v->IsDouble() == true || v->IsLosslessDouble() == true)
	{
		double d = v->GetDouble();

		return value_create_double((double)d);
	}
	else if (v->IsString() == true /*&& v->GetStringLength() > 1*/)
	{
		rapidjson::SizeType length = v->GetStringLength();

		const char *str = v->GetString();

		return value_create_string(str, (size_t)length);
	}
	else if (v->IsArray() == true)
	{
		rapidjson::SizeType size = v->Size();

		value v_array = value_create_array(NULL, size);

		value *values;

		size_t index = 0;

		if (size == 0 || v_array == NULL)
		{
			return v_array;
		}

		values = static_cast<value *>(value_to_array(v_array));

		for (rapidjson::Value::ConstValueIterator it = v->Begin(); it != v->End(); ++it)
		{
			values[index] = rapid_json_serial_impl_deserialize_value(it);

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
	else if (v->IsObject() == true)
	{
		const rapidjson::SizeType size = v->MemberCount();

		value v_map = value_create_map(NULL, size);

		value *tuples;

		size_t index = 0;

		if (size == 0 || v_map == NULL)
		{
			return v_map;
		}

		tuples = static_cast<value *>(value_to_map(v_map));

		for (rapidjson::Value::ConstMemberIterator it = v->MemberBegin(); it != v->MemberEnd(); ++it)
		{
			const value tupla[] = {
				rapid_json_serial_impl_deserialize_value(&it->name),
				rapid_json_serial_impl_deserialize_value(&it->value)
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

	log_write("metacall", LOG_LEVEL_ERROR, "Unsuported value type in RapidJSON implementation");

	return NULL;
}

value rapid_json_serial_impl_deserialize(serial_handle handle, const char *buffer, size_t size)
{
	rapid_json_document document = static_cast<rapid_json_document>(handle);

	if (handle == NULL || buffer == NULL || size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Deserialization called with wrong arguments in RapidJSON implementation");

		return NULL;
	}

	rapidjson::ParseResult parse_result = document->impl.Parse(buffer, size - 1);

	if (parse_result.IsError() == true)
	{
		const RAPIDJSON_ERROR_CHARTYPE *error_message = rapidjson::GetParseError_En(parse_result.Code());

		log_write("metacall", LOG_LEVEL_ERROR, "Invalid parsing of document (%s) in RapidJSON implementation: %s at %" PRIuS,
			buffer, error_message, parse_result.Offset());

		return NULL;
	}

	return rapid_json_serial_impl_deserialize_value(&document->impl);
}

int rapid_json_serial_impl_destroy(serial_handle handle)
{
	rapid_json_document document = static_cast<rapid_json_document>(handle);

	if (document != NULL)
	{
		delete document;
	}

	return 0;
}
