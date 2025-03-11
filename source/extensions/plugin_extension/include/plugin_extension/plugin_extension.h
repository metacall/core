/*
 *	Extension Library by Parra Studios
 *	An extension for loading a folder of plugins based on metacall.json files.
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

#ifndef PLUGIN_EXTENSION_H
#define PLUGIN_EXTENSION_H 1

#include <plugin_extension/plugin_extension_api.h>

#ifdef __cplusplus
extern "C" {
#endif

PLUGIN_EXTENSION_API int plugin_extension(void *loader, void *handle);

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_EXTENSION_H */
