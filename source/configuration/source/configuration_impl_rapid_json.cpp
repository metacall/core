/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <configuration/configuration_impl_rapid_json.h>

#include <log/log.h>

#include <rapidjson/document.h>

/* -- Methods -- */

int configuration_impl_rapid_json_initialize()
{
	return 0;
}

configuration_impl configuration_impl_rapid_json_load(configuration config)
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

	return document;
}

value configuration_impl_rapid_json_get(configuration config, const char * key, type_id id)
{
	rapidjson::Document & document = *((rapidjson::Document *)configuration_object_impl(config));

	const rapidjson::Value & v = document[key];

	switch (id)
	{
		case TYPE_BOOL :
		{
			if (v.IsBool() == false)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid boolean type requested in RapidJSON implementation");

				return NULL;
			}

			return value_create_bool(v.GetBool() == true ? 1L : 0L);
		}

		case TYPE_CHAR :
		{
			if (v.IsString() == false)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid character type requested in RapidJSON implementation");

				return NULL;
			}

			size_t length = v.GetStringLength();

			if (length != 1)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid character size requested in RapidJSON implementation");

				return NULL;
			}

			const char * str = v.GetString();

			return value_create_char(str[0]);
		}

		case TYPE_SHORT :
		{
			if (v.IsInt() == false && v.IsUint() == false)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid short type requested in RapidJSON implementation");

				return NULL;
			}

			if (v.IsInt() == true)
			{
				int i = v.GetInt();

				log_write("metacall", LOG_LEVEL_WARNING, "Casting integer to short (posible information lost) in RapidJSON implementation");

				return value_create_short((short)i);
			}

			if (v.IsUint() == true)
			{
				unsigned int ui = v.GetUint();

				log_write("metacall", LOG_LEVEL_WARNING, "Casting unsigned integer to short (posible information lost or overflow) in RapidJSON implementation");

				return value_create_short((short)ui);
			}

			return NULL;
		}

		case TYPE_INT :
		{
			if (v.IsInt() == false && v.IsUint() == false &&
				v.IsInt64() == false && v.IsUint64() == false)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid integer type requested in RapidJSON implementation");

				return NULL;
			}

			if (v.IsInt() == true)
			{
				int i = v.GetInt();

				return value_create_int(i);
			}

			if (v.IsUint() == true)
			{
				unsigned int ui = v.GetUint();

				log_write("metacall", LOG_LEVEL_WARNING, "Casting unsigned integer to integer (posible overflow) in RapidJSON implementation");

				return value_create_int((int)ui);
			}

			if (v.IsInt64() == true)
			{
				int64_t i = v.GetInt64();

				log_write("metacall", LOG_LEVEL_WARNING, "Casting long to int (posible information lost) in RapidJSON implementation");

				return value_create_int((int)i);
			}

			if (v.IsUint64() == true)
			{
				uint64_t ui = v.GetUint64();

				log_write("metacall", LOG_LEVEL_WARNING, "Casting unsigned long to int (posible information lost or overflow) in RapidJSON implementation");

				return value_create_int((int)ui);
			}

			return NULL;
		}

		case TYPE_LONG :
		{
			if (v.IsInt() == false && v.IsUint() == false &&
				v.IsInt64() == false && v.IsUint64() == false)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid long type requested in RapidJSON implementation");

				return NULL;
			}

			if (v.IsInt() == true)
			{
				int i = v.GetInt();

				return value_create_long((long)i);
			}

			if (v.IsUint() == true)
			{
				unsigned int ui = v.GetUint();

				log_write("metacall", LOG_LEVEL_WARNING, "Casting unsigned integer to long in RapidJSON implementation");

				return value_create_long((long)ui);
			}

			if (v.IsInt64() == true)
			{
				int64_t i = v.GetInt64();

				return value_create_long((long)i);
			}

			if (v.IsUint64() == true)
			{
				uint64_t ui = v.GetUint64();

				log_write("metacall", LOG_LEVEL_WARNING, "Casting unsigned long to int (posible overflow) in RapidJSON implementation");

				return value_create_long((long)ui);
			}

			return NULL;
		}

		case TYPE_FLOAT :
		{
			if (v.IsFloat() == false && v.IsDouble() == false)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid float type requested in RapidJSON implementation");

				return NULL;
			}

			if (v.IsFloat() == true)
			{
				float f = v.GetFloat();

				return value_create_float(f);
			}

			if (v.IsDouble() == true)
			{
				double d = v.GetDouble();

				log_write("metacall", LOG_LEVEL_WARNING, "Casting double to float (possible information lost) in RapidJSON implementation");

				return value_create_float((float)d);
			}

			return NULL;
		}

		case TYPE_DOUBLE :
		{
			if (v.IsFloat() == false && v.IsDouble() == false)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid double type requested in RapidJSON implementation");

				return NULL;
			}

			if (v.IsFloat() == true)
			{
				float f = v.GetFloat();

				return value_create_double((double)f);
			}

			if (v.IsDouble() == true)
			{
				double d = v.GetDouble();

				return value_create_double((double)d);
			}

			return NULL;
		}

		case TYPE_STRING :
		{
			if (v.IsString() == false)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid string type requested in RapidJSON implementation");

				return NULL;
			}

			size_t length = v.GetStringLength();

			const char * str = v.GetString();

			return value_create_string(str, length);
		}

		default :
		{
			break;
		}
	}

	return NULL;
}

int configuration_impl_rapid_json_unload(configuration config)
{
	rapidjson::Document * document = (rapidjson::Document *)configuration_object_impl(config);

	if (document != nullptr)
	{
		delete document;
	}

	return 0;
}

int configuration_impl_rapid_json_destroy()
{
	return 0;
}
