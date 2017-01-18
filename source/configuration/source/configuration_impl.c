/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <configuration/configuration_impl.h>

#include <log/log.h>

/* -- Methods -- */

int configuration_impl_initialize(configuration_interface iface)
{
	configuration_interface config_iface = configuration_interface_instance();

	config_iface->initialize = iface->initialize;
	config_iface->load = iface->load;
	config_iface->get = iface->get;
	config_iface->unload = iface->unload;
	config_iface->destroy = iface->destroy;

	return config_iface->initialize();
}

int configuration_impl_load(configuration config)
{
	configuration_interface config_iface = configuration_interface_instance();

	configuration_impl impl = config_iface->load(config);

	if (impl == NULL)
	{
		return 1;
	}

	configuration_object_instantiate(config, impl);

	return 0;
}

value configuration_impl_value(configuration config, const char * key, type_id id)
{
	configuration_interface config_iface = configuration_interface_instance();

	return config_iface->get(config, key, id);
}

int configuration_impl_unload(configuration config)
{
	configuration_interface config_iface = configuration_interface_instance();

	return config_iface->unload(config);
}

int configuration_impl_destroy()
{
	configuration_interface config_iface = configuration_interface_instance();

	int result = config_iface->destroy();

	config_iface->initialize = NULL;
	config_iface->load = NULL;
	config_iface->get = NULL;
	config_iface->unload = NULL;
	config_iface->destroy = NULL;

	return result;
}
