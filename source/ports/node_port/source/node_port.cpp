/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
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

/* -- Headers -- */

#include <node_port/node_port.h>
#include <metacall/metacall.h>
#include <cstring>

#define napi_call(env, call) \
	do { \
		napi_status status = (call); \
		\
		if (status != napi_ok) \
		{ \
			const napi_extended_error_info * error_info = NULL; \
			bool is_pending; \
			napi_get_last_error_info((env), &error_info); \
			napi_is_exception_pending((env), &is_pending); \
			if (!is_pending) \
			{ \
				const char * message = (error_info->error_message == NULL) \
					? "empty error message" \
					: error_info->error_message; \
				napi_throw_error((env), NULL, message); \
				return NULL; \
			} \
		} \
	} while(0)

/* TODO: Remove this? */
#define FUNCTION_NAME_LENGTH 50
#define GENERAL_STRING_LENGTH 256

/* -- Methods -- */

void metacall_node_value_to_napi(napi_env env, void * v, napi_value * js_v)
{
	enum metacall_value_id id = metacall_value_id(v);

	switch (id)
	{
		case METACALL_BOOL :
		{
			bool b = (bool)metacall_value_to_bool(v);
			napi_get_boolean(env, b, js_v);
			break;
		}

		case METACALL_CHAR :
		{
			char c = metacall_value_to_char(v);
			const char str[2] = { c, '\0' };
			napi_create_string_utf8(env, (const char *)str, 1, js_v);
			break;
		}

		case METACALL_SHORT :
		{
			short s = metacall_value_to_short(v);
			napi_create_int32(env, (int32_t)s, js_v);
			break;
		}

		case METACALL_INT :
		{
			int n = metacall_value_to_int(v);
			napi_create_int32(env, (int32_t)n, js_v);
			break;
		}

		case METACALL_LONG :
		{
			long l = metacall_value_to_long(v);
			napi_create_int64(env, (int64_t)l, js_v);
			break;
		}

		case METACALL_FLOAT :
		{
			float f = metacall_value_to_float(v);
			napi_create_double(env, (double)f, js_v);
			break;
		}

		case METACALL_DOUBLE :
		{
			double d = metacall_value_to_double(v);
			napi_create_double(env, d, js_v);
			break;
		}

		case METACALL_STRING :
		{
			char * str = metacall_value_to_string(v);
			size_t length = metacall_value_size(v) - 1;
			napi_create_string_utf8(env, str, length, js_v);
			break;
		}

		/* BEGIN-TODO: Review Buffer, Array and Map */

		case METACALL_BUFFER:
		{
			auto bufferPtr = metacall_value_to_buffer(v);
			auto typeSize = metacall_value_size(v);
			napi_create_buffer(env, typeSize, &bufferPtr, js_v);
			break;
		}

		case METACALL_ARRAY:
		{
			auto arrayptr = metacall_value_to_array(v);
			size_t arraysize = metacall_value_size(v);
			napi_create_array_with_length(env, arraysize, js_v);
			for (size_t i = 0; i < arraysize; i++)
			{
				napi_value tempValue;
				metacall_node_value_to_napi(env, arrayptr[i], &tempValue);
				napi_set_element(env, *js_v, i, tempValue);
				//convertMetacallArray_To_NodeJsArray(env, i, arrayptr, js_v);
			}
			break;
		}

		case METACALL_MAP:
		{
			auto mapValu = metacall_value_to_map(v);
			auto mapSize = metacall_value_count(v);
			for (size_t i = 0; i < mapSize; i++)
			{
				auto value = metacall_value_to_array(mapValu[i]);
				auto key = metacall_value_to_string(value[0]);
				napi_value js_Value;
				metacall_node_value_to_napi(env, value[1], &js_Value);
				napi_set_named_property(env, *js_v, key, js_Value);
			}
			break;
		}

		/* END-TODO */

		case METACALL_PTR :
		{
			/* TODO: Implement opaque pointer */
			break;
		}

		case METACALL_NULL:
		{
			napi_get_null(env, js_v);
			break;
		}

		default :
		{
			/* TODO: This should never be reach, throw an exception */
			break;
		}
	}
}

/* BEGIN-TODO: Review the whole code from here to the bottom */

void convertNodeArray_to_Metacall_Array(napi_env env, void *metacallArgs[], napi_value js_array)
{
	size_t array_count;
	napi_get_arraybuffer_info(env, js_array, NULL, &array_count);
	for (size_t i = 0; i < array_count; i++)
	{
		napi_value element;
		napi_valuetype type;
		napi_get_element(env, js_array, i, &element);
		napi_typeof(env, element, &type);
		switch (type)
		{
		case napi_string /* constant-expression */:
			char string[GENERAL_STRING_LENGTH];
			size_t _result;
			napi_get_value_string_utf8(env, element, string, GENERAL_STRING_LENGTH, &_result);

			// checking if its a char(there is no char Javascript)
			if (_result == 1)
			{
				metacallArgs[i - 1] = metacall_value_create_char(string[0]);
			}
			else
			{
				// remember the for loop started from index 1 NOT 0.....
				metacallArgs[i - 1] = metacall_value_create_string(string, _result);
			}

			break;
		case napi_number:
			double number;
			napi_get_value_double(env, element, &number);
			metacallArgs[i] = metacall_value_create_double(number);
			break;
		case napi_boolean:
			bool truth;
			napi_get_value_bool(env, element, &truth);
			metacallArgs[i] = metacall_value_create_bool(truth);
			break;
		case napi_null:
			metacallArgs[i] = metacall_value_create_null();
			break;
		case napi_object:
			bool result;
			napi_is_array(env, element, &result);
			if (result)
			{
				size_t length;
				napi_get_arraybuffer_info(env, element, NULL, &length);
				void *metacallArrayOfValues[length];
				convertNodeArray_to_Metacall_Array(env, metacallArrayOfValues, element);
			}
		default:
			break;
		}
	}
}

napi_value metacall_node(napi_env env, napi_callback_info info)
{
	size_t argc = 0;
	napi_get_cb_info(env, info, &argc, NULL, NULL, NULL);
	napi_value argv[argc];
	void *metacallArgs[argc - 1];
	napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
	char functionName[FUNCTION_NAME_LENGTH];
	size_t writeNumber;
	napi_get_value_string_utf8(env, argv[0], functionName, FUNCTION_NAME_LENGTH, &writeNumber);
	for (size_t i = 1; i < argc; i++)
	{
		napi_valuetype type;
		napi_typeof(env, argv[i], &type);
		switch (type)
		{
		case napi_string /* constant-expression */:
			char string[256];
			size_t _result;
			napi_get_value_string_utf8(env, argv[i], string, 256, &_result);

			// checking if its a char(there is no char Javascript)
			if (_result == 1)
			{
				metacallArgs[i - 1] = metacall_value_create_char(string[0]);
			}
			else
			{
				// remember the for loop started from index 1 NOT 0.....
				metacallArgs[i - 1] = metacall_value_create_string(string, _result);
			}

			break;
		case napi_number:
			double number;
			napi_get_value_double(env, argv[i], &number);
			metacallArgs[i - 1] = metacall_value_create_double(number);
			break;
		case napi_boolean:
			bool truth;
			napi_get_value_bool(env, argv[i], &truth);
			metacallArgs[i - 1] = metacall_value_create_bool(truth);
			break;
		case napi_null:
			metacallArgs[i - 1] = metacall_value_create_null();
			break;
		case napi_object:
			bool result;
			napi_is_array(env, argv[i], &result);
			if (result)
			{
				size_t length;
				napi_get_arraybuffer_info(env, argv[i], NULL, &length);
				void *vs[length];
				convertNodeArray_to_Metacall_Array(env, vs, argv[i]);
				auto ptr = const_cast<const void **>(vs);
				metacallArgs[i - 1] = metacall_value_create_array(ptr, length);
			}
			else
			{
				bool typedResult;
				napi_is_typedarray(env, argv[i], &typedResult);
				if (typedResult)
				{
					size_t length;
					napi_typedarray_type arrayType;
					napi_get_typedarray_info(env, argv[i], &arrayType, &length, NULL, NULL, NULL);

					// I will handle conversion of typed Array..
				}
			}
		default:
			break;
		}
	}

	// Phew!!!.... after we are done converting JS types to Metacall Type into a single Array Above
	void * ptr = metacallv(functionName, metacallArgs);
	napi_value js_object;
	metacall_node_value_to_napi(env, ptr, &js_object);
	return js_object;
}

// this function is the handler of the "metacall_load_from_file"
napi_value metacall_node_load_from_file(napi_env env, napi_callback_info info)
{
	size_t argc = 2, result;
	uint32_t length_of_JS_array;
	napi_value argv[argc];
	char tagBuf[18];
	napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
	// checks will be done in the JS Wrapper..... SO we believe whatever the JS_Wrapper is passing is valid
	napi_get_value_string_utf8(env, argv[0], tagBuf, 18, &result);
	napi_get_array_length(env, argv[1], &length_of_JS_array);
	const char ** file_name_strings = new const char *[256];
	size_t _result = 0;
	for (size_t i = 0; i < length_of_JS_array; i++)
	{
		napi_value tmpValue;
		napi_get_element(env, argv[1], i, &tmpValue);
		// converting to strings
		char c_strings[256] = { 0 };
		napi_coerce_to_string(env, tmpValue, &tmpValue);
		napi_get_value_string_utf8(env, tmpValue, c_strings, 256, &_result);
		file_name_strings[i] = new char[_result + 1];
		strncpy((char *)file_name_strings[i], c_strings, _result + 1);
	}
	if(_result == 0) return NULL;
	int met_result = metacall_load_from_file(tagBuf, file_name_strings, sizeof(file_name_strings)/sizeof(file_name_strings[0]), NULL);
	if (met_result > 0)
	{
		napi_throw_error(env, NULL, "Metacall could not load from file");
		return NULL;
	}

	/* TODO */
	return NULL;
}

/* END-TODO */

/* TODO: Add documentation */
napi_value metacall_node_inspect(napi_env env, napi_callback_info)
{
	napi_value result;

	size_t size = 0;

	struct metacall_allocator_std_type std_ctx = { &malloc, &realloc, &free };

	void * allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	char * inspect_str = metacall_inspect(&size, allocator);

	if (!(inspect_str != NULL && size != 0))
	{
		napi_throw_error(env, NULL, "Invalid MetaCall inspect string");
	}

	napi_call(env, napi_create_string_utf8(env, inspect_str, size - 1, &result));

	metacall_allocator_free(allocator, inspect_str);

	metacall_allocator_destroy(allocator);

	return result;
}

/* TODO: Add documentation */
napi_value metacall_node_logs(napi_env env, napi_callback_info)
{
	struct metacall_log_stdio_type log_stdio = { stdout };

	if (metacall_log(METACALL_LOG_STDIO, (void *)&log_stdio) != 0)
	{
		napi_throw_error(env, NULL, "MetaCall failed to initialize debug logs");
	}

	return NULL;
}

/* TODO: Review documentation */
// This functions sets the necessary js functions that could be called in NodeJs
void metacall_node_exports(napi_env env, napi_value exports)
{
	const char function_metacall_str[] = "metacall";
	const char function_load_from_file_str[] = "metacall_load_from_file";
	const char function_inspect_str[] = "metacall_inspect";
	const char function_logs_str[] = "metacall_logs";

	napi_value function_metacall, function_load_from_file, function_inspect, function_logs;

	napi_create_function(env, function_metacall_str, sizeof(function_metacall_str) - 1, metacall_node, NULL, &function_metacall);
	napi_create_function(env, function_load_from_file_str, sizeof(function_load_from_file_str) - 1, metacall_node_load_from_file, NULL, &function_load_from_file);
	napi_create_function(env, function_inspect_str, sizeof(function_inspect_str) - 1, metacall_node_inspect, NULL, &function_inspect);
	napi_create_function(env, function_logs_str, sizeof(function_logs_str) - 1, metacall_node_logs, NULL, &function_logs);

	napi_set_named_property(env, exports, function_metacall_str, function_metacall);
	napi_set_named_property(env, exports, function_load_from_file_str, function_load_from_file);
	napi_set_named_property(env, exports, function_inspect_str, function_inspect);
	napi_set_named_property(env, exports, function_logs_str, function_logs);
}

/* TODO: Review documentation */
/* This function is called by NodeJs when the module is required */
napi_value metacall_node_initialize(napi_env env, napi_value exports)
{
	if (metacall_initialize() != 0)
	{
		/* TODO: Show error message (when error handling is properly implemented in the core lib) */
		napi_throw_error(env, NULL, "MetaCall failed to initialize");

		return NULL;
	}

	metacall_node_exports(env, exports);

	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, metacall_node_initialize)
