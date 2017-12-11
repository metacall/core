/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#include <log/log_singleton.h>
#include <log/log_map.h>

#include <stdlib.h>

/* -- Definitions -- */

#define LOG_SINGLETON_MAP_SIZE	((size_t)0x0200)

/* -- Member Data -- */

struct log_singleton_type
{
	log_map map;
};

/* -- Private Methods -- */

static log_singleton log_singleton_create(void);

static int log_singleton_destroy(void);

static log_singleton log_singleton_instance_impl(void);

/* -- Methods -- */

static void log_atexit_callback(void)
{
	int result = log_singleton_destroy();

	(void)result;
}

log_singleton log_singleton_create()
{
	log_singleton s = malloc(sizeof(struct log_singleton_type));

	if (s == NULL)
	{
		return NULL;
	}

	s->map = log_map_create(LOG_SINGLETON_MAP_SIZE);

	if (s->map == NULL)
	{
		free(s);

		return NULL;
	}

	return s;
}

int log_singleton_destroy()
{
	log_singleton * s = log_singleton_instance();

	if (*s == NULL)
	{
		abort();
	}

	log_singleton_clear();

	if (log_map_destroy((*s)->map) != 0)
	{
		return 1;
	}

	(*s)->map = NULL;

	free(*s);

	*s = NULL;

	return 0;
}

log_singleton log_singleton_instance_impl()
{
	log_singleton * singleton_ptr = log_singleton_instance();

	if (*singleton_ptr == NULL)
	{
		*singleton_ptr = log_singleton_create();

		if (*singleton_ptr == NULL)
		{
			abort();
		}

		if (atexit(&log_atexit_callback) != 0)
		{
			if (log_singleton_destroy() != 0)
			{
				/* ... */
			}

			abort();
		}
	}

	return *singleton_ptr;
}

log_singleton * log_singleton_instance()
{
	static log_singleton s = NULL;

	return &s;
}

int log_singleton_initialize(log_singleton singleton)
{
	log_singleton * s = log_singleton_instance();

	if (*s != NULL)
	{
		/* Log already initialized */

		return 1;
	}

	*s = singleton;

	return 0;
}

int log_singleton_insert(const char * name, log_impl impl)
{
	log_singleton s = log_singleton_instance_impl();

	return log_map_insert(s->map, name, impl);
}

log_impl log_singleton_get(const char * name)
{
	log_singleton s = log_singleton_instance_impl();

	return (log_impl)log_map_get(s->map, name);
}

log_impl log_singleton_remove(const char * name)
{
	log_singleton s = log_singleton_instance_impl();

	return (log_impl)log_map_remove(s->map, name);
}

void log_singleton_clear()
{
	log_singleton s = log_singleton_instance_impl();

	if (log_map_clear(s->map) != 0)
	{
		/* ... */
	}
}
