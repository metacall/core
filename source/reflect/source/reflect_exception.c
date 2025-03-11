/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <reflect/reflect_exception.h>

#include <threading/threading_atomic_ref_count.h>
#include <threading/threading_thread_id.h>

#include <reflect/reflect_memory_tracker.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct exception_type
{
	char *message;	  /* Description of the error */
	char *label;	  /* Type of error */
	int64_t code;	  /* Numeric code of error */
	char *stacktrace; /* Stack trace of the error */
	uint64_t id;	  /* Thread id where the error was raised */
	struct threading_atomic_ref_count_type ref;
	/* TODO: value attributes; // This should implement a map for representing the extra attributes of an exception */
};

reflect_memory_tracker(exception_stats);

exception exception_create(char *message, char *label, int64_t code, char *stacktrace)
{
	exception ex = malloc(sizeof(struct exception_type));

	if (ex == NULL)
	{
		return NULL;
	}

	ex->message = message;
	ex->label = label;
	ex->code = code;
	ex->stacktrace = stacktrace;
	ex->id = thread_id_get_current();

	threading_atomic_ref_count_initialize(&ex->ref);

	reflect_memory_tracker_allocation(exception_stats);

	return ex;
}

exception exception_create_const(const char *message, const char *label, int64_t code, const char *stacktrace)
{
	exception ex = malloc(sizeof(struct exception_type));

	if (ex == NULL)
	{
		goto exception_bad_alloc;
	}

	if (message != NULL)
	{
		size_t message_size = strlen(message) + 1;

		ex->message = malloc(sizeof(char) * message_size);

		if (ex->message == NULL)
		{
			goto message_bad_alloc;
		}

		memcpy(ex->message, message, message_size);
	}
	else
	{
		ex->message = NULL;
	}

	if (label != NULL)
	{
		size_t label_size = strlen(label) + 1;

		ex->label = malloc(sizeof(char) * label_size);

		if (ex->label == NULL)
		{
			goto label_bad_alloc;
		}

		memcpy(ex->label, label, label_size);
	}
	else
	{
		ex->label = NULL;
	}

	if (stacktrace != NULL)
	{
		size_t stacktrace_size = strlen(stacktrace) + 1;

		ex->stacktrace = malloc(sizeof(char) * stacktrace_size);

		if (ex->stacktrace == NULL)
		{
			goto stacktrace_bad_alloc;
		}

		memcpy(ex->stacktrace, stacktrace, stacktrace_size);
	}
	else
	{
		ex->stacktrace = NULL;
	}

	ex->code = code;
	ex->id = thread_id_get_current();

	threading_atomic_ref_count_initialize(&ex->ref);

	reflect_memory_tracker_allocation(exception_stats);

	return ex;

stacktrace_bad_alloc:
	free(ex->label);
label_bad_alloc:
	free(ex->message);
message_bad_alloc:
	free(ex);
exception_bad_alloc:
	return NULL;
}

int exception_increment_reference(exception ex)
{
	if (ex == NULL)
	{
		return 1;
	}

	if (threading_atomic_ref_count_increment(&ex->ref) == 1)
	{
		return 1;
	}

	reflect_memory_tracker_increment(exception_stats);

	return 0;
}

int exception_decrement_reference(exception ex)
{
	if (ex == NULL)
	{
		return 1;
	}

	if (threading_atomic_ref_count_decrement(&ex->ref) == 1)
	{
		return 1;
	}

	reflect_memory_tracker_decrement(exception_stats);

	return 0;
}

const char *exception_message(exception ex)
{
	if (ex == NULL)
	{
		return NULL;
	}

	return ex->message;
}

const char *exception_label(exception ex)
{
	if (ex == NULL)
	{
		return NULL;
	}

	return ex->label;
}

int64_t exception_error_code(exception ex)
{
	if (ex == NULL)
	{
		return 0;
	}

	return ex->code;
}

const char *exception_stacktrace(exception ex)
{
	if (ex == NULL)
	{
		return NULL;
	}

	return ex->stacktrace;
}

void exception_stats_debug(void)
{
	reflect_memory_tracker_print(exception_stats, "EXCEPTIONS");
}

void exception_destroy(exception ex)
{
	if (ex != NULL)
	{
		if (exception_decrement_reference(ex) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid reference counter in exception: %s", ex->label ? ex->label : "<anonymous>");
		}

		if (threading_atomic_ref_count_load(&ex->ref) == 0)
		{
			if (ex->message != NULL)
			{
				free(ex->message);
			}

			if (ex->label != NULL)
			{
				free(ex->label);
			}

			if (ex->stacktrace != NULL)
			{
				free(ex->stacktrace);
			}

			threading_atomic_ref_count_destroy(&ex->ref);

			free(ex);

			reflect_memory_tracker_deallocation(exception_stats);
		}
	}
}
