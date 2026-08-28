

#include <grpc_serial/grpc_serial_impl.h>

#include <metacall/metacall_error.h>

#include <log/log.h>

#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wstrict-overflow"
	#pragma clang diagnostic ignored "-Wshadow"
	#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GNUC__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wstrict-overflow"
	#pragma GCC diagnostic ignored "-Wshadow"
	#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <google/protobuf/struct.pb.h>
#include <google/protobuf/util/json_util.h>

#if defined(__clang__)
	#pragma clang diagnostic pop
#elif defined(__GNUC__)
	#pragma GCC diagnostic pop
#endif

#include <climits>
#include <sstream>
#include <string>



typedef struct grpc_document_type
{
	google::protobuf::Value impl; 
	memory_allocator allocator;

} * grpc_document;

/* -- Private Methods -- */

static void grpc_serial_impl_serialize_value(value v, google::protobuf::Value *pb_v);

static char *grpc_serial_impl_document_stringify(grpc_document document, size_t *size);

static value grpc_serial_impl_deserialize_value(const google::protobuf::Value *pb_v);

/* -- Methods -- */

const char *grpc_serial_impl_extension()
{
	static const char extension[] = "grpc";
	
	return extension;
}

serial_handle grpc_serial_impl_initialize(memory_allocator allocator)
{
	grpc_document document = new grpc_document_type();

	if (document == nullptr)
	{
		return NULL;
	}

	document->allocator = allocator;

	return (serial_handle)document;
}

/* -------------------------------------------------------------------------
 * Serialize: MetaCall value  →  google::protobuf::Value
 * ---------------------------------------------------------------------- */

void grpc_serial_impl_serialize_value(value v, google::protobuf::Value *pb_v)
{
	type_id id = value_type_id(v);

	if (id == TYPE_BOOL)
	{
		pb_v->set_bool_value(value_to_bool(v) == 1L ? true : false);
	}
	else if (id == TYPE_CHAR)
	{
		char buf[2] = { value_to_char(v), '\0' };
		pb_v->set_string_value(buf);
	}
	else if (id == TYPE_SHORT)
	{
		pb_v->set_number_value(static_cast<double>(value_to_short(v)));
	}
	else if (id == TYPE_INT)
	{
		pb_v->set_number_value(static_cast<double>(value_to_int(v)));
	}
	else if (id == TYPE_LONG)
	{
		pb_v->set_number_value(static_cast<double>(value_to_long(v)));
	}
	else if (id == TYPE_FLOAT)
	{
		pb_v->set_number_value(static_cast<double>(value_to_float(v)));
	}
	else if (id == TYPE_DOUBLE)
	{
		pb_v->set_number_value(value_to_double(v));
	}
	else if (id == TYPE_STRING)
	{
		const char *str = value_to_string(v);
		size_t size = value_type_size(v);
		/* size includes the NUL terminator */
		pb_v->set_string_value(str, size > 0 ? size - 1 : 0);
	}
	else if (id == TYPE_BUFFER)
	{
		/*
		 * Encode as: { "data": [byte0, byte1, ...], "length": N }
		 * Mirrors the RapidJSON layout so consumers can detect buffers.
		 */
		google::protobuf::Struct *st = pb_v->mutable_struct_value();

		void *buffer = value_to_buffer(v);
		size_t size = value_type_size(v);

		google::protobuf::Value data_val;
		google::protobuf::ListValue *list = data_val.mutable_list_value();

		for (size_t i = 0; i < size; ++i)
		{
			const unsigned char byte = *(static_cast<const unsigned char *>(buffer) + i);
			google::protobuf::Value *elem = list->add_values();
			elem->set_number_value(static_cast<double>(byte));
		}

		(*st->mutable_fields())["data"] = data_val;

		google::protobuf::Value len_val;
		len_val.set_number_value(static_cast<double>(size));
		(*st->mutable_fields())["length"] = len_val;
	}
	else if (id == TYPE_ARRAY)
	{
		google::protobuf::ListValue *list = pb_v->mutable_list_value();

		value *value_array = value_to_array(v);
		size_t array_size = value_type_count(v);

		for (size_t i = 0; i < array_size; ++i)
		{
			grpc_serial_impl_serialize_value(value_array[i], list->add_values());
		}
	}
	else if (id == TYPE_MAP)
	{
		google::protobuf::Struct *st = pb_v->mutable_struct_value();

		value *value_map = value_to_map(v);
		size_t map_size = value_type_count(v);

		for (size_t i = 0; i < map_size; ++i)
		{
			value tupla = value_map[i];
			value *tupla_array = value_to_array(tupla);

			/* Key must be a string in Protobuf Struct */
			google::protobuf::Value key_pb;
			grpc_serial_impl_serialize_value(tupla_array[0], &key_pb);

			std::string key_str;
			if (key_pb.kind_case() == google::protobuf::Value::kStringValue)
			{
				key_str = key_pb.string_value();
			}
			else
			{
				/* Non-string keys: convert number/bool to string representation */
				std::ostringstream oss;
				if (key_pb.kind_case() == google::protobuf::Value::kNumberValue)
				{
					oss << key_pb.number_value();
				}
				else if (key_pb.kind_case() == google::protobuf::Value::kBoolValue)
				{
					oss << (key_pb.bool_value() ? "true" : "false");
				}
				else
				{
					oss << "(null)";
				}
				key_str = oss.str();
			}

			grpc_serial_impl_serialize_value(tupla_array[1], &(*st->mutable_fields())[key_str]);
		}
	}
	else if (id == TYPE_FUTURE)
	{
		pb_v->set_string_value("[Future]");
	}
	else if (id == TYPE_FUNCTION)
	{
		pb_v->set_string_value("[Function]");
	}
	else if (id == TYPE_CLASS)
	{
		pb_v->set_string_value("[Class]");
	}
	else if (id == TYPE_OBJECT)
	{
		pb_v->set_string_value("[Object]");
	}
	else if (id == TYPE_EXCEPTION)
	{
		exception ex = value_to_exception(v);

		google::protobuf::Struct *st = pb_v->mutable_struct_value();

		auto &fields = *st->mutable_fields();

		fields["message"].set_string_value(exception_message(ex));
		fields["label"].set_string_value(exception_label(ex));
		fields["code"].set_number_value(static_cast<double>(exception_error_code(ex)));
		fields["stacktrace"].set_string_value(exception_stacktrace(ex));
	}
	else if (id == TYPE_THROWABLE)
	{
		throwable th = value_to_throwable(v);

		google::protobuf::Struct *st = pb_v->mutable_struct_value();

		grpc_serial_impl_serialize_value(throwable_value(th), &(*st->mutable_fields())["ExceptionThrown"]);
	}
	else if (id == TYPE_PTR)
	{
		std::ostringstream oss;
		oss << value_to_ptr(v);
		pb_v->set_string_value(oss.str());
	}
	else if (id == TYPE_NULL)
	{
		pb_v->set_null_value(google::protobuf::NULL_VALUE);
	}
}


char *grpc_serial_impl_document_stringify(grpc_document document, size_t *size)
{
	std::string wire;

	if (!document->impl.SerializeToString(&wire))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to serialise Protobuf Value to binary wire format in gRPC implementation");
		return NULL;
	}

	size_t wire_size = wire.size();
	size_t alloc_size = wire_size + 1; /* +1: NUL sentinel so callers can detect empty */

	char *buffer_str = static_cast<char *>(memory_allocator_allocate(document->allocator, sizeof(char) * alloc_size));

	if (buffer_str == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid string allocation for document stringification in gRPC implementation");
		return NULL;
	}

	memcpy(buffer_str, wire.data(), wire_size);
	buffer_str[wire_size] = '\0';

	*size = alloc_size;

	return buffer_str;
}


char *grpc_serial_impl_serialize(serial_handle handle, value v, size_t *size)
{
	grpc_document document = static_cast<grpc_document>(handle);

	if (handle == NULL || v == NULL || size == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Serialization called with wrong arguments in gRPC implementation");
		return NULL;
	}

	/* Reset the embedded message before reuse */
	document->impl.Clear();

	grpc_serial_impl_serialize_value(v, &document->impl);

	return grpc_serial_impl_document_stringify(document, size);
}


value grpc_serial_impl_deserialize_value(const google::protobuf::Value *pb_v)
{
	switch (pb_v->kind_case())
	{
		case google::protobuf::Value::kNullValue:
		{
			return value_create_null();
		}

		case google::protobuf::Value::kBoolValue:
		{
			return value_create_bool(pb_v->bool_value() ? 1L : 0L);
		}

		case google::protobuf::Value::kNumberValue:
		{
			double d = pb_v->number_value();

			/* Prefer the narrowest integer type that fits, mirroring
			 * the RapidJSON deserialization heuristic. */
			if (d == static_cast<double>(static_cast<int>(d)) &&
				d >= static_cast<double>(INT_MIN) &&
				d <= static_cast<double>(INT_MAX))
			{
				return value_create_int(static_cast<int>(d));
			}

			if (d == static_cast<double>(static_cast<long>(d)) &&
				d >= static_cast<double>(LONG_MIN) &&
				d <= static_cast<double>(LONG_MAX))
			{
				return value_create_long(static_cast<long>(d));
			}

			/* Fallback: preserve as double */
			return value_create_double(d);
		}

		case google::protobuf::Value::kStringValue:
		{
			const std::string &s = pb_v->string_value();
			return value_create_string(s.c_str(), s.size());
		}

		case google::protobuf::Value::kListValue:
		{
			const google::protobuf::ListValue &list = pb_v->list_value();
			int count = list.values_size();

			value v_array = value_create_array(NULL, static_cast<size_t>(count));

			if (count == 0 || v_array == NULL)
			{
				return v_array;
			}

			value *values = static_cast<value *>(value_to_array(v_array));

			for (int i = 0; i < count; ++i)
			{
				values[i] = grpc_serial_impl_deserialize_value(&list.values(i));

				if (values[i] == NULL)
				{
					for (int j = 0; j < i; ++j)
					{
						value_type_destroy(values[j]);
					}
					value_destroy(v_array);
					return NULL;
				}
			}

			return v_array;
		}

		case google::protobuf::Value::kStructValue:
		{
			const google::protobuf::Struct &st = pb_v->struct_value();
			const auto &fields = st.fields();
			size_t map_size = static_cast<size_t>(fields.size());

			value v_map = value_create_map(NULL, map_size);

			if (map_size == 0 || v_map == NULL)
			{
				return v_map;
			}

			value *tuples = static_cast<value *>(value_to_map(v_map));
			size_t index = 0;

			for (auto it = fields.begin(); it != fields.end(); ++it)
			{
				value key_val = value_create_string(it->first.c_str(), it->first.size());
				value inner_val = grpc_serial_impl_deserialize_value(&it->second);

				if (key_val == NULL || inner_val == NULL)
				{
					if (key_val != NULL)
					{
						value_type_destroy(key_val);
					}
					if (inner_val != NULL)
					{
						value_type_destroy(inner_val);
					}
					value_type_destroy(v_map);
					return NULL;
				}

				const value tupla[] = { key_val, inner_val };
				tuples[index++] = value_create_array(tupla, sizeof(tupla) / sizeof(tupla[0]));
			}

			return v_map;
		}

		default:
		{
			break;
		}
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Unsupported value kind in gRPC implementation");
	return NULL;
}

/* -------------------------------------------------------------------------
 * Public: deserialize
 * ---------------------------------------------------------------------- */

value grpc_serial_impl_deserialize(serial_handle handle, const char *buffer, size_t size)
{
	grpc_document document = static_cast<grpc_document>(handle);

	if (handle == NULL || buffer == NULL || size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Deserialization called with wrong arguments in gRPC implementation");
		return NULL;
	}

	/* size includes a NUL sentinel byte added by stringify — exclude it */
	const size_t wire_size = size - 1;

	document->impl.Clear();

	if (!document->impl.ParseFromArray(buffer, static_cast<int>(wire_size)))
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid parsing of Protobuf Value in gRPC implementation (%" PRIuS " bytes)", wire_size);
		return NULL;
	}

	return grpc_serial_impl_deserialize_value(&document->impl);
}


int grpc_serial_impl_destroy(serial_handle handle)
{
	grpc_document document = static_cast<grpc_document>(handle);

	if (document != NULL)
	{
		delete document;
	}

	return 0;
}
