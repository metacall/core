#include <node_api.h>
#include <metacall/metacall.h>
#include <cstring>
#include <node_port/node_port.h>

void convertMetacall_To_NODEJS(napi_env env, void *metacallValue, napi_value *jsObj_to_return)
{
	auto value = metacall_value_id(metacallValue);
	switch (value)
	{
	case METACALL_BOOL:{
		bool truth = (bool)metacall_value_to_bool(metacallValue);
		napi_get_boolean(env, truth, jsObj_to_return);
		break;
	}
	case METACALL_STRING:{
		//char string = metacall_value_to_string(metacallValue);
		char *string = metacall_value_to_string(metacallValue);
		napi_create_string_utf8(env, string, NAPI_AUTO_LENGTH, jsObj_to_return);
		break;
	}
	case METACALL_CHAR:{
		char character = metacall_value_to_char(metacallValue);
		char str[2] = { character, '\0' };

		// am using "1" because char is a single string
		napi_create_string_utf8(env, (const char *)str, 1, jsObj_to_return);
		break;
	}
	case METACALL_INT:{
		int number = metacall_value_to_int(metacallValue);
		napi_create_int32(env, number, jsObj_to_return);
		break;
	}
	case METACALL_FLOAT:{
		float number = metacall_value_to_float(metacallValue);
		napi_create_double(env, (double)number, jsObj_to_return);
		break;
	}
	case METACALL_LONG:{
		long number = metacall_value_to_long(metacallValue);
		napi_create_int64(env, (double)number, jsObj_to_return);
		break;
	}
	case METACALL_NULL:{
		napi_get_null(env, jsObj_to_return);
		break;
	}
	case METACALL_DOUBLE:{
		double number = metacall_value_to_double(metacallValue);
		napi_create_double(env, number, jsObj_to_return);
		break;
	}
	case METACALL_ARRAY:{
		// will do this one 2mao..
		auto arrayptr = metacall_value_to_array(metacallValue);
		size_t arraysize = metacall_value_size(metacallValue);
		napi_create_array_with_length(env, arraysize, jsObj_to_return);
		for (size_t i = 0; i < arraysize; i++)
		{
			napi_value tempValue;
			convertMetacall_To_NODEJS(env, arrayptr[i], &tempValue);
			napi_set_element(env, *jsObj_to_return, i, tempValue);
			//convertMetacallArray_To_NodeJsArray(env, i, arrayptr, jsObj_to_return);   
		}
		
		break;
	}
	case METACALL_BUFFER:{
		auto bufferPtr = metacall_value_to_buffer(metacallValue);
		auto typeSize = metacall_value_size(metacallValue);
		napi_create_buffer(env, typeSize, &bufferPtr, jsObj_to_return);
		break;
	}
	case METACALL_SHORT:{
		auto shortptr = metacall_value_to_short(metacallValue);
		napi_create_int32(env, shortptr, jsObj_to_return);
		break;
	}
	case METACALL_MAP:{
		 auto mapValu = metacall_value_to_map(metacallValue);
		auto mapSize = metacall_value_count(metacallValue);
		for (size_t i = 0; i < mapSize; i++)
		{
			auto value = metacall_value_to_array(mapValu[i]);
			auto key = metacall_value_to_string(value[0]);
			napi_value js_Value;
			convertMetacall_To_NODEJS(env, value[1], &js_Value);
			napi_set_named_property(env, *jsObj_to_return, key, js_Value);
		}
		break;
	}
	default:
		break;
	}
}
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

napi_value JS_Metacall(napi_env env, napi_callback_info info)
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
				void *metacallValues[length];
				convertNodeArray_to_Metacall_Array(env, metacallValues, argv[i]);
				auto ptr = const_cast<const void **>(metacallValues);
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
	convertMetacall_To_NODEJS(env, ptr, &js_object);
	return js_object;
}
// this function is the handler of the "metacall_load_from_file"
napi_value JS_metacall_load_file(napi_env env, napi_callback_info info)
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
		char c_strings[256];
		napi_coerce_to_string(env, tmpValue, &tmpValue);
		napi_get_value_string_utf8(env, tmpValue, c_strings, 256, &_result);
		file_name_strings[i] = new char[_result];
		strncpy((char *)file_name_strings[i], c_strings, _result);
		
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
// This functions sets the necessary js functions that could be called in NodeJs
void setMetall_JS_Properties(napi_env env, napi_value *js_obj)
{
	napi_value function_metacall_load_file, function_metacall; // the js function object
	napi_create_object(env, js_obj);
	napi_create_function(env, "metacall", NAPI_AUTO_LENGTH, JS_Metacall, NULL, &function_metacall);
	napi_create_function(env, "metacall_load_from_file", NAPI_AUTO_LENGTH, JS_metacall_load_file, NULL, &function_metacall_load_file);
	napi_set_named_property(env, *js_obj, "metacall_load_from_file", function_metacall_load_file);
	napi_set_named_property(env, *js_obj, "metacall", function_metacall);
}
/*This function is called by NodeJs when the module is imported/require*/
napi_value init(napi_env env, napi_value exports)
{
	napi_value metacall_js_object;
	// make sure you call metacall initialize...
	int result = metacall_initialize();
	if (result > 0)
	{
		napi_throw_error(env, NULL, "Sorry Could not Initialize MetaCall!");
		return NULL;
	}
	setMetall_JS_Properties(env, &metacall_js_object);
	return metacall_js_object;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init);
