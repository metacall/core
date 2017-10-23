/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

/* -- Headers -- */

#include <serial/serial_impl.h>
#include <serial/serial_singleton.h>

#include <environment/environment_variable_path.h>

#include <adt/adt_vector.h>
#include <adt/adt_set.h>

#include <dynlink/dynlink.h>

#include <log/log.h>

#include <string.h>

/* -- Definitions -- */

#define SERIAL_DYNLINK_NAME_SIZE		0x40
#define SERIAL_LIBRARY_PATH			"SERIAL_LIBRARY_PATH"
#define SERIAL_DEFAULT_LIBRARY_PATH	"serials"

/* -- Forward Declarations -- */

struct serial_impl_singleton_type;

/* -- Type Definitions -- */

typedef struct serial_impl_singleton_type * serial_impl_singleton;

/* -- Member Data -- */

struct serial_impl_singleton_type
{
	char * library_path;
	serial_interface iface;
	dynlink handle;
};

/* -- Private Methods -- */

static serial_impl_singleton serial_impl_singleton_instance(void);

static int serial_impl_load_impl(serial s);

static dynlink serial_impl_initialize_load(const char * name);

static int serial_impl_initialize_symbol(dynlink handle, const char * name, dynlink_symbol_addr * singleton_addr_ptr);

/* -- Methods -- */

serial_impl_singleton serial_impl_singleton_instance()
{
	static struct serial_impl_singleton_type instance =
	{
		NULL,
		NULL,
		NULL
	};

	return &instance;
}

const char * serial_impl_extension()
{
	serial_impl_singleton singleton = serial_impl_singleton_instance();

	return singleton->iface->extension();
}

dynlink serial_impl_initialize_load(const char * name)
{
	serial_impl_singleton singleton = serial_impl_singleton_instance();

	#if (!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
		const char config_dynlink_suffix[] = "_configd";
	#else
		const char config_dynlink_suffix[] = "_config";
	#endif

	#define SERIAL_DYNLINK_NAME_FULL_SIZE \
		(sizeof(config_dynlink_suffix) + SERIAL_DYNLINK_NAME_SIZE)

	char config_dynlink_name[SERIAL_DYNLINK_NAME_FULL_SIZE];

	strncpy(config_dynlink_name, name, SERIAL_DYNLINK_NAME_FULL_SIZE - 1);

	strncat(config_dynlink_name, config_dynlink_suffix,
		SERIAL_DYNLINK_NAME_FULL_SIZE - strnlen(config_dynlink_name, SERIAL_DYNLINK_NAME_FULL_SIZE - 1) - 1);

	#undef SERIAL_DYNLINK_NAME_FULL_SIZE

	log_write("metacall", LOG_LEVEL_DEBUG, "Loading config: %s", config_dynlink_name);

	return dynlink_load(singleton->library_path, config_dynlink_name, DYNLINK_FLAGS_BIND_LAZY | DYNLINK_FLAGS_BIND_GLOBAL);
}

int serial_impl_initialize_symbol(dynlink handle, const char * name, dynlink_symbol_addr * singleton_addr_ptr)
{
	const char config_dynlink_symbol_prefix[] = DYNLINK_SYMBOL_STR("");
	const char config_dynlink_symbol_suffix[] = "_config_impl_interface_singleton";

	#define SERIAL_DYNLINK_SYMBOL_SIZE \
		(sizeof(config_dynlink_symbol_prefix) + SERIAL_DYNLINK_NAME_SIZE + sizeof(config_dynlink_symbol_suffix))

	char config_dynlink_symbol[SERIAL_DYNLINK_SYMBOL_SIZE];

	strncpy(config_dynlink_symbol, config_dynlink_symbol_prefix, SERIAL_DYNLINK_SYMBOL_SIZE - 1);

	strncat(config_dynlink_symbol, name,
		SERIAL_DYNLINK_SYMBOL_SIZE - strnlen(config_dynlink_symbol, SERIAL_DYNLINK_SYMBOL_SIZE - 1) - 1);

	strncat(config_dynlink_symbol, config_dynlink_symbol_suffix,
		SERIAL_DYNLINK_SYMBOL_SIZE - strnlen(config_dynlink_symbol, SERIAL_DYNLINK_SYMBOL_SIZE - 1) - 1);

	#undef SERIAL_DYNLINK_SYMBOL_SIZE

	log_write("metacall", LOG_LEVEL_DEBUG, "Config symbol: %s", config_dynlink_symbol);

	return dynlink_symbol(handle, config_dynlink_symbol, singleton_addr_ptr);
}

int serial_impl_initialize(const char * name)
{
	serial_impl_singleton singleton = serial_impl_singleton_instance();

	dynlink_symbol_addr singleton_addr;

	serial_interface_singleton iface_singleton;

	static const char serial_library_path[] = SERIAL_LIBRARY_PATH;

	singleton->library_path = environment_variable_path_create(serial_library_path, SERIAL_DEFAULT_LIBRARY_PATH);

	if (singleton->library_path == NULL)
	{
		return 1;
	}

	singleton->handle = serial_impl_initialize_load(name);

	if (singleton->handle == NULL)
	{
		return 1;
	}

	if (serial_impl_initialize_symbol(singleton->handle, name, &singleton_addr) != 0)
	{
		dynlink_unload(singleton->handle);

		singleton->handle = NULL;

		return 1;
	}

	iface_singleton = (serial_interface_singleton)DYNLINK_SYMBOL_GET(singleton_addr);

	singleton->iface = iface_singleton();

	return singleton->iface->initialize();
}

int serial_impl_load_impl(serial s)
{
	serial_impl_singleton singleton = serial_impl_singleton_instance();

	serial_impl impl = singleton->iface->load(config);

	if (impl == NULL)
	{
		return 1;
	}

	serial_object_instantiate(config, impl);

	return 0;
}

int serial_impl_load(serial s)
{
	set storage = set_create(&hash_callback_str, &comparable_callback_str);

	vector queue, childs;

	if (storage == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation load set allocation");

		return 1;
	}

	queue = vector_create(sizeof(serial));

	if (queue == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation load queue allocation");

		set_destroy(storage);

		return 1;
	}

	childs = vector_create(sizeof(serial));

	if (queue == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation load childs allocation");

		set_destroy(storage);

		vector_destroy(queue);

		return 1;
	}

	vector_push_back(queue, &config);

	while (vector_size(queue) > 0)
	{
		serial current = *((serial *)vector_front(queue));

		vector_pop_front(queue);

		if (serial_impl_load_impl(current) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation load (childs) <%p>", current);

			set_destroy(storage);

			vector_destroy(queue);

			vector_destroy(childs);

			return 1;
		}

		vector_clear(childs);

		if (serial_object_childs(current, childs, storage) == 0 && vector_size(childs) > 0)
		{
			size_t iterator;

			for (iterator = 0; iterator < vector_size(childs); ++iterator)
			{
				serial child = *((serial *)vector_at(childs, iterator));

				if (set_get(storage, (set_key)serial_object_name(child)) == NULL)
				{
					if (serial_singleton_register(child) != 0)
					{
						log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation child singleton insertion (%s, %s)",
							serial_object_name(child), serial_object_path(child));

						set_destroy(storage);

						vector_destroy(queue);

						vector_destroy(childs);

						return 1;
					}

					vector_push_back(queue, &child);
				}

				if (set_insert(storage, (set_key)serial_object_name(child), child) != 0)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Invalid serial implementation child set insertion");

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

int serial_impl_unload(serial s)
{
	serial_impl_singleton singleton = serial_impl_singleton_instance();

	if (singleton->iface != NULL && singleton->iface->unload != NULL)
	{
		return singleton->iface->unload(config);
	}

	return 0;
}

int serial_impl_destroy()
{
	serial_impl_singleton singleton = serial_impl_singleton_instance();

	int result = 0;

	if (singleton->library_path != NULL)
	{
		environment_variable_path_destroy(singleton->library_path);

		singleton->library_path = NULL;
	}

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
