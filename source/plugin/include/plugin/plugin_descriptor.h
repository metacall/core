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

#ifndef PLUGIN_DESCRIPTOR_H
#define PLUGIN_DESCRIPTOR_H 1

/* -- Headers -- */

#include <plugin/plugin_api.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Declarations  -- */

struct plugin_descriptor_type
{
	dynlink handle;
	char *library_name;
	char *symbol_iface_name;
	void *(*iface_singleton)(void);
};

/* -- Type Declarations  -- */

typedef struct plugin_descriptor_type *plugin_descriptor;

/* -- Methods -- */

PLUGIN_API plugin_descriptor plugin_descriptor_create(char *path, char *library_name, char *symbol_iface_name);

PLUGIN_API void plugin_descriptor_destroy(plugin_descriptor descriptor);

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_DESCRIPTOR_H */
