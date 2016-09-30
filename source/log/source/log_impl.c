/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_impl.h>

#include <log/log_aspect_format.h>
#include <log/log_aspect_schedule.h>
#include <log/log_aspect_storage.h>
#include <log/log_aspect_stream.h>

#include <stdlib.h>

/* -- Member Data -- */

struct log_impl_type
{
	const char * name;
	log_handle handle;
	log_aspect aspects[LOG_ASPECT_SIZE];
};

/* -- Methods -- */

log_aspect_interface log_impl_aspect_interface(enum log_aspect_id aspect_id)
{
	static log_aspect_singleton aspect_impl[LOG_ASPECT_SIZE] =
	{
		&log_aspect_format_interface,
		&log_aspect_schedule_interface,
		&log_aspect_storage_interface,
		&log_aspect_stream_interface
	};

	return aspect_impl[aspect_id]();
}

log_impl log_impl_create(const char * name)
{
	log_impl impl = malloc(sizeof(struct log_impl_type));

	if (impl != NULL)
	{
		size_t iterator;

		impl->name = name;
		impl->handle = log_handle_create();

		for (iterator = 0; iterator < LOG_ASPECT_SIZE; ++iterator)
		{
			impl->aspects[iterator] = NULL;
		}
	}

	return impl;
}

void log_impl_define(log_impl impl, log_aspect aspect, enum log_aspect_id aspect_id)
{
	impl->aspects[aspect_id] = aspect;
}

const char * log_impl_name(log_impl impl)
{
	return impl->name;
}

log_handle log_impl_handle(log_impl impl)
{
	return impl->handle;
}

log_aspect log_impl_aspect(log_impl impl, enum log_aspect_id aspect_id)
{
	if (aspect_id == LOG_ASPECT_SIZE)
	{
		return NULL;
	}

	return impl->aspects[aspect_id];
}

/*
int log_impl_lock(log_impl impl)
{
	log_aspect schedule = impl->aspects[LOG_ASPECT_SCHEDULE];

	log_aspect_schedule_impl schedule_impl = log_aspect_derived(schedule);

	return schedule_impl->lock(schedule);
}

int log_impl_unlock(log_impl impl)
{
	log_aspect schedule = impl->aspects[LOG_ASPECT_SCHEDULE];

	log_aspect_schedule_impl schedule_impl = log_aspect_derived(schedule);

	return schedule_impl->unlock(schedule);
}
*/

int log_impl_write(log_impl impl, const log_record_ctor record_ctor)
{
	log_aspect stream = log_impl_aspect(impl, LOG_ASPECT_STREAM);

	log_aspect_stream_impl stream_impl = log_aspect_derived(stream);

	if (stream_impl->write(stream, record_ctor) != 0)
	{
		return 1;
	}

	return 0;

	/*
	log_aspect format = impl->aspects[LOG_ASPECT_FORMAT];

	log_aspect_format_impl format_impl = log_aspect_derived(format);

	log_aspect storage = impl->aspects[LOG_ASPECT_STORAGE];

	log_aspect_storage_impl storage_impl = log_aspect_derived(storage);

	log_aspect stream = impl->aspects[LOG_ASPECT_STREAM];

	log_aspect_stream_impl stream_impl = log_aspect_derived(stream);
	*/

	/*
	log_record record = log_handle_push(impl->handle, record_ctor);

	log_aspect stream = impl->aspects[LOG_ASPECT_STREAM];

	log_aspect_stream_impl stream_impl = log_aspect_derived(stream);

	if (record == NULL)
	{
		return 1;
	}

	if (log_impl_lock(impl) != 0)
	{
		return 1;
	}


	
	if (stream_impl->write(stream, record_ctor->message, strlen(record_ctor->message)) != 0)
	{
		if (log_impl_unlock(impl) != 0)
		{
			return 1;
		}

		return 1;
	}

	if (stream_impl->flush(stream) != 0)
	{
		if (log_impl_unlock(impl) != 0)
		{
			return 1;
		}

		return 1;
	}
	
	if (log_impl_unlock(impl) != 0)
	{
		return 1;
	}

	return 0;
	*/
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
		int result;

		size_t iterator;

		for (iterator = 0; iterator < LOG_ASPECT_SIZE; ++iterator)
		{
			if (impl->aspects[iterator] != NULL)
			{
				log_aspect_destroy(impl->aspects[iterator]);
			}
		}

		result = log_handle_destroy(impl->handle);

		free(impl);

		return result;
	}

	return 0;
}
