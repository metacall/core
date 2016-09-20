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
#include <log/log_impl.h>
#include <log/log_singleton.h>

#include <stdio.h>

/* -- Methods -- */

int log_create(const char * name)
{
	log_singleton s = log_instance();

	return 0;
}

int log_define(const char * name, enum log_aspect_id aspect_id, log_policy policy)
{
	log_singleton s = log_instance();

	return 0;
}


int log_write_impl(const char * name, const char * tag, size_t line, const char * func, const char * file, enum log_level_id level, const char * message)
{
	log_singleton s = log_instance();

	return 0;
}

int log_write_impl_v(const char * name, const char * tag, size_t line, const char * func, const char * file, enum log_level_id level, const char * message, void * args[])
{
	log_singleton s = log_instance();

	return 0;
}

int log_write_impl_va(const char * name, const char * tag, size_t line, const char * func, const char * file, enum log_level_id level, const char * message, ...)
{
	log_singleton s = log_instance();

	return 0;
}

int log_clear(const char * name)
{
	log_singleton s = log_instance();

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
