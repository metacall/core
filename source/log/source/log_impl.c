/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_impl.h>
#include <log/log_handle.h>

#include <stdlib.h>

/* -- Member Data -- */

struct log_impl_type
{
	const char * name;
	log_handle handle;
};

/* -- Methods -- */

log_impl log_impl_create(const char * name)
{
	log_impl impl = malloc(sizeof(struct log_impl_type));

	if (impl != NULL)
	{
		impl->name = name;
		impl->handle = log_handle_create();
	}

	return impl;
}

const char * log_impl_name(log_impl impl)
{
	return impl->name;
}

int log_impl_write(log_impl impl, const char * tag, const log_record_ctor record_ctor)
{
	const log_record record = log_handle_push(impl->handle, record_ctor);

	/* TODO: apply point-cuts with callbacks to the policies */

	if (record != NULL)
	{
		/* TODO: index record by tag */

		return 0;
	}

	return 1;
}

int log_impl_clear(log_impl impl)
{
	/* TODO: remove all tags associated to records */

	return log_handle_clear(impl->handle);
}

int log_impl_destroy(log_impl impl)
{
	if (impl != NULL)
	{
		const int result = log_handle_destroy(impl->handle);

		free(impl);

		return result;
	}

	return 0;
}
