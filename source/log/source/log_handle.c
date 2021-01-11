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

#include <log/log_handle.h>

#include <stdint.h>
#include <string.h>

/* -- Definitions -- */

#define LOG_HANDLE_RECORD_SIZE_DEFAULT UINTMAX_C(0x0100)
#define LOG_HANDLE_RECORD_SIZE_FULL UINTMAX_C(0x8000)

/* -- Member Data -- */

struct log_handle_type
{
	log_record buffer;
	size_t count;
	size_t size;
};

/* -- Methods -- */

log_handle log_handle_create()
{
	log_handle handle = malloc(sizeof(struct log_handle_type));

	if (handle == NULL)
	{
		return NULL;
	}

	return log_handle_initialize(handle);
}

log_handle log_handle_initialize(log_handle handle)
{
	if (handle == NULL)
	{
		return NULL;
	}

	handle->buffer = malloc(LOG_HANDLE_RECORD_SIZE_DEFAULT * log_record_size());

	handle->count = 0;

	handle->size = LOG_HANDLE_RECORD_SIZE_DEFAULT;

	if (handle->buffer == NULL)
	{
		free(handle);

		return NULL;
	}

	return handle;
}

log_record log_handle_get(log_handle handle, const size_t record_id)
{
	if (record_id < handle->count)
	{
		void * const ptr = (unsigned char *)(handle->buffer) + (record_id * log_record_size());

		return ptr;
	}

	return NULL;
}

log_record log_handle_set(log_handle handle, const size_t record_id, const log_record_ctor record_ctor)
{
	if (record_id < handle->count)
	{
		void * const ptr = (unsigned char *)(handle->buffer) + (record_id * log_record_size());

		return log_record_initialize(ptr, record_ctor);
	}

	return NULL;
}

log_record log_handle_push(log_handle handle, const log_record_ctor record_ctor)
{
	if ((handle->count + 1) == handle->size)
	{
		const size_t new_size = handle->size << 0x01;

		register void * buffer = realloc(handle->buffer, new_size);

		if (buffer == NULL)
		{
			return NULL;
		}

		handle->buffer = buffer;

		handle->size = new_size;
	}

	if (handle->count > 0)
	{
		void * dest = (unsigned char *)(handle->buffer) + log_record_size();

		memmove(dest, handle->buffer, log_record_size() * handle->count);
	}

	++handle->count;

	return log_handle_set(handle, 0, record_ctor);
}

log_record log_handle_pop(log_handle handle)
{
	if (handle->count > 0)
	{
		const size_t new_size = handle->size >> 0x01;

		--handle->count;

		if (new_size >= LOG_HANDLE_RECORD_SIZE_DEFAULT && handle->count < new_size)
		{
			register void * buffer = realloc(handle->buffer, new_size);

			if (buffer == NULL)
			{
				return NULL;
			}

			handle->buffer = buffer;

			handle->size = new_size;
		}

		return (void * const)((unsigned char *)(handle->buffer) + (handle->count * log_record_size()));
	}

	return NULL;
}

size_t log_handle_count(log_handle handle)
{
	return handle->count;
}

size_t log_handle_size(log_handle handle)
{
	return handle->size;
}

int log_handle_clear(log_handle handle)
{
	handle->count = 0;

	if (handle->size != LOG_HANDLE_RECORD_SIZE_DEFAULT)
	{
		const size_t default_size = LOG_HANDLE_RECORD_SIZE_DEFAULT;

		register void * buffer = realloc(handle->buffer, default_size);

		if (buffer == NULL)
		{
			return 1;
		}

		handle->buffer = buffer;

		handle->size = default_size;
	}

	return 0;
}

int log_handle_destroy(log_handle handle)
{
	if (handle != NULL)
	{
		if (handle->buffer != NULL)
		{
			free(handle->buffer);
		}

		free(handle);
	}

	return 0;
}
