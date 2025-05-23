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

#include <plugin/plugin_impl.h>

#include <log/log.h>

#include <string.h>

/* -- Declarations -- */

struct plugin_type
{
	char *name;
	plugin_descriptor descriptor;
	void *iface;
	void *impl;
	void (*dtor)(plugin);
};

/* -- Methods -- */

plugin plugin_create(const char *name, plugin_descriptor descriptor, void *iface, void *impl, void (*dtor)(plugin))
{
	if (name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin name");
		return NULL;
	}

	size_t name_size = strlen(name) + 1;

	if (name_size <= 1)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin name length");
		return NULL;
	}

	plugin p = malloc(sizeof(struct plugin_type));

	if (p == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin allocation");
		return NULL;
	}

	p->descriptor = descriptor;
	p->iface = iface;
	p->impl = impl;
	p->dtor = dtor;
	p->name = malloc(sizeof(char) * name_size);

	if (p->name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin name allocation");
		plugin_destroy(p);
		return NULL;
	}

	memcpy(p->name, name, name_size);

	return p;
}

char *plugin_name(plugin p)
{
	return p->name;
}

plugin_descriptor plugin_desc(plugin p)
{
	return p->descriptor;
}

void *plugin_iface(plugin p)
{
	return p->iface;
}

void *plugin_impl(plugin p)
{
	return p->impl;
}

void plugin_destructor(plugin p)
{
	if (p != NULL)
	{
		if (p->dtor != NULL)
		{
			p->dtor(p);
			p->dtor = NULL;
		}
	}
}

void plugin_destroyed(plugin p)
{
	if (p != NULL)
	{
		p->dtor = NULL;
	}
}

void plugin_destroy(plugin p)
{
	if (p != NULL)
	{
		if (p->dtor != NULL)
		{
			p->dtor(p);
		}

		plugin_descriptor_destroy(p->descriptor);

		if (p->name != NULL)
		{
			free(p->name);
		}

		free(p);
	}
}
