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

/* -- Headers -- */

#include <plugin/plugin_loader.h>

#include <plugin/plugin_descriptor.h>
#include <plugin/plugin_manager.h>

#include <log/log.h>

#include <string.h>

/* -- Declarations -- */

struct plugin_loader_type
{
	plugin_manager manager;
	char *library_suffix;
	char *symbol_iface_suffix;
};

/* -- Type Definitions -- */

typedef void *(*plugin_interface_singleton)(void);

/* -- Private Methods -- */

static int plugin_loader_library_suffix(plugin_loader l, const char *name, size_t name_length);
static int plugin_loader_symbol_iface_suffix(plugin_loader l, const char *name, size_t name_length);
static char *plugin_loader_generate_library_name(const char *name, char *suffix);
static char *plugin_loader_generate_symbol_iface_name(const char *name, char *suffix);

/* -- Methods -- */

int plugin_loader_library_suffix(plugin_loader l, const char *name, size_t name_length)
{
	/* This generates a suffix string for the library name */
	/* They are in the form of: <plugin_name>_<manager_name>[d] */
	/* For example, in Linux with Debug: rapid_json_seriald */

#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	static const size_t extra_length = 3; /* 3: for the _ at the begining and d\0 at the end */
#else
	static const size_t extra_length = 2; /* 3: for the _ at the begining and \0 at the end */

#endif

	char *result = malloc(sizeof(char) * (name_length + extra_length));

	if (result == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin loader library suffix allocation");
		return 1;
	}

	result[0] = '_';

	memcpy(&result[1], name, name_length);

#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	result[1 + name_length] = 'd';
	++name_length;
#endif

	result[1 + name_length] = '\0';

	l->library_suffix = result;

	return 0;
}

int plugin_loader_symbol_iface_suffix(plugin_loader l, const char *name, size_t name_length)
{
	/* This generates a suffix string for accessing the entry point of a plugin (the interface symbol) */
	/* They are in the form of: [dynlink_symbol_]<plugin_name>_<manager_name>_impl_interface_singleton */
	/* For example, in Linux: dynlink_symbol_rapid_json_serial_impl_interface_singleton */

	static const char suffix[] = "_impl_interface_singleton";
	size_t suffix_length = sizeof(suffix) - 1;
	char *result = malloc(sizeof(char) * (name_length + suffix_length + 2)); /* 2: for the _ at the begining and \0 at the end */

	if (result == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin loader symbol suffix allocation");
		return 1;
	}

	result[0] = '_';

	memcpy(&result[1], name, name_length);

	memcpy(&result[1 + name_length], suffix, suffix_length);

	result[1 + name_length + suffix_length] = '\0';

	l->symbol_iface_suffix = result;

	return 0;
}

plugin_loader plugin_loader_create(plugin_manager manager)
{
	if (manager == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin loader manager");
		return NULL;
	}

	plugin_loader l = malloc(sizeof(struct plugin_loader_type));

	if (l == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin loader allocation");
		return NULL;
	}

	l->manager = manager;
	l->library_suffix = NULL;
	l->symbol_iface_suffix = NULL;

	/* Generate the suffixes */
	size_t name_length = strlen(manager->name);

	if (plugin_loader_library_suffix(l, manager->name, name_length) != 0)
	{
		plugin_loader_destroy(l);
		return NULL;
	}

	if (plugin_loader_symbol_iface_suffix(l, manager->name, name_length) != 0)
	{
		plugin_loader_destroy(l);
		return NULL;
	}

	return l;
}

char *plugin_loader_generate_library_name(const char *name, char *suffix)
{
	size_t name_length = strlen(name);
	size_t suffix_length = strlen(suffix);
	char *library_name = malloc(sizeof(char) * (name_length + suffix_length + 1));

	if (library_name == NULL)
	{
		return NULL;
	}

	memcpy(library_name, name, name_length);

	memcpy(&library_name[name_length], suffix, suffix_length);

	library_name[name_length + suffix_length] = '\0';

	return library_name;
}

char *plugin_loader_generate_symbol_iface_name(const char *name, char *suffix)
{
	size_t name_length = strlen(name);
	size_t mangle_length = dynlink_symbol_name_mangle(name, name_length, NULL);
	size_t suffix_length = strlen(suffix);
	char *symbol_iface_name = malloc(sizeof(char) * (mangle_length + suffix_length + 1));

	if (symbol_iface_name == NULL)
	{
		return NULL;
	}

	dynlink_symbol_name_mangle(name, name_length, symbol_iface_name);

	memcpy(&symbol_iface_name[mangle_length], suffix, suffix_length);

	symbol_iface_name[mangle_length + suffix_length] = '\0';

	return symbol_iface_name;
}

plugin plugin_loader_load(plugin_loader l, const char *name, void *impl, void (*dtor)(plugin))
{
	char *library_name = plugin_loader_generate_library_name(name, l->library_suffix);

	if (library_name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Plugin loader from manager '%s' failed to allocate the library name for plugin: %s", l->manager->name, name);
		return NULL;
	}

	char *symbol_iface_name = plugin_loader_generate_symbol_iface_name(name, l->symbol_iface_suffix);

	if (symbol_iface_name == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Plugin loader from manager '%s' failed to allocate the symbol interface name for plugin: %s", l->manager->name, name);
		free(library_name);
		return NULL;
	}

	plugin_descriptor descriptor = plugin_descriptor_create(l->manager->library_path, library_name, symbol_iface_name);

	if (descriptor == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Plugin loader from manager '%s' failed to load plugin: %s", l->manager->name, name);
		return NULL;
	}

	return plugin_create(name, descriptor, descriptor->iface_singleton(), impl, dtor);
}

void plugin_loader_destroy(plugin_loader l)
{
	if (l != NULL)
	{
		if (l->library_suffix != NULL)
		{
			free(l->library_suffix);
		}

		if (l->symbol_iface_suffix != NULL)
		{
			free(l->symbol_iface_suffix);
		}

		free(l);
	}
}
