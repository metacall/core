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

#include <dynlink/dynlink.h>

#include <log/log.h>

#include <string.h>

/* -- Definitions -- */

#define CONFIG_DYNLINK_NAME_SIZE 0x40

/* -- Forward Declarations -- */

struct configuration_impl_singleton_type;

/* -- Type Definitions -- */

typedef struct configuration_impl_singleton_type * configuration_impl_singleton;

/* -- Member Data -- */

struct configuration_impl_singleton_type
{
	configuration_interface iface;
	dynlink handle;
};

/* -- Private Methods -- */

static configuration_impl_singleton configuration_impl_singleton_instance(void);

static int configuration_impl_load_impl(configuration config);

static dynlink configuration_impl_initialize_load(const char * name);

static int configuration_impl_initialize_symbol(dynlink handle, const char * name, dynlink_symbol_addr * singleton_addr_ptr);

/* -- Methods -- */

configuration_impl_singleton configuration_impl_singleton_instance()
{
	static struct configuration_impl_singleton_type instance =
	{
		NULL,
		NULL
	};

	return &instance;
}

const char * configuration_impl_extension()
{
	configuration_impl_singleton singleton = configuration_impl_singleton_instance();

	return singleton->iface->extension();
}

dynlink configuration_impl_initialize_load(const char * name)
{
	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		const char config_dynlink_suffix[] = "_configd";
	#else
		const char config_dynlink_suffix[] = "_config";
	#endif

	#define CONFIG_DYNLINK_NAME_FULL_SIZE \
		(sizeof(config_dynlink_suffix) + CONFIG_DYNLINK_NAME_SIZE)

	char config_dynlink_name[CONFIG_DYNLINK_NAME_FULL_SIZE];

	strncpy(config_dynlink_name, name, CONFIG_DYNLINK_NAME_FULL_SIZE - 1);

	strncat(config_dynlink_name, config_dynlink_suffix,
		CONFIG_DYNLINK_NAME_FULL_SIZE - strnlen(config_dynlink_name, CONFIG_DYNLINK_NAME_FULL_SIZE - 1) - 1);

	#undef CONFIG_DYNLINK_NAME_FULL_SIZE

	log_write("metacall", LOG_LEVEL_DEBUG, "Loading config: %s", config_dynlink_name);

	return dynlink_load(NULL, config_dynlink_name, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
}

int configuration_impl_initialize_symbol(dynlink handle, const char * name, dynlink_symbol_addr * singleton_addr_ptr)
{
	const char config_dynlink_symbol_prefix[] = DYNLINK_SYMBOL_STR("");
	const char config_dynlink_symbol_suffix[] = "_config_impl_interface_singleton";

	#define CONFIG_DYNLINK_SYMBOL_SIZE \
		(sizeof(config_dynlink_symbol_prefix) + CONFIG_DYNLINK_NAME_SIZE + sizeof(config_dynlink_symbol_suffix))

	char config_dynlink_symbol[CONFIG_DYNLINK_SYMBOL_SIZE];

	strncpy(config_dynlink_symbol, config_dynlink_symbol_prefix, CONFIG_DYNLINK_SYMBOL_SIZE - 1);

	strncat(config_dynlink_symbol, name,
		CONFIG_DYNLINK_SYMBOL_SIZE - strnlen(config_dynlink_symbol, CONFIG_DYNLINK_SYMBOL_SIZE - 1) - 1);

	strncat(config_dynlink_symbol, config_dynlink_symbol_suffix,
		CONFIG_DYNLINK_SYMBOL_SIZE - strnlen(config_dynlink_symbol, CONFIG_DYNLINK_SYMBOL_SIZE - 1) - 1);

	#undef CONFIG_DYNLINK_SYMBOL_SIZE

	log_write("metacall", LOG_LEVEL_DEBUG, "Config symbol: %s", config_dynlink_symbol);

	return dynlink_symbol(handle, config_dynlink_symbol, singleton_addr_ptr);
}

int configuration_impl_initialize(const char * name)
{
	configuration_impl_singleton singleton = configuration_impl_singleton_instance();

	dynlink_symbol_addr singleton_addr;

	configuration_interface_singleton iface_singleton;

	singleton->handle = configuration_impl_initialize_load(name);

	if (singleton->handle == NULL)
	{
		return 1;
	}

	if (configuration_impl_initialize_symbol(singleton->handle, name, &singleton_addr) != 0)
	{
		dynlink_unload(singleton->handle);

		singleton->handle = NULL;

		return 1;
	}

	iface_singleton = (configuration_interface_singleton)DYNLINK_SYMBOL_GET(singleton_addr);

	singleton->iface = iface_singleton();

	return singleton->iface->initialize();
}

int configuration_impl_load_impl(configuration config)
{
	configuration_impl_singleton singleton = configuration_impl_singleton_instance();

	configuration_impl impl = singleton->iface->load(config);

	if (impl == NULL)
	{
		return 1;
	}

	configuration_object_instantiate(config, impl);

	return 0;
}

int configuration_impl_load(configuration config)
{
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

		vector_pop_front(queue);

		if (configuration_impl_load_impl(current) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration implementation load (childs) <%p>", current);

			set_destroy(storage);

			vector_destroy(queue);

			vector_destroy(childs);

			return 1;
		}

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

int configuration_impl_unload(configuration config)
{
	configuration_impl_singleton singleton = configuration_impl_singleton_instance();

	if (singleton->iface != NULL && singleton->iface->unload != NULL)
	{
		return singleton->iface->unload(config);
	}

	return 0;
}

int configuration_impl_destroy()
{
	configuration_impl_singleton singleton = configuration_impl_singleton_instance();

	int result = 0;

	if (singleton->iface != NULL && singleton->iface->destroy)
	{
        result = singleton->iface->destroy();
	}

	singleton->iface = NULL;

	if (singleton->handle != NULL)
	{
		dynlink_unload(singleton->handle);
	}

	singleton->handle = NULL;

	return result;
}
