/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/* TODO:
	This port is going to be refactored eventually, here is the first step through the next approach:
	https://github.com/metacall/core/blob/cb5372609c9acdc7fc3cf6b3ebb4c8e799c57f23/source/ports/node_port/source/node_port.cpp

	Here is information about how to do the refactor including loader:
	https://github.com/metacall/core/blob/cb5372609c9acdc7fc3cf6b3ebb4c8e799c57f23/source/loaders/node_loader/include/node_loader/node_loader_impl_port.h#L26
*/

/* TODO: Remove this */
#define FUNCTION_NAME_LENGTH 50
#define GENERAL_STRING_LENGTH 256

/* TODO: Remove this, it is duplicated code copied from node loader */

static inline void metacall_node_exception(napi_env env, napi_status status);

static void * metacall_node_napi_to_value(/* loader_impl_node node_impl,*/ napi_env env, napi_value recv, napi_value v);

static napi_value metacall_node_value_to_napi(/* loader_impl_node node_impl,*/ napi_env env, void * arg);

static void * metacall_node_callback_value_to_napi(size_t argc, void * args[], void * data);

static napi_value metacall_node_callback_napi_to_value(napi_env env, napi_callback_info info);

static void metacall_node_finalizer(napi_env env, napi_value v, void * data);

typedef struct metacall_node_callback_closure_type
{
	napi_env env;
	napi_ref callback_ref;
	napi_ref recv_ref;

} * metacall_node_callback_closure;

/* -- Methods -- */

/* BEGIN-TODO: (Maybe) Implement this in the loader, then remove it from here */

void * metacall_node_callback_value_to_napi(size_t argc, void * args[], void * data)
{
	metacall_node_callback_closure closure = static_cast<metacall_node_callback_closure>(data);
	napi_value ret;
	napi_status status;
	napi_value * argv = NULL;
	napi_value callback, recv;

	if (closure == NULL)
	{
		return NULL;
	}

	if (argc > 0)
	{
		argv = static_cast<napi_value *>(malloc(sizeof(napi_value *) * argc));

		if (argv == NULL)
		{
			free(closure);
			return NULL;
		}

		for (uint32_t args_count = 0; args_count < argc; ++args_count)
		{
			argv[args_count] = metacall_node_value_to_napi(closure->env, args[args_count]);
		}
	}

	status = napi_get_reference_value(closure->env, closure->recv_ref, &recv);

	metacall_node_exception(closure->env, status);

	status = napi_get_reference_value(closure->env, closure->callback_ref, &callback);

	metacall_node_exception(closure->env, status);

	status = napi_call_function(closure->env, recv, callback, argc, argv, &ret);

	metacall_node_exception(closure->env, status);

	status = napi_delete_reference(closure->env, closure->callback_ref);

	metacall_node_exception(closure->env, status);

	if (argv != NULL)
	{
		free(argv);
	}

	void * result = metacall_node_napi_to_value(closure->env, recv, ret);

	status = napi_delete_reference(closure->env, closure->recv_ref);

	metacall_node_exception(closure->env, status);

	free(closure);

	return result;
}

napi_value metacall_node_callback_napi_to_value(napi_env env, napi_callback_info info)
{
	void * f = NULL;
	size_t iterator, argc = 0;

	napi_get_cb_info(env, info, &argc, NULL, NULL, NULL);

	napi_value argv[argc];
	void * args[argc];
	napi_value recv;

	napi_get_cb_info(env, info, &argc, argv, &recv, &f);

	for (iterator = 0; iterator < argc; ++iterator)
	{
		args[iterator] = metacall_node_napi_to_value(env, recv, argv[iterator]);
	}

	void * ret = metacallfv(f, args);

	for (iterator = 0; iterator < argc; ++iterator)
	{
		metacall_value_destroy(args[iterator]);
	}

	napi_value result = metacall_node_value_to_napi(env, ret);

	metacall_node_finalizer(env, result, ret);

	return result;
}

void metacall_node_finalizer(napi_env env, napi_value v, void * data)
{
	napi_status status;

	if (metacall_value_id(data) == METACALL_NULL)
	{
		metacall_value_destroy(data);
		return;
	}

	auto finalizer = [](napi_env, void * finalize_data, void *)
	{
		metacall_value_destroy(finalize_data);
	};

	// Create a finalizer for the value
	#if (NAPI_VERSION < 5)
	{
		napi_value symbol, external;

		status = napi_create_symbol(env, nullptr, &symbol);

		metacall_node_exception(env, status);

		status = napi_create_external(env, data, finalizer, nullptr, &external);

		metacall_node_exception(env, status);

		napi_property_descriptor desc =
		{
			nullptr,
			symbol,
			nullptr,
			nullptr,
			nullptr,
			external,
			napi_default,
			nullptr
		};

		status = napi_define_properties(env, v, 1, &desc);

		metacall_node_exception(env, status);
	}
	#else // NAPI_VERSION >= 5
	{
		status = napi_add_finalizer(env, v, data, finalizer, nullptr, nullptr);

		metacall_node_exception(env, status);
	}
	#endif
}

/* END-TODO */

/* BEGIN-TODO: Remove this, it is duplicated code copied from node loader */

inline void metacall_node_exception(napi_env env, napi_status status)
{
	if (status != napi_ok)
	{
		if (status != napi_pending_exception)
		{
			const napi_extended_error_info * error_info = NULL;

			bool pending;

			napi_get_last_error_info(env, &error_info);

			napi_is_exception_pending(env, &pending);

			const char * message = (error_info != NULL && error_info->error_message != NULL) ? error_info->error_message : "Error message not available";

			/* TODO: Notify MetaCall error handling system when it is implemented */
			/* ... */

			if (pending)
			{
				napi_throw_error(env, NULL, message);
			}
		}
		else
		{
			napi_value error, message;
			bool result;
			napi_valuetype valuetype;
			size_t length;
			char * str;

			status = napi_get_and_clear_last_exception(env, &error);

			metacall_node_exception(env, status);

			status = napi_is_error(env, error, &result);

			metacall_node_exception(env, status);

			if (result == false)
			{
				/* TODO: Notify MetaCall error handling system when it is implemented */
				return;
			}

			status = napi_get_named_property(env, error, "message", &message);

			metacall_node_exception(env, status);

			status = napi_typeof(env, message, &valuetype);

			metacall_node_exception(env, status);

			if (valuetype != napi_string)
			{
				/* TODO: Notify MetaCall error handling system when it is implemented */
				return;
			}

			status = napi_get_value_string_utf8(env, message, NULL, 0, &length);

			metacall_node_exception(env, status);

			str = static_cast<char *>(malloc(sizeof(char) * (length + 1)));

			if (str == NULL)
			{
				/* TODO: Notify MetaCall error handling system when it is implemented */
				return;
			}

			status = napi_get_value_string_utf8(env, message, str, length + 1, &length);

			metacall_node_exception(env, status);

			/* TODO: Notify MetaCall error handling system when it is implemented */
			/* Meanwhile, throw it again */
			status = napi_throw_error(env, nullptr, str);

			metacall_node_exception(env, status);

			free(str);
		}
	}
}

void * metacall_node_napi_to_value(/*loader_impl_node node_impl,*/ napi_env env, napi_value recv, napi_value v)
{
	void * ret = NULL;

	napi_valuetype valuetype;

	napi_status status = napi_typeof(env, v, &valuetype);

	metacall_node_exception(env, status);

	if (valuetype == napi_undefined || valuetype == napi_null)
	{
		/* TODO: Review this, type null will be lost due to mapping of two N-API types into one metacall type */
		ret = metacall_value_create_null();
	}
	else if (valuetype == napi_boolean)
	{
		bool b;

		status = napi_get_value_bool(env, v, &b);

		metacall_node_exception(env, status);

		ret = metacall_value_create_bool((b == true) ? static_cast<boolean>(1) : static_cast<boolean>(0));
	}
	else if (valuetype == napi_number)
	{
		double d;

		status = napi_get_value_double(env, v, &d);

		metacall_node_exception(env, status);

		ret = metacall_value_create_double(d);
	}
	else if (valuetype == napi_string)
	{
		size_t length;

		status = napi_get_value_string_utf8(env, v, NULL, 0, &length);

		metacall_node_exception(env, status);

		ret = metacall_value_create_string(NULL, length);

		if (ret != NULL)
		{
			char * str = metacall_value_to_string(ret);

			status = napi_get_value_string_utf8(env, v, str, length + 1, &length);

			metacall_node_exception(env, status);
		}
	}
	else if (valuetype == napi_symbol)
	{
		/* TODO */
	}
	else if (valuetype == napi_object)
	{
		bool result = false;

		if (napi_is_array(env, v, &result) == napi_ok && result == true)
		{
			uint32_t iterator, length = 0;

			void ** array_value;

			status = napi_get_array_length(env, v, &length);

			metacall_node_exception(env, status);

			ret = metacall_value_create_array(NULL, static_cast<size_t>(length));

			array_value = metacall_value_to_array(ret);

			for (iterator = 0; iterator < length; ++iterator)
			{
				napi_value element;

				status = napi_get_element(env, v, iterator, &element);

				metacall_node_exception(env, status);

				/* TODO: Review recursion overflow */
				array_value[iterator] = metacall_node_napi_to_value(/*node_impl,*/ env, recv, element);
			}
		}
		else if (napi_is_buffer(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
		}
		else if (napi_is_error(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
		}
		else if (napi_is_typedarray(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
		}
		else if (napi_is_dataview(env, v, &result) == napi_ok && result == true)
		{
			/* TODO */
		}
		else if (napi_is_promise(env, v, &result) == napi_ok && result == true)
		{
			/* TODO: Future is implemented by means of node_loader */

			napi_throw_error(env, NULL, "Promises are not supported yet in NodeJS Port");

			#if 0
				loader_impl_node_future node_future = static_cast<loader_impl_node_future>(malloc(sizeof(struct loader_impl_node_future_type)));

				future f;

				if (node_future == NULL)
				{
					return static_cast<function_return>(NULL);
				}

				f = future_create(node_future, &future_node_singleton);

				if (f == NULL)
				{
					free(node_future);

					return static_cast<function_return>(NULL);
				}

				ret = metacall_value_create_future(f);

				if (ret == NULL)
				{
					future_destroy(f);
				}

				/* Create reference to promise */
				node_future->node_impl = node_impl;

				status = napi_create_reference(env, v, 1, &node_future->promise_ref);

				metacall_node_exception(env, status);
			#endif // 0
		}
		else
		{
			/* TODO: Strict check if it is an object (map) */
			uint32_t iterator, length = 0;

			napi_value keys;

			void ** map_value;

			status = napi_get_property_names(env, v, &keys);

			metacall_node_exception(env, status);

			status = napi_get_array_length(env, keys, &length);

			metacall_node_exception(env, status);

			ret = metacall_value_create_map(NULL, static_cast<size_t>(length));

			map_value = metacall_value_to_map(ret);

			for (iterator = 0; iterator < length; ++iterator)
			{
				napi_value key;

				size_t key_length;

				void ** tupla;

				/* Create tupla */
				map_value[iterator] = metacall_value_create_array(NULL, 2);

				tupla = metacall_value_to_array(map_value[iterator]);

				/* Get key from object */
				status = napi_get_element(env, keys, iterator, &key);

				metacall_node_exception(env, status);

				/* Set key string in the tupla */
				status = napi_get_value_string_utf8(env, key, NULL, 0, &key_length);

				metacall_node_exception(env, status);

				tupla[0] = metacall_value_create_string(NULL, key_length);

				if (tupla[0] != NULL)
				{
					napi_value element;

					char * str = metacall_value_to_string(tupla[0]);

					status = napi_get_value_string_utf8(env, key, str, key_length + 1, &key_length);

					metacall_node_exception(env, status);

					status = napi_get_property(env, v, key, &element);

					metacall_node_exception(env, status);

					/* TODO: Review recursion overflow */
					tupla[1] = metacall_node_napi_to_value(/* node_impl,*/ env, recv, element);
				}
			}

		}
	}
	else if (valuetype == napi_function)
	{
		void * f = metacall_function("__metacall_node_callback_value_to_napi__");
		metacall_node_callback_closure closure = static_cast<metacall_node_callback_closure>(malloc(sizeof(struct metacall_node_callback_closure_type)));
		napi_value length;
		uint32_t argc;

		closure->env = env;

		// Create a reference to this
		status = napi_create_reference(env, recv, 1, &closure->recv_ref);

		metacall_node_exception(env, status);

		// Create a reference to the callback
		status = napi_create_reference(env, v, 1, &closure->callback_ref);

		metacall_node_exception(env, status);

		// Get number of arguments to the callback (this workaround should be reviewed)
		status = napi_get_named_property(env, v, "length", &length);

		metacall_node_exception(env, status);

		status = napi_get_value_uint32(env, length, &argc);

		metacall_node_exception(env, status);

		// Workaround for accepting variable arguments
		metacall_function_resize(f, (size_t)argc);

		return metacall_value_create_function_closure(f, (void *)closure);
	}
	else if (valuetype == napi_external)
	{
		/* Returns the previously allocated copy */
		void * c = NULL;

		status = napi_get_value_external(env, v, &c);

		metacall_node_exception(env, status);

		return c;
	}

	return ret;
}

napi_value metacall_node_value_to_napi(/* loader_impl_node node_impl,*/ napi_env env, void * arg)
{
	void * arg_value = static_cast<void *>(arg);

	enum metacall_value_id id = metacall_value_id(arg_value);

	napi_status status;

	napi_value v = nullptr;

	if (id == METACALL_BOOL)
	{
		boolean bool_value = metacall_value_to_bool(arg_value);

		status = napi_get_boolean(env, (bool_value == 0) ? false : true, &v);

		metacall_node_exception(env, status);
	}
	else if (id == METACALL_CHAR)
	{
		char char_value = metacall_value_to_char(arg_value);

		status = napi_create_int32(env, static_cast<int32_t>(char_value), &v);

		metacall_node_exception(env, status);
	}
	else if (id == METACALL_SHORT)
	{
		short short_value = metacall_value_to_short(arg_value);

		status = napi_create_int32(env, static_cast<int32_t>(short_value), &v);

		metacall_node_exception(env, status);
	}
	else if (id == METACALL_INT)
	{
		int int_value = metacall_value_to_int(arg_value);

		/* TODO: Check integer overflow */
		status = napi_create_int32(env, static_cast<int32_t>(int_value), &v);

		metacall_node_exception(env, status);
	}
	else if (id == METACALL_LONG)
	{
		long long_value = metacall_value_to_long(arg_value);

		/* TODO: Check integer overflow */
		status = napi_create_int64(env, static_cast<int64_t>(long_value), &v);

		metacall_node_exception(env, status);
	}
	else if (id == METACALL_FLOAT)
	{
		float float_value = metacall_value_to_float(arg_value);

		status = napi_create_double(env, static_cast<double>(float_value), &v);

		metacall_node_exception(env, status);
	}
	else if (id == METACALL_DOUBLE)
	{
		double double_value = metacall_value_to_double(arg_value);

		status = napi_create_double(env, double_value, &v);

		metacall_node_exception(env, status);
	}
	else if (id == METACALL_STRING)
	{
		const char * str_value = metacall_value_to_string(arg_value);

		size_t length = metacall_value_size(arg_value) - 1;

		status = napi_create_string_utf8(env, str_value, length, &v);

		metacall_node_exception(env, status);
	}
	else if (id == METACALL_BUFFER)
	{
		void * buff_value = metacall_value_to_buffer(arg_value);

		size_t size = metacall_value_size(arg_value);

		status = napi_create_buffer(env, size, &buff_value, &v);

		metacall_node_exception(env, status);
	}
	else if (id == METACALL_ARRAY)
	{
		void ** array_value = metacall_value_to_array(arg_value);

		size_t array_size = metacall_value_count(arg_value);

		uint32_t iterator;

		status = napi_create_array_with_length(env, array_size, &v);

		metacall_node_exception(env, status);

		for (iterator = 0; iterator < array_size; ++iterator)
		{
			/* TODO: Review recursion overflow */
			napi_value element_v = metacall_node_value_to_napi(/*node_impl,*/ env, static_cast<void *>(array_value[iterator]));

			status = napi_set_element(env, v, iterator, element_v);

			metacall_node_exception(env, status);
		}
	}
	else if (id == METACALL_MAP)
	{
		void ** map_value = metacall_value_to_map(arg_value);

		size_t iterator, map_size = metacall_value_count(arg_value);

		status = napi_create_object(env, &v);

		metacall_node_exception(env, status);

		for (iterator = 0; iterator < map_size; ++iterator)
		{
			void ** pair_value = metacall_value_to_array(map_value[iterator]);

			const char * key = metacall_value_to_string(pair_value[0]);

			/* TODO: Review recursion overflow */
			napi_value element_v = metacall_node_value_to_napi(/*node_impl,*/ env, static_cast<void *>(pair_value[1]));

			status = napi_set_named_property(env, v, key, element_v);

			metacall_node_exception(env, status);
		}
	}
	else if (id == METACALL_PTR)
	{
		// Copy value and set the ownership, the old value will be deleted after the call
		void * c = metacall_value_copy(arg_value);

		metacall_value_move(arg_value, c);

		status = napi_create_external(env, c, nullptr, nullptr, &v);

		metacall_node_exception(env, status);
	}
	else if (id == METACALL_FUTURE)
	{
		/* TODO: Implement promise properly for await */
	}
	else if (id == METACALL_FUNCTION)
	{
		void * f = metacall_value_to_function(arg_value);

		size_t length = metacall_function_size(f);

		napi_value length_v;

		status = napi_create_function(env, NULL, 0, metacall_node_callback_napi_to_value, f, &v);

		metacall_node_exception(env, status);

		status = napi_create_uint32(env, static_cast<uint32_t>(length), &length_v);

		metacall_node_exception(env, status);

		status = napi_set_named_property(env, v, "length", length_v);

		metacall_node_exception(env, status);
	}
	else if (id == METACALL_NULL)
	{
		status = napi_get_undefined(env, &v);

		metacall_node_exception(env, status);
	}
	else
	{
		napi_throw_error(env, NULL, "MetaCall could not convert the value to N-API");
	}

	return v;
}

/* END-TODO */

/* TODO: Review this against buffer overflows, remove constants and use of stack */
napi_value metacall_node_call(napi_env env, napi_callback_info info)
{
	size_t argc = 0;

	napi_get_cb_info(env, info, &argc, NULL, NULL, NULL);

	napi_value argv[argc];
	void * args[argc - 1];
	napi_value recv;

	napi_get_cb_info(env, info, &argc, argv, &recv, NULL);

	char name[FUNCTION_NAME_LENGTH];
	size_t name_length;

	napi_status status = napi_get_value_string_utf8(env, argv[0], name, FUNCTION_NAME_LENGTH, &name_length);

	metacall_node_exception(env, status);

	for (size_t i = 1; i < argc; ++i)
	{
		args[i - 1] = metacall_node_napi_to_value(env, recv, argv[i]);
	}

	void * ret = metacallv(name, args);

	for (size_t args_count = 0; args_count < argc - 1; ++args_count)
	{
		metacall_value_destroy(args[args_count]);
	}

	napi_value result = metacall_node_value_to_napi(env, ret);

	metacall_node_finalizer(env, result, ret);

	return result;
}

napi_value metacall_node_load_from_file(napi_env env, napi_callback_info info)
{
	const size_t args_size = 2;
	size_t argc = args_size, result;
	napi_value argv[args_size];
	uint32_t length_of_JS_array;
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
	int met_result = metacall_load_from_file(tagBuf, file_name_strings, length_of_JS_array, NULL);
	if (met_result > 0)
	{
		napi_throw_error(env, NULL, "MetaCall could not load from file");
		return NULL;
	}

	/* TODO */
	return NULL;
}

/* END-TODO */

/**
*  @brief
*    Loads a script from string by tag
*
*  @param[in] env
*    N-API reference to the enviroment
*
*  @param[in] info
*    Reference to the call information
*
*  @return
*    TODO: Not implemented yet
*/
napi_value metacall_node_load_from_memory(napi_env env, napi_callback_info info)
{
	const size_t args_size = 2;
	size_t argc = args_size, tag_length, script_length;
	napi_value argv[args_size];
	napi_status status;
	char * tag, * script;

	// Get arguments
	status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

	metacall_node_exception(env, status);

	// Get tag length
	status = napi_get_value_string_utf8(env, argv[0], NULL, 0, &tag_length);

	metacall_node_exception(env, status);

	// Allocate tag
	tag = static_cast<char *>(malloc(sizeof(char) * (tag_length + 1)));

	if (tag == NULL)
	{
		napi_throw_error(env, NULL, "MetaCall could not load from memory, tag allocation failed");
		return NULL;
	}

	// Get tag
	status = napi_get_value_string_utf8(env, argv[0], tag, tag_length + 1, &tag_length);

	metacall_node_exception(env, status);

	// Get script length
	status = napi_get_value_string_utf8(env, argv[1], NULL, 0, &script_length);

	metacall_node_exception(env, status);

	// Allocate script
	script = static_cast<char *>(malloc(sizeof(char) * (script_length + 1)));

	if (script == NULL)
	{
		free(tag);
		napi_throw_error(env, NULL, "MetaCall could not load from memory, script allocation failed");
		return NULL;
	}

	// Get script
	status = napi_get_value_string_utf8(env, argv[1], script, script_length + 1, &script_length);

	metacall_node_exception(env, status);

	// Load script from memory
	if (metacall_load_from_memory(tag, script, script_length, NULL) != 0)
	{
		free(tag);
		free(script);
		napi_throw_error(env, NULL, "MetaCall could not load from memory");
		return NULL;
	}

	free(tag);
	free(script);

	/* TODO: Return value and logs */
	return NULL;
}

/* TODO: Add documentation */
napi_value metacall_node_inspect(napi_env env, napi_callback_info)
{
	napi_value result;

	size_t size = 0;

	struct metacall_allocator_std_type std_ctx = { &malloc, &realloc, &free };

	void * allocator = metacall_allocator_create(METACALL_ALLOCATOR_STD, (void *)&std_ctx);

	char * inspect_str = metacall_inspect(&size, allocator);

	napi_status status;

	if (!(inspect_str != NULL && size != 0))
	{
		napi_throw_error(env, NULL, "Invalid MetaCall inspect string");
	}

	status = napi_create_string_utf8(env, inspect_str, size - 1, &result);

	metacall_node_exception(env, status);

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
	const char function_load_from_memory_str[] = "metacall_load_from_memory";
	const char function_inspect_str[] = "metacall_inspect";
	const char function_logs_str[] = "metacall_logs";

	napi_value function_metacall, function_load_from_file, function_load_from_memory, function_inspect, function_logs;

	napi_create_function(env, function_metacall_str, sizeof(function_metacall_str) - 1, metacall_node_call, NULL, &function_metacall);
	napi_create_function(env, function_load_from_file_str, sizeof(function_load_from_file_str) - 1, metacall_node_load_from_file, NULL, &function_load_from_file);
	napi_create_function(env, function_load_from_memory_str, sizeof(function_load_from_memory_str) - 1, metacall_node_load_from_memory, NULL, &function_load_from_memory);
	napi_create_function(env, function_inspect_str, sizeof(function_inspect_str) - 1, metacall_node_inspect, NULL, &function_inspect);
	napi_create_function(env, function_logs_str, sizeof(function_logs_str) - 1, metacall_node_logs, NULL, &function_logs);

	napi_set_named_property(env, exports, function_metacall_str, function_metacall);
	napi_set_named_property(env, exports, function_load_from_file_str, function_load_from_file);
	napi_set_named_property(env, exports, function_load_from_memory_str, function_load_from_memory);
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

	if (metacall_register("__metacall_node_callback_value_to_napi__", metacall_node_callback_value_to_napi, METACALL_INVALID, 0) != 0)
	{
		/* TODO: Show error message (when error handling is properly implemented in the core lib) */
		napi_throw_error(env, NULL, "MetaCall failed to initialize callback support");

		return NULL;
	}

	metacall_node_exports(env, exports);

	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, metacall_node_initialize)
