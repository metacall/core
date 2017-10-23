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

/* -- Private Methods -- */

static value rapid_json_serial_impl_deserialize_value(const rapidjson::Value & v);

/* -- Methods -- */

const char * rapid_json_serial_impl_extension()
{
	static const char extension[] = "json";

	return extension;
}

serial_impl rapid_json_serial_impl_initialize()
{
	rapidjson::Document * document = new rapidjson::Document();

	if (document == nullptr)
	{
		return NULL;
	}

	return document;
}

const char * rapid_json_serial_impl_serialize(serial_impl impl, value v, size_t * size)
{
	/* TODO */

	return NULL;
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

		return value_create_list(values, size);
	}
	else if (v.IsObject() == true)
	{
		/* TODO: Implement map conversion */
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Unsuported value type in RapidJSON implementation");

	return NULL;
}

value rapid_json_serial_impl_deserialize(serial_impl impl, const char * buffer, size_t size)
{
	rapidjson::Document * document = (rapidjson::Document *)impl;

	if (impl == NULL || buffer == NULL || size == 0)
	{
		return NULL;
	}

	if (document->Parse(buffer, size - 1).HasParseError() == false)
	{
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

		return value_create_list(values, size);
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

int rapid_json_serial_impl_destroy(serial_impl impl)
{
	rapidjson::Document * document = (rapidjson::Document *)impl;

	if (document != nullptr)
	{
		delete document;
	}

	return 0;
}
