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

#include <plugin/plugin_impl.h>

#include <log/log.h>

/* -- Methods -- */

plugin plugin_create(const char *name, dynlink handle, void *iface, void *impl)
{
	if (name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin name");

		return NULL;
	}

	size_t name_length = strlen(name);

	if (name_length == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin name length");

		return NULL;
	}

	if (handle == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin handle");

		return NULL;
	}

	plugin p = malloc(sizeof(struct plugin_type));

	if (p == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin allocation");

		return NULL;
	}

	p->name = malloc(sizeof(char) * (name_length + 1));

	if (p->name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin name allocation");

		free(p);

		return NULL;
	}

	strncpy(p->name, name, name_length);

	p->handle = handle;
	p->iface = iface;
	p->impl = impl;

	return p;
}

char *plugin_name(plugin p)
{
	return p->name;
}

dynlink *plugin_handle(plugin p)
{
	return p->handle;
}

void *plugin_iface(plugin p)
{
	return p->iface;
}

void *plugin_impl(plugin p)
{
	return p->impl;
}

void plugin_destroy(plugin p)
{
	if (p != NULL)
	{
		if (p->name != NULL)
		{
			free(p->name);
		}

		free(p);
	}
}
