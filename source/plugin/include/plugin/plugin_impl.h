/*
 *	Plugin Library by Parra Studios
 *	A library for plugins at run-time into a process.
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

#ifndef PLUGIN_IMPL_H
#define PLUGIN_IMPL_H 1

/* -- Headers -- */

#include <plugin/plugin_api.h>

#include <plugin/plugin_descriptor.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */

#define plugin_iface_type(p, type_name) \
	((type_name)plugin_iface(p))

#define plugin_impl_type(p, type_name) \
	((type_name)plugin_impl(p))

/* -- Forward Declarations  -- */

struct plugin_type;

/* -- Type Declarations  -- */

typedef struct plugin_type *plugin;

/* -- Methods  -- */

PLUGIN_API plugin plugin_create(const char *name, plugin_descriptor descriptor, void *iface, void *impl, void (*dtor)(plugin));

PLUGIN_API char *plugin_name(plugin p);

PLUGIN_API plugin_descriptor plugin_desc(plugin p);

PLUGIN_API void *plugin_iface(plugin p);

PLUGIN_API void *plugin_impl(plugin p);

/**
*  @brief
*    Executes the destructor once and nullifies it, it does not free
*    the memory of the plugin but the destructor is removed
*
*  @param[in] p
*    The plugin that will be used to execute the destructor
*/
PLUGIN_API void plugin_destructor(plugin p);

/**
*  @brief
*    Marks the plugin as destroyed, nullifies the destructor
*    but does not free the memory of the plugin, this is useful
*    for when we execute loader plugin manager in host mode,
*    because the destructor is called without the control of metacall,
*    and it prevents to call it again after it has been called by host
*
*  @param[in] p
*    The plugin that will be used to clear the destructor
*/
PLUGIN_API void plugin_destroyed(plugin p);

/**
*  @brief
*    Executes the destructor if any and frees all the memory
*
*  @param[in] p
*    The plugin to be destroyed
*/
PLUGIN_API void plugin_destroy(plugin p);

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_IMPL_H */
