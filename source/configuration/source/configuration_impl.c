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

#include <serial/serial.h>

#include <string.h>

/* -- Forward Declarations -- */

struct configuration_impl_singleton_type;

/* -- Type Definitions -- */

typedef struct configuration_impl_singleton_type * configuration_impl_singleton;

/* -- Member Data -- */

struct configuration_impl_singleton_type
{
	serial s;
};

/* -- Private Methods -- */

static configuration_impl_singleton configuration_impl_singleton_instance(void);

/* -- Methods -- */

configuration_impl_singleton configuration_impl_singleton_instance()
{
	static struct configuration_impl_singleton_type instance =
	{
		NULL,
	};

	return &instance;
}

const char * configuration_impl_extension()
{
	configuration_impl_singleton singleton = configuration_impl_singleton_instance();

	return serial_extension(singleton->s);
}

int configuration_impl_initialize(const char * name)
{
	configuration_impl_singleton singleton = configuration_impl_singleton_instance();

	singleton->s = serial_create(name);

	return !(singleton->s != NULL);
}

int configuration_impl_load(configuration config)
{
	configuration_impl_singleton singleton = configuration_impl_singleton_instance();

	set storage = set_create(&hash_callback_str, &comparable_callback_str);

	vector queue, childs;

	if (storage == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load set allocation");

		return 1;
	}

	queue = vector_create(sizeof(configuration));

	if (queue == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load queue allocation");

		set_destroy(storage);

		return 1;
	}

	childs = vector_create(sizeof(configuration));

	if (queue == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load childs allocation");

		set_destroy(storage);

		vector_destroy(queue);

		return 1;
	}

	vector_push_back(queue, &config);

	while (vector_size(queue) > 0)
	{
		configuration current = *((configuration *)vector_front(queue));

		const char * source = configuration_object_source(current);

		value v;

		vector_pop_front(queue);

		v = serial_deserialize(singleton->s, source, strlen(source) + 1);

		if (v == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load (childs) <%p>", current);

			set_destroy(storage);

			vector_destroy(queue);

			vector_destroy(childs);

			return 1;
		}

		configuration_object_instantiate(current, v);

		vector_clear(childs);

		if (configuration_object_childs(current, childs, storage) == 0 && vector_size(childs) > 0)
		{
			size_t iterator;

			for (iterator = 0; iterator < vector_size(childs); ++iterator)
			{
				configuration child = *((configuration *)vector_at(childs, iterator));

				if (set_get(storage, (set_key)configuration_object_name(child)) == NULL)
				{
					if (configuration_singleton_register(child) != 0)
					{
						log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation child singleton insertion (%s, %s)",
							configuration_object_name(child), configuration_object_path(child));

						set_destroy(storage);

						vector_destroy(queue);

						vector_destroy(childs);

						return 1;
					}

					vector_push_back(queue, &child);
				}

				if (set_insert(storage, (set_key)configuration_object_name(child), child) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation child set insertion");

					set_destroy(storage);

					vector_destroy(queue);

					vector_destroy(childs);

					return 1;
				}
			}
		}
	}

	set_destroy(storage);

	vector_destroy(queue);

	vector_destroy(childs);

	return 0;
}

int configuration_impl_destroy()
{
	configuration_impl_singleton singleton = configuration_impl_singleton_instance();

	return serial_clear(singleton->s);
}
