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


/* -- Methods -- */

int log_create(const char * name)
{
	log_impl impl = log_impl_create(name);

	if (impl == NULL)
	{
		return 1;
	}

	if (log_singleton_insert(log_impl_name(impl), impl) != 0)
	{
		log_impl_destroy(impl);

		return 1;
	}

	return 0;
}

int log_define(const char * name, log_policy policy)
{
	log_impl impl = log_singleton_get(name);

	enum log_aspect_id aspect_id = log_policy_aspect_id(policy);

	log_aspect aspect = log_impl_aspect(impl, aspect_id);

	if (aspect == NULL)
	{
		log_aspect_interface aspect_iface = log_impl_aspect_interface(aspect_id);

		/* TODO: Review aspect constructor */

		aspect = log_aspect_create(aspect_iface, NULL);

		if (aspect == NULL)
		{
			return 1;
		}

		log_impl_define(impl, aspect, aspect_id);
	}

	if (log_aspect_attach(aspect, policy) != 0)
	{
		return 1;
	}

	return 0;
}


int log_write_impl(const char * name, const char * tag, const size_t line, const char * func, const char * file, const enum log_level_id level, const char * message)
{
	log_impl impl = log_singleton_get(name);

	struct log_record_ctor_type record_ctor;
	
	record_ctor.line = line;
	record_ctor.func = func;
	record_ctor.file = file;
	record_ctor.level = level;
	record_ctor.message = message;

	return log_impl_write(impl, tag, &record_ctor);
}

int log_write_impl_v(const char * name, const char * tag, const size_t line, const char * func, const char * file, const enum log_level_id level, const char * message, void * args[])
{
	static const char unimplemented_str[] = "TODO: log_write_impl_v unimplemented.";

	log_impl impl = log_singleton_get(name);

	struct log_record_ctor_type record_ctor;

	record_ctor.line = line;
	record_ctor.func = func;
	record_ctor.file = file;
	record_ctor.level = level;
	record_ctor.message = unimplemented_str /* message */;

	/* TODO: Parse mesasge and apply arguments in a new string */
	(void)message;
	(void)args;

	return log_impl_write(impl, tag, &record_ctor);
}

int log_write_impl_va(const char * name, const char * tag, const size_t line, const char * func, const char * file, const enum log_level_id level, const char * message, ...)
{
	static const char unimplemented_str[] = "TODO: log_write_impl_va unimplemented.";

	log_impl impl = log_singleton_get(name);

	struct log_record_ctor_type record_ctor;

	record_ctor.line = line;
	record_ctor.func = func;
	record_ctor.file = file;
	record_ctor.level = level;
	record_ctor.message = unimplemented_str /* message */;

	/* TODO: sprintf() */
	(void)message;

	return log_impl_write(impl, tag, &record_ctor);
}

int log_clear(const char * name)
{
	log_impl impl = log_singleton_get(name);

	if (impl == NULL)
	{
		return 1;
	}

	return log_impl_clear(impl);
}

int log_delete(const char * name)
{
	log_impl impl = log_singleton_remove(name);

	if (impl == NULL)
	{
		return 1;
	}

	return log_impl_destroy(impl);
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
