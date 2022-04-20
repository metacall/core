/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <threading/threading_thread_id.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct exception_type
{
	char *message;	  /* Description of the error */
	char *label;	  /* Type of error */
	int number;		  /* Numeric code of error */
	char *stacktrace; /* Stack trace of the error */
	uint64_t id;	  /* Thread id where the error was raised */
	size_t ref_count;
};

static struct
{
	uint64_t allocations;
	uint64_t deallocations;
	uint64_t increments;
	uint64_t decrements;
} exception_stats = { 0, 0, 0, 0 };

exception exception_create(char *message, char *label, int number, char *stacktrace)
{
	exception ex = malloc(sizeof(struct exception_type));

	if (ex == NULL)
	{
		return NULL;
	}

	ex->message = message;
	ex->label = label;
	ex->number = number;
	ex->stacktrace = stacktrace;
	ex->id = thread_id_get_current();
	ex->ref_count = 0;

	return ex;
}

exception exception_create_const(const char *message, const char *label, int number, const char *stacktrace)
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

	ex->number = number;
	ex->id = thread_id_get_current();
	ex->ref_count = 0;

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

	if (ex->ref_count == SIZE_MAX)
	{
		return 1;
	}

	++ex->ref_count;
	++exception_stats.increments;

	++exception_stats.allocations;

	return 0;
}

int exception_decrement_reference(exception ex)
{
	if (ex == NULL)
	{
		return 1;
	}

	if (ex->ref_count == 0)
	{
		return 1;
	}

	--ex->ref_count;
	++exception_stats.decrements;

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

int exception_number(exception ex)
{
	if (ex == NULL)
	{
		return 0;
	}

	return ex->number;
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
#if !(!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	if (exception_stats.allocations != exception_stats.deallocations || exception_stats.increments != exception_stats.decrements)
#endif
	{
		printf("----------------- EXCEPTIONS -----------------\n");
		printf("Allocations: %" PRIuS "\n", exception_stats.allocations);
		printf("Deallocations: %" PRIuS "\n", exception_stats.deallocations);
		printf("Increments: %" PRIuS "\n", exception_stats.increments);
		printf("Decrements: %" PRIuS "\n", exception_stats.decrements);
		fflush(stdout);
	}
}

void exception_destroy(exception ex)
{
	if (ex != NULL)
	{
		if (exception_decrement_reference(ex) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid reference counter in exception: %s", ex->label ? ex->label : "<anonymous>");
		}

		if (ex->ref_count == 0)
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

			free(ex);

			++exception_stats.deallocations;
		}
	}
}
