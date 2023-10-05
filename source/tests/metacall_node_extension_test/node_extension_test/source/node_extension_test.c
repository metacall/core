/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <node_api.h>

static napi_value Method(napi_env env, napi_callback_info info)
{
	napi_status status;
	napi_value world;
	(void)info;
	status = napi_create_string_utf8(env, "world", 5, &world);
	if (status != napi_ok)
	{
		return NULL;
	}
	return world;
}

#define DECLARE_NAPI_METHOD(name, func) \
	{ \
		name, 0, func, 0, 0, 0, napi_default, 0 \
	}

static napi_value Init(napi_env env, napi_value exports)
{
	napi_status status;
	napi_property_descriptor desc = DECLARE_NAPI_METHOD("hello", Method);
	status = napi_define_properties(env, exports, 1, &desc);
	if (status != napi_ok)
	{
		return NULL;
	}
	return exports;
}

NAPI_MODULE(node_extension_test, Init)
