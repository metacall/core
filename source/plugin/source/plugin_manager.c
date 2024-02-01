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

#include <plugin/plugin_manager.h>

#include <plugin/plugin_loader.h>

#include <environment/environment_variable_path.h>

#include <log/log.h>

#include <string.h>

#if defined(WIN32) || defined(_WIN32)
	#include <winbase.h>
#endif

/* -- Declarations -- */

struct plugin_manager_iterate_cb_type
{
	plugin_manager manager;
	int (*iterator)(plugin_manager, plugin, void *);
	void *data;
};

/* -- Private Methods -- */

static int plugin_manager_unregister(plugin_manager manager, plugin p);
static int plugin_manager_iterate_cb(set s, set_key key, set_value val, set_cb_iterate_args args);
static int plugin_manager_destroy_cb(set s, set_key key, set_value val, set_cb_iterate_args args);

/* -- Methods -- */

int plugin_manager_initialize(plugin_manager manager, const char *name, const char *environment_library_path, const char *default_library_path, plugin_manager_interface iface, void *impl)
{
	/* Initialize the name */
	if (manager->name == NULL)
	{
		if (name == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin manager name");
			return 1;
		}

		size_t name_size = strlen(name) + 1;

		if (name_size <= 1)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin manager name length");

			return 1;
		}

		manager->name = malloc(sizeof(char) * name_size);

		if (manager->name == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin manager name allocation");

			return 1;
		}

		memcpy(manager->name, name, name_size);
	}

	/* Copy manager interface and implementation */
	manager->iface = iface;
	manager->impl = impl;

	/* Allocate the set which maps the plugins by their name */
	if (manager->plugins == NULL)
	{
		manager->plugins = set_create(&hash_callback_str, &comparable_callback_str);

		if (manager->plugins == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin manager set initialization");

			plugin_manager_destroy(manager);

			return 1;
		}
	}

	/* Initialize the library path */
	if (manager->library_path == NULL)
	{
		const char name[] = "metacall"
#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
							"d"
#endif
			;

		dynlink_library_path_str path;
		size_t length = 0;

		/* The order of precedence is:
		* 1) Environment variable
		* 2) Dynamic link library path of the host library
		* 3) Default compile time path
		*/
		if (dynlink_library_path(name, path, &length) == 0)
		{
			default_library_path = path;
		}
		else
		{
			length = strlen(default_library_path);
		}

		manager->library_path = environment_variable_path_create(environment_library_path, default_library_path, length + 1, NULL);

		if (manager->library_path == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin manager library path initialization");

			plugin_manager_destroy(manager);

			return 1;
		}
	}

	/* On Windows, pass the library path to the loader so it can find the dependencies of the plugins */
	/* For more information: https://github.com/metacall/core/issues/479 */
#if defined(WIN32) || defined(_WIN32)
	if (SetDllDirectoryA(manager->library_path) == FALSE)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to register the DLL directory %s; plugins with other dependant DLLs may fail to load", manager->library_path);
	}
#endif

	/* Initialize the plugin loader */
	if (manager->l == NULL)
	{
		manager->l = plugin_loader_create(manager);

		if (manager->l == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin manager loader initialization");

			plugin_manager_destroy(manager);

			return 1;
		}
	}

	return 0;
}

const char *plugin_manager_name(plugin_manager manager)
{
	return manager->name;
}

char *plugin_manager_library_path(plugin_manager manager)
{
	return manager->library_path;
}

void *plugin_manager_impl(plugin_manager manager)
{
	return manager->impl;
}

size_t plugin_manager_size(plugin_manager manager)
{
	return set_size(manager->plugins);
}

int plugin_manager_register(plugin_manager manager, plugin p)
{
	const char *name = plugin_name(p);

	if (set_get(manager->plugins, (set_key)name) != NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to register plugin %s into manager %s, it already exists", name, manager->name);

		return 1;
	}

	return set_insert(manager->plugins, (set_key)name, p);
}

plugin plugin_manager_create(plugin_manager manager, const char *name, void *impl, void (*dtor)(plugin))
{
	/* Check if plugin is already loaded and return it */
	plugin p = plugin_manager_get(manager, name);

	if (p != NULL)
	{
		return p;
	}

	/* Load the plugin (dynamic library) and initialize the interface */
	p = plugin_loader_load(manager->l, name, impl, dtor);

	if (p == NULL)
	{
		return NULL;
	}

	/* Register plugin into the plugin manager set */
	if (plugin_manager_register(manager, p) != 0)
	{
		plugin_destroy(p);
		return NULL;
	}

	return p;
}

plugin plugin_manager_get(plugin_manager manager, const char *name)
{
	return set_get(manager->plugins, (set_key)name);
}

int plugin_manager_iterate_cb(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
	(void)key;

	if (val != NULL && args != NULL)
	{
		struct plugin_manager_iterate_cb_type *args_ptr = (struct plugin_manager_iterate_cb_type *)args;
		return args_ptr->iterator(args_ptr->manager, (plugin)val, args_ptr->data);
	}

	return 0;
}

void plugin_manager_iterate(plugin_manager manager, int (*iterator)(plugin_manager, plugin, void *), void *data)
{
	if (iterator == NULL)
	{
		return;
	}

	struct plugin_manager_iterate_cb_type args = {
		manager,
		iterator,
		data
	};

	set_iterate(manager->plugins, &plugin_manager_iterate_cb, (void *)&args);
}

int plugin_manager_unregister(plugin_manager manager, plugin p)
{
	const char *name = plugin_name(p);

	if (set_get(manager->plugins, (set_key)name) == NULL)
	{
		return 0;
	}

	if (set_remove(manager->plugins, (const set_key)name) == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Failed to unregister plugin %s from manager %s", name, manager->name);

		return 1;
	}

	return 0;
}

int plugin_manager_clear(plugin_manager manager, plugin p)
{
	if (p == NULL)
	{
		return 1;
	}

	/* Remove the plugin from the plugins set */
	int result = plugin_manager_unregister(manager, p);

	/* Unload the dynamic link library and destroy the plugin */
	plugin_destroy(p);

	return result;
}

int plugin_manager_destroy_cb(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	int result = 0;

	(void)s;
	(void)key;

	if (val != NULL)
	{
		plugin p = (plugin)val;

		if (args != NULL)
		{
			plugin_manager manager = (plugin_manager)args;

			if (manager->iface != NULL && manager->iface->clear != NULL)
			{
				/* Call to the clear method of the manager */
				result = manager->iface->clear(manager, p);
			}
		}

		/* Unload the dynamic link library and destroy the plugin */
		plugin_destroy(p);
	}

	return result;
}

void plugin_manager_destroy(plugin_manager manager)
{
	/* If there's a destroy callback, probably the plugin manager needs a complex destroy algorithm */
	if (manager->iface != NULL && manager->iface->destroy != NULL)
	{
		manager->iface->destroy(manager, manager->impl);
	}

	/* Unload and destroy each plugin. The destroy callback is executed before this so the user can clear the
	* plugin set and this will do nothing if the set has been emptied before with plugin_manager_clear */
	if (manager->plugins != NULL)
	{
		set_iterate(manager->plugins, &plugin_manager_destroy_cb, NULL);
	}

	/* Clear the name */
	if (manager->name != NULL)
	{
		free(manager->name);
		manager->name = NULL;
	}

	/* Destroy the plugin set */
	if (manager->plugins != NULL)
	{
		set_destroy(manager->plugins);
		manager->plugins = NULL;
	}

	/* Clear the library path */
	if (manager->library_path != NULL)
	{
		environment_variable_path_destroy(manager->library_path);
		manager->library_path = NULL;
	}

	/* Clear the loader */
	if (manager->l != NULL)
	{
		plugin_loader_destroy(manager->l);
		manager->l = NULL;
	}

	/* Nullify the rest of parameters that do not need deallocation */
	manager->iface = NULL;
	manager->impl = NULL;
}
