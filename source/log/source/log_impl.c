/*
 *	Logger Library by Parra Studios
 *	A generic logger library providing application execution reports.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
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
	enum log_level_id level;
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

		#if !defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__)
			impl->level = LOG_LEVEL_DEBUG;
		#else
			impl->level = LOG_LEVEL_INFO;
		#endif

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

enum log_level_id log_impl_level(log_impl impl)
{
	return impl->level;
}

void log_impl_verbosity(log_impl impl, enum log_level_id level)
{
	impl->level = level;
}

int log_impl_write(log_impl impl, const log_record_ctor record_ctor)
{
	log_aspect stream = log_impl_aspect(impl, LOG_ASPECT_STREAM);

	log_aspect_stream_impl stream_impl = log_aspect_derived(stream);

	if (stream_impl->write(stream, record_ctor) != 0)
	{
		return 1;
	}

	return 0;
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
