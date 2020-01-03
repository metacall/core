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

#ifndef METACALL_NODE_PORT_H
#define METACALL_NODE_PORT_H 1

/* -- Headers -- */

#include <node_port/node_port_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <external-napi/node_api.h>

/* -- Methods -- */

/**
*  @brief
*    Convert a MetaCall value into a N-API value
*
*  @param[in] env
*    Environment context of N-API
*
*  @param[in] v
*    MetaCall value to be converted
*
*  @param[out] js_v
*    N-API value result after conversion from MetaCall
*/
NODE_PORT_API void metacall_node_value_to_napi(napi_env env, void * v, napi_value * js_v);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_NODE_PORT_H */
