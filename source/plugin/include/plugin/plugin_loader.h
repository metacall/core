/*
 *	Plugin Library by Parra Studios
 *	A library for plugins at run-time into a process.
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

#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H 1

/* -- Headers -- */

#include <plugin/plugin_api.h>

#include <plugin/plugin_impl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations  -- */

struct plugin_loader_type;

/* -- Type Declarations  -- */

typedef struct plugin_loader_type *plugin_loader;

/* -- Methods  -- */

PLUGIN_API plugin_loader plugin_loader_create(const char *name);

PLUGIN_API plugin plugin_loader_load(plugin_loader l, char *name, void *impl, void (*dtor)(plugin));

PLUGIN_API void plugin_loader_unload(plugin_loader l, plugin p);

PLUGIN_API void plugin_loader_destroy(plugin_loader l);

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_LOADER_H */
