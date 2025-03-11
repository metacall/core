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

/* -- Headers -- */

#include <plugin/plugin_descriptor.h>

#include <log/log.h>

/* -- Methods -- */

plugin_descriptor plugin_descriptor_create(char *path, char *library_name, char *symbol_iface_name)
{
	plugin_descriptor descriptor = malloc(sizeof(struct plugin_descriptor_type));

	if (descriptor == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin descriptor allocation");
		return NULL;
	}

	descriptor->handle = NULL;
	descriptor->library_name = library_name;
	descriptor->symbol_iface_name = symbol_iface_name;
	descriptor->iface_singleton = NULL;

	log_write("metacall", LOG_LEVEL_DEBUG, "Loading plugin: %s", library_name);

	dynlink handle = dynlink_load(path, library_name, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to load library from plugin descriptor");
		plugin_descriptor_destroy(descriptor);
		return NULL;
	}

	descriptor->handle = handle;

	log_write("metacall", LOG_LEVEL_DEBUG, "Loading plugin symbol: %s", symbol_iface_name);

	dynlink_symbol_addr address;

	if (dynlink_symbol(handle, symbol_iface_name, &address) != 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin descriptor dynlink symbol loading");
		plugin_descriptor_destroy(descriptor);
		return NULL;
	}

	descriptor->iface_singleton = (void *(*)(void))(address);

	if (descriptor->iface_singleton == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin descriptor dynlink symbol access");
		plugin_descriptor_destroy(descriptor);
		return NULL;
	}

	return descriptor;
}

void plugin_descriptor_destroy(plugin_descriptor descriptor)
{
	if (descriptor != NULL)
	{
		dynlink_unload(descriptor->handle);

		if (descriptor->library_name != NULL)
		{
			free(descriptor->library_name);
		}

		if (descriptor->symbol_iface_name != NULL)
		{
			free(descriptor->symbol_iface_name);
		}

		free(descriptor);
	}
}
