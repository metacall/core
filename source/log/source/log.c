/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <metacall/metacall-version.h>

#include <log/log.h>

#include <stdio.h>

/* -- Forward Declarations -- */

struct log_type;

struct log_singleton_type;

/* -- Type Definitions -- */

typedef struct log_type * log;

typedef struct log_singleton_type * log_singleton;

/* -- Member Data -- */

struct log_singleton_type
{
	log *	map;
	size_t	size;
};

/* -- Private Methods -- */

log_singleton log_instance(void);

int log_initialize(void);

int log_destroy(void);

/* -- Methods -- */

log_singleton log_instance()
{
	static struct log_singleton_type singleton =
	{
		NULL, 0
	};

	return &singleton;
}

int log_initialize()
{
	log_singleton s = log_instance();

	if (/*s->log_map == */NULL)
	{
		//s->log_map = hash_map_create(&hash_callback_str, &comparable_callback_str)
	}

	return 0;
}

int log_create(const char * name, log_policy policy, log_impl impl)
{
	return 0;
}

int log_write(const char * name, const char * tag, enum log_level_id level, const char * message)
{
	return 0;
}

int log_write_v(const char * name, const char * tag, enum log_level_id level, const char * message, void * args[])
{
	return 0;
}

int log_write_va(const char * name, const char * tag, enum log_level_id level, const char * message, ...)
{
	return 0;
}

int log_clear(const char * name)
{
	return 0;
}

int log_destroy()
{
	return 0;
}

void log_print_info()
{
	printf("Logger Library " METACALL_VERSION "\n");
	printf("Copyright (c) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n");

	#ifdef LOG_STATIC_DEFINE
		printf("Compiled as static library type\n");
	#else
		printf("Compiled as shared library type\n");
	#endif
}
