/*
 *	Plugin Library by Parra Studios
 *	A library for plugins at run-time into a process.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef PLUGIN_H
#define PLUGIN_H 1

/* -- Headers -- */

#include <plugin/plugin_api.h>

#include <plugin/plugin_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods  -- */

PLUGIN_API const char *plugin_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_H */
