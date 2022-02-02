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

#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H 1

/* -- Headers -- */

#include <plugin/plugin_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */

#define plugin_manager_impl_type(manager, type_name) \
	((type_name *)plugin_manager_impl(manager))

/* -- Forward Declarations -- */

struct plugin_type;
struct plugin_manager_type;
struct plugin_manager_interface_type;

/* -- Type Definitions -- */

typedef struct plugin_type *plugin;
typedef struct plugin_manager_type *plugin_manager;
typedef struct plugin_manager_interface_type *plugin_manager_interface;

/* -- Declarations -- */

struct plugin_manager_type
{
	const char *name;				/* Defines the plugin manager name (a pointer to a static string defining the manager type) */
	char *library_path;				/* Defines current library path */
	set plugins;					/* Contains the plugins indexed by name */
	plugin_manager_interface iface; /* Hooks into the plugin manager from the implementation */
	void *impl;						/* User defined plugin manager data */
};

struct plugin_manager_interface_type
{
	int (*clear)(plugin_manager, plugin); /* Hook for clearing the plugin implementation */
	void (*destroy)(plugin_manager);	  /* Hook for destroying the plugin manager implementation */
};

/* -- Methods  -- */

PLUGIN_API int plugin_manager_initialize(plugin_manager manager, const char *environment_library_path, const char *default_library_path, plugin_manager_interface iface, void *impl);

PLUGIN_API const char *plugin_manager_name(plugin_manager manager);

PLUGIN_API char *plugin_manager_library_path(plugin_manager manager);

PLUGIN_API void *plugin_manager_impl(plugin_manager manager);

PLUGIN_API int plugin_manager_register(plugin_manager manager, plugin p);

PLUGIN_API plugin plugin_manager_get(plugin_manager manager, const char *name);

PLUGIN_API void plugin_manager_iterate(plugin_manager manager, int (*iterator)(plugin_manager, plugin, void *), void *data);

PLUGIN_API int plugin_manager_clear(plugin_manager manager, plugin p);

PLUGIN_API void plugin_manager_destroy(plugin_manager manager);

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_MANAGER_H */
