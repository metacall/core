/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <rapid_json_serial/rapid_json_serial_impl.h>

#include <log/log.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

/* -- Private Methods -- */

static void rapid_json_serial_impl_serialize_value(value v, rapidjson::Value & json_value, rapidjson::Document::AllocatorType & allocator);

static const char * rapid_json_serial_impl_document_stringify(rapidjson::Document * document, size_t * size);

static value rapid_json_serial_impl_deserialize_value(const rapidjson::Value & v);

/* -- Methods -- */

const char * rapid_json_serial_impl_extension()
{
	static const char extension[] = "json";

	return extension;
}

serial_impl_handle rapid_json_serial_impl_initialize()
{
	rapidjson::Document * document = new rapidjson::Document();

	if (document == nullptr)
	{
		return NULL;
	}

	return (serial_impl_handle)document;
}

void rapid_json_serial_impl_serialize_value(value v, rapidjson::Value & json_value, rapidjson::Document::AllocatorType & allocator)
{
	type_id id = value_type_id(v);

	if (id == TYPE_BOOL)
	{
		boolean b = value_to_bool(v);

		json_value.SetBool(b == 1L ? true : false);
	}
	else if (id == TYPE_CHAR)
	{
		char str[1];

		size_t length = 1;

		str[0] = value_to_char(v);

		json_value.SetString(str, length);
	}
	else if (id == TYPE_SHORT)
	{
		short s = value_to_short(v);

		int i = (int)s;

		json_value.SetInt(i);
	}
	else if (id == TYPE_INT)
	{
		int i = value_to_int(v);

		json_value.SetInt(i);
	}
	else if (id == TYPE_LONG)
	{
		long l = value_to_long(v);

		log_write("metacall", LOG_LEVEL_WARNING, "Casting long to int64_t (posible incompatible types) in RapidJSON implementation");

		json_value.SetInt64(l);
	}
	else if (id == TYPE_FLOAT)
	{
		float f = value_to_float(v);

		json_value.SetFloat(f);
	}
	else if (id == TYPE_DOUBLE)
	{
		double d = value_to_double(v);

		json_value.SetDouble(d);
	}
	else if (id == TYPE_STRING)
	{
		const char * str = value_to_string(v);

		size_t size = value_type_size(v);

		size_t length = size > 0 ? size - 1 : 0;

		json_value.SetString(str, length);
	}
	else if (id == TYPE_BUFFER)
	{
		/* TODO: Implement array-like map */
	}
	else if (id == TYPE_ARRAY)
	{
		rapidjson::Value & json_array = json_value.SetArray();

		value * value_array = value_to_array(v);

		size_t array_size = value_type_size(v) / sizeof(const value);

		for (size_t index = 0; index < array_size; ++array_size)
		{
			value current_value = value_array[index];

			rapidjson::Value json_inner_value;

			rapid_json_serial_impl_serialize_value(current_value, json_inner_value);

			json_array.PushBack(json_inner_value, allocator);
		}
	}
	/*else if (id == TYPE_MAP)
	{
		for (rapidjson::Value::ConstMemberIterator it = document->MemberBegin(); it != document->MemberEnd(); ++it)
		{
			*//* TODO: Implement map conversion */
			/*
			value v = rapid_json_config_impl_get(it->value);

			if (v != NULL)
			{
			if (configuration_object_set(config, it->name.GetString(), v) != 0)
			{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid value insertion in RapidJSON implementation");

			delete document;

			return NULL;
			}
			}
			*//*
		}
	}*/
	else if (id == TYPE_PTR)
	{
		const size_t PTR_STR_MAX_SIZE = 19; /* 16 (64-bit pointer to string) + 2 (0x prefix) + '\0' */

		char ptr_str[PTR_STR_MAX_SIZE] = { 0 };

		size_t length = 0;

		value_stringify(v, ptr_str, PTR_STR_MAX_SIZE, &length);

		json_value.SetString(ptr_str, length);
	}
}

const char * rapid_json_serial_impl_document_stringify(rapidjson::Document * document, size_t * size)
{
	char * buffer_str;

	size_t buffer_size = 0;

	rapidjson::StringBuffer buffer;

	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	document->Accept(writer);

	buffer_size = buffer.GetSize();

	buffer_str = static_cast<char *>(malloc(sizeof(char) * buffer_size));

	if (buffer_str == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid string allocation for document stringifycation in RapidJSON implementation");

		return NULL;
	}

	strncpy(buffer_str, buffer.GetString(), buffer_size);

	*size = buffer_size;

	return buffer_str;
}

const char * rapid_json_serial_impl_serialize(serial_impl_handle handle, value v, size_t * size)
{
	rapidjson::Document * document = (rapidjson::Document *)handle;

	type_id id;

	if (handle == NULL || v == NULL || size == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Serialization called with wrong arguments in RapidJSON implementation");

		return NULL;
	}

	id = value_type_id(v);

	if (id == TYPE_BOOL)
	{
		boolean b = value_to_bool(v);

		document->SetBool(b == 1L ? true : false);
	}
	else if (id == TYPE_CHAR)
	{
		char str[1];

		size_t length = 1;
		
		str[0] = value_to_char(v);

		document->SetString(str, length);
	}
	else if (id == TYPE_SHORT)
	{
		short s = value_to_short(v);

		int i = (int)s;

		document->SetInt(i);
	}
	else if (id == TYPE_INT)
	{
		int i = value_to_int(v);

		document->SetInt(i);
	}
	else if (id == TYPE_LONG)
	{
		long l = value_to_long(v);

		log_write("metacall", LOG_LEVEL_WARNING, "Casting long to int64_t (posible incompatible types) in RapidJSON implementation");

		document->SetInt64(l);
	}
	else if (id == TYPE_FLOAT)
	{
		float f = value_to_float(v);

		document->SetFloat(f);
	}
	else if (id == TYPE_DOUBLE)
	{
		double d = value_to_double(v);

		document->SetDouble(d);
	}
	else if (id == TYPE_STRING)
	{
		const char * str = value_to_string(v);

		size_t size = value_type_size(v);

		size_t length = size > 0 ? size - 1 : 0;

		document->SetString(str, length);
	}
	else if (id == TYPE_BUFFER)
	{
		/* TODO: Implement array-like map */
	}
	else if (id == TYPE_ARRAY)
	{
		rapidjson::Document::AllocatorType & allocator = document->GetAllocator();

		rapidjson::Value & json_array = document->SetArray();

		value * value_array = value_to_array(v);

		size_t array_size = value_type_size(v) / sizeof(const value);

		for (size_t index = 0; index < array_size; ++array_size)
		{
			value current_value = value_array[index];

			rapidjson::Value json_value;

			rapid_json_serial_impl_serialize_value(current_value, json_value, allocator);

			json_array.PushBack(json_value, allocator);
		}
	}
	/*else if (id == TYPE_MAP)
	{
		for (rapidjson::Value::ConstMemberIterator it = document->MemberBegin(); it != document->MemberEnd(); ++it)
		{
			*//* TODO: Implement map conversion */
			/*
			value v = rapid_json_config_impl_get(it->value);

			if (v != NULL)
			{
			if (configuration_object_set(config, it->name.GetString(), v) != 0)
			{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid value insertion in RapidJSON implementation");

			delete document;

			return NULL;
			}
			}
			*//*
		}
	}*/
	else if (id == TYPE_PTR)
	{
		const size_t PTR_STR_MAX_SIZE = 19; /* 16 (64-bit pointer to string) + 2 (0x prefix) + '\0' */

		char ptr_str[PTR_STR_MAX_SIZE] = { 0 };

		size_t length = 0;

		value_stringify(v, ptr_str, PTR_STR_MAX_SIZE, &length);

		document->SetString(ptr_str, length);
	}

	return rapid_json_serial_impl_document_stringify(document, size);
}

value rapid_json_serial_impl_deserialize_value(const rapidjson::Value & v)
{
	if (v.IsBool() == true)
	{
		return value_create_bool(v.GetBool() == true ? 1L : 0L);
	}
	else if (v.IsString() == true && v.GetStringLength() == 1)
	{
		const char * str = v.GetString();

		return value_create_char(str[0]);
	}
	else if (v.IsInt() == true)
	{
		int i = v.GetInt();

		return value_create_int(i);
	}
	else if (v.IsUint() == true)
	{
		unsigned int ui = v.GetUint();

		log_write("metacall", LOG_LEVEL_WARNING, "Casting unsigned integer to integer (posible overflow) in RapidJSON implementation");

		return value_create_int((int)ui);
	}
	else if (v.IsInt64() == true)
	{
		int64_t i = v.GetInt64();

		return value_create_long((long)i);
	}
	else if (v.IsUint64() == true)
	{
		uint64_t ui = v.GetUint64();

		log_write("metacall", LOG_LEVEL_WARNING, "Casting unsigned long to int (posible overflow) in RapidJSON implementation");

		return value_create_long((long)ui);
	}
	else if (v.IsFloat() == true || v.IsLosslessFloat() == true)
	{
		float f = v.GetFloat();

		return value_create_float(f);
	}
	else if (v.IsDouble() == true || v.IsLosslessDouble() == true)
	{
		double d = v.GetDouble();

		return value_create_double((double)d);
	}
	else if (v.IsString() == true && v.GetStringLength() > 1)
	{
		size_t length = v.GetStringLength();

		const char * str = v.GetString();

		return value_create_string(str, length);
	}
	else if (v.IsArray() == true && v.Empty() == false)
	{
		rapidjson::SizeType index, size = v.Size();

		value * values = static_cast<value *>(malloc(sizeof(value) * size));

		if (values == NULL)
		{
			return NULL;
		}

		for (index = 0; index < size; ++index)
		{
			values[index] = rapid_json_serial_impl_deserialize_value(v[index]);
		}

		return value_create_array(values, size);
	}
	else if (v.IsObject() == true)
	{
		/* TODO: Implement map conversion */
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Unsuported value type in RapidJSON implementation");

	return NULL;
}

value rapid_json_serial_impl_deserialize(serial_impl_handle handle, const char * buffer, size_t size)
{
	rapidjson::Document * document = (rapidjson::Document *)handle;

	if (handle == NULL || buffer == NULL || size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Deserialization called with wrong arguments in RapidJSON implementation");

		return NULL;
	}

	if (document->Parse(buffer, size - 1).HasParseError() == false)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid parsing of document (%s) in RapidJSON implementation", buffer);

		delete document;

		return NULL;
	}

	if (document->IsBool() == true)
	{
		return value_create_bool(document->GetBool() == true ? 1L : 0L);
	}
	else if (document->IsString() == true && document->GetStringLength() == 1)
	{
		const char * str = document->GetString();

		return value_create_char(str[0]);
	}
	else if (document->IsInt() == true)
	{
		int i = document->GetInt();

		return value_create_int(i);
	}
	else if (document->IsUint() == true)
	{
		unsigned int ui = document->GetUint();

		log_write("metacall", LOG_LEVEL_WARNING, "Casting unsigned integer to integer (posible overflow) in RapidJSON implementation");

		return value_create_int((int)ui);
	}
	else if (document->IsInt64() == true)
	{
		int64_t i = document->GetInt64();

		return value_create_long((long)i);
	}
	else if (document->IsUint64() == true)
	{
		uint64_t ui = document->GetUint64();

		log_write("metacall", LOG_LEVEL_WARNING, "Casting unsigned long to int (posible overflow) in RapidJSON implementation");

		return value_create_long((long)ui);
	}
	else if (document->IsFloat() == true || document->IsLosslessFloat() == true)
	{
		float f = document->GetFloat();

		return value_create_float(f);
	}
	else if (document->IsDouble() == true || document->IsLosslessDouble() == true)
	{
		double d = document->GetDouble();

		return value_create_double((double)d);
	}
	else if (document->IsString() == true && document->GetStringLength() > 1)
	{
		size_t length = document->GetStringLength();

		const char * str = document->GetString();

		return value_create_string(str, length);
	}
	else if (document->IsArray() == true && document->Empty() == false)
	{
		rapidjson::SizeType index, size = document->Size();

		value * values = static_cast<value *>(malloc(sizeof(value)* size));

		if (values == NULL)
		{
			return NULL;
		}

		for (index = 0; index < size; ++index)
		{
			values[index] = rapid_json_serial_impl_deserialize_value(document[index]);
		}

		return value_create_array(values, size);
	}
	else if (document->IsObject() == true)
	{
		for (rapidjson::Value::ConstMemberIterator it = document->MemberBegin(); it != document->MemberEnd(); ++it)
		{
			/* TODO: Implement map conversion */
			/*
			value v = rapid_json_config_impl_get(it->value);

			if (v != NULL)
			{
				if (configuration_object_set(config, it->name.GetString(), v) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Invalid value insertion in RapidJSON implementation");

					delete document;

					return NULL;
				}
			}
			*/
		}
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Unsuported document type in RapidJSON implementation");

	return NULL;
}

int rapid_json_serial_impl_destroy(serial_impl_handle handle)
{
	rapidjson::Document * document = (rapidjson::Document *)handle;

	if (document != nullptr)
	{
		delete document;
	}

	return 0;
}
