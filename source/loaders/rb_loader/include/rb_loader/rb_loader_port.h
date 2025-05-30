/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef RB_LOADER_PORT_H
#define RB_LOADER_PORT_H 1

#include <rb_loader/rb_loader_api.h>

#include <loader/loader_impl_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

RB_LOADER_NO_EXPORT int rb_loader_port_initialize(loader_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* RB_LOADER_PORT_H */
