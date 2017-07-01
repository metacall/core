/*
 *	Configuration Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing json configuration format.
 *
 */

/* -- Headers -- */

#include <rapid_json_config/rapid_json_config_impl.h>

#include <log/log.h>

#include <rapidjson/document.h>

/* -- Private Methods -- */

static value rapid_json_config_impl_get(const rapidjson::Value & v);

/* -- Methods -- */

const char * rapid_json_config_impl_extension()
{
	static const char extension[] = "json";

	return extension;
}

int rapid_json_config_impl_initialize()
{
	return 0;
}

configuration_impl rapid_json_config_impl_load(configuration config)
{
	const char * source = configuration_object_source(config);

	rapidjson::Document * document = new rapidjson::Document();

	if (document == nullptr)
	{
		return NULL;
	}

	if (!(document->Parse(source).HasParseError() == false && document->IsObject()))
	{
		delete document;

		return NULL;
	}

	for (rapidjson::Value::ConstMemberIterator it = document->MemberBegin(); it != document->MemberEnd(); ++it)
	{
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
	}

	return document;
}

value rapid_json_config_impl_get(const rapidjson::Value & v)
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
	else if (v.IsFloat() == true)
	{
		float f = v.GetFloat();

		return value_create_float(f);
	}
	else if (v.IsDouble() == true)
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
			values[index] = rapid_json_config_impl_get(v[index]);
		}

		return value_create_list(values, size);
	}

	log_write("metacall", LOG_LEVEL_WARNING, "Unsuported type in RapidJSON implementation");

	return NULL;
}

int rapid_json_config_impl_unload(configuration config)
{
	rapidjson::Document * document = (rapidjson::Document *)configuration_object_impl(config);

	if (document != nullptr)
	{
		delete document;
	}

	return 0;
}

int rapid_json_config_impl_destroy()
{
	return 0;
}
