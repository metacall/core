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

/* -- Headers -- */

#include <plugin/plugin_loader.h>

/* -- Declarations -- */

struct plugin_loader_type
{
	char *library_suffix;
	char *symbol_interface_suffix;
};

/* -- Methods -- */

plugin_loader plugin_loader_create(const char *name)
{
	plugin_loader l = malloc(sizeof(struct plugin_loader_type));

	if (l == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin loader allocation");
		return NULL;
	}

	// TODO: Generate the suffixes

	return l;
}

plugin plugin_loader_load(plugin_loader l, char *name, void *impl, void (*dtor)(plugin))
{
	// TODO: plugin_create(...)
	// TODO: Implement dynlink load
}

void plugin_loader_unload(plugin_loader l, plugin p)
{
	// TODO: Implement dynlink unload
	// TODO: plugin_destroy(...)
}

void plugin_loader_destroy(plugin_loader l)
{
	if (l != NULL)
	{
		if (l->library_suffix != NULL)
		{
			free(l->library_suffix);
		}

		if (l->symbol_interface_suffix != NULL)
		{
			free(l->symbol_interface_suffix);
		}

		free(l);
	}
}
