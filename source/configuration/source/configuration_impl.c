/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <configuration/configuration_impl.h>
#include <configuration/configuration_singleton.h>

#include <adt/adt_set.h>
#include <adt/adt_vector.h>

#include <log/log.h>

#include <serial/serial.h>

#include <string.h>

/* -- Forward Declarations -- */

struct configuration_impl_singleton_type;

/* -- Type Definitions -- */

typedef struct configuration_impl_singleton_type *configuration_impl_singleton;

/* -- Member Data -- */

struct configuration_impl_singleton_type
{
	serial s;
};

/* -- Private Methods -- */

static configuration_impl_singleton configuration_impl_singleton_instance(void);

/* -- Methods -- */

configuration_impl_singleton configuration_impl_singleton_instance(void)
{
	static struct configuration_impl_singleton_type instance = {
		NULL,
	};

	return &instance;
}

const char *configuration_impl_extension(void)
{
	configuration_impl_singleton singleton = configuration_impl_singleton_instance();

	return serial_extension(singleton->s);
}

int configuration_impl_initialize(const char *name)
{
	configuration_impl_singleton singleton = configuration_impl_singleton_instance();

	singleton->s = serial_create(name);

	return !(singleton->s != NULL);
}

int configuration_impl_load(configuration config, void *allocator)
{
	configuration_impl_singleton singleton = configuration_impl_singleton_instance();
	vector queue, childs;
	set storage = set_create(&hash_callback_str, &comparable_callback_str);
	int result = 1;

	if (storage == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load set allocation");

		goto alloc_storage_error;
	}

	queue = vector_create(sizeof(configuration));

	if (queue == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load queue allocation");

		goto alloc_queue_error;
	}

	childs = vector_create(sizeof(configuration));

	if (queue == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load childs allocation");

		goto alloc_childs_error;
	}

	vector_push_back(queue, &config);

	while (vector_size(queue) > 0)
	{
		configuration current = *((configuration *)vector_front(queue));

		const char *source = configuration_object_source(current);

		value v;

		vector_pop_front(queue);

		if (source == NULL)
		{
			v = value_create_map(NULL, 0);
		}
		else
		{
			v = serial_deserialize(singleton->s, source, strlen(source) + 1, (memory_allocator)allocator);

			if (v == NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load (childs) <%p>", current);

				goto load_error;
			}
		}

		configuration_object_instantiate(current, v);

		vector_clear(childs);

		if (configuration_object_childs(current, childs, storage) != 0)
		{
			goto load_error;
		}

		if (vector_size(childs) > 0)
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

						goto load_error;
					}

					vector_push_back(queue, &child);
				}

				if (set_insert(storage, (set_key)configuration_object_name(child), child) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation child set insertion");

					goto load_error;
				}
			}
		}
	}

	result = 0;

load_error:
	vector_destroy(childs);
alloc_childs_error:
	vector_destroy(queue);
alloc_queue_error:
	set_destroy(storage);
alloc_storage_error:
	return result;
}

int configuration_impl_destroy(void)
{
	configuration_impl_singleton singleton = configuration_impl_singleton_instance();

	return serial_clear(singleton->s);
}
