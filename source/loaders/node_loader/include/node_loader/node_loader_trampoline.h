/*
 *	Loader Library by Parra Studios
 *	A plugin for loading nodejs code at run-time into a process.
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

#ifndef NODE_LOADER_TRAMPOLINE_H
#define NODE_LOADER_TRAMPOLINE_H 1

#include <node_loader/node_loader_api.h>

#include <node_api.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void * (*node_loader_trampoline_register_ptr)(void *, void *, void *);

NODE_LOADER_NO_EXPORT napi_value node_loader_trampoline_initialize(napi_env env, napi_value exports);

#ifdef __cplusplus
}
#endif

#endif /* NODE_LOADER_TRAMPOLINE_H */
