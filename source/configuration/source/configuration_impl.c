/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <configuration/configuration_impl.h>
#include <configuration/configuration_singleton.h>

#include <adt/adt_vector.h>
#include <adt/adt_set.h>

#include <log/log.h>

/* -- Private Methods -- */

static int configuration_impl_load_impl(configuration config);

/* -- Methods -- */

const char * configuration_impl_extension()
{
	configuration_interface config_iface = configuration_interface_instance();

	return config_iface->extension();
}

int configuration_impl_initialize(configuration_interface iface)
{
	configuration_interface config_iface = configuration_interface_instance();

	config_iface->extension = iface->extension;
	config_iface->initialize = iface->initialize;
	config_iface->load = iface->load;
	config_iface->unload = iface->unload;
	config_iface->destroy = iface->destroy;

	return config_iface->initialize();
}

int configuration_impl_load_impl(configuration config)
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

int configuration_impl_load(configuration config)
{
	set s = set_create(&hash_callback_str, &comparable_callback_str);

	vector queue, childs;

	if (s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load set allocation");

		return 1;
	}

	queue = vector_create(sizeof(configuration));

	if (queue == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load queue allocation");

		set_destroy(s);

		return 1;
	}

	childs = vector_create(sizeof(configuration));

	if (queue == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load childs allocation");

		set_destroy(s);

		vector_destroy(queue);

		return 1;
	}

	vector_push_back(queue, &config);

	while (vector_size(queue) > 0)
	{
		configuration current = *((configuration *)vector_front(queue));

		vector_pop_front(queue);

		if (configuration_impl_load_impl(current) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load (childs) <%p>", current);

			set_destroy(s);

			vector_destroy(queue);

			vector_destroy(childs);

			return 1;
		}

		vector_clear(childs);

		if (configuration_object_childs(current, childs) == 0 && vector_size(childs) > 0)
		{
			size_t iterator;

			for (iterator = 0; iterator < vector_size(childs); ++iterator)
			{
				configuration child = *((configuration *)vector_at(childs, iterator));

				if (set_get(s, (set_key)configuration_object_name(child)) == NULL)
				{
					if (configuration_singleton_register(child) != 0)
					{
						log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation child singleton insertion (%s, %s)",
							configuration_object_name(child), configuration_object_path(child));

						set_destroy(s);

						vector_destroy(queue);

						vector_destroy(childs);

						return 1;
					}

					vector_push_back(queue, &child);
				}

				if (set_insert(s, (set_key)configuration_object_name(child), &child) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation child set insertion");

					set_destroy(s);

					vector_destroy(queue);

					vector_destroy(childs);

					return 1;
				}
			}
		}
	}

	set_destroy(s);

	vector_destroy(queue);

	vector_destroy(childs);

	return 0;
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

	config_iface->extension = NULL;
	config_iface->initialize = NULL;
	config_iface->load = NULL;
	config_iface->unload = NULL;
	config_iface->destroy = NULL;

	return result;
}
