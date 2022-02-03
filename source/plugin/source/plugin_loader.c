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
	char *symbol_iface_suffix;
};

/* -- Private Methods -- */

static char *plugin_loader_library_suffix(plugin_loader l, const char *name, size_t name_length);
static char *plugin_loader_symbol_iface_suffix(plugin_loader l, const char *name, size_t name_length);

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

	char *result = malloc(sizeof(char) * (name_length + suffix_length + extra_length));

	if (result == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin loader library suffix allocation");
		return 1;
	}

	result[0] == '_';

	strncpy(&result[1], name, name_length);

#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	result[1 + name_length] = 'd';
	++name_length;
#endif

	result[1 + name_length] = '\0';

	l->library_suffix = result;
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

	result[0] == '_';

	strncpy(&result[1], name, name_length);

	strncpy(&result[1 + name_length], suffix, suffix_length);

	result[1 + name_length + suffix_length] = '\0';

	l->symbol_iface_suffix = result;

	return 0;
}

plugin_loader plugin_loader_create(const char *name)
{
	if (name == NULL)
	{
		return NULL;
	}

	plugin_loader l = malloc(sizeof(struct plugin_loader_type));

	if (l == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin loader allocation");
		return NULL;
	}

	l->library_suffix = NULL;
	l->symbol_iface_suffix = NULL;

	/* Generate the suffixes */
	size_t name_length = strlen(name);

	if (plugin_loader_library_suffix(l, name, name_length) != 0)
	{
		plugin_loader_destroy(l);
		return NULL;
	}

	if (plugin_loader_symbol_iface_suffix(l, name, name_length) != 0)
	{
		plugin_loader_destroy(l);
		return NULL;
	}

	return l;
}

plugin plugin_loader_load(plugin_loader l, char *name, void *impl, void (*dtor)(plugin))
{
	// TODO: plugin_create(...)
	// TODO: Generate library name from suffix
	// TODO: Implement dynlink load
	// TODO: dynlink_symbol_name_mangle with suffix
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
