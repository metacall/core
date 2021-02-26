/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#include <reflect/reflect_exception.h>

#include <threading/threading_thread_id.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct exception_type
{
	char * message;
	char * label;
	int code;
	char * stacktrace;
	uint64_t id;
};

exception exception_create(const char * message, const char * label, int code, const char * stacktrace)
{
	exception ex = malloc(sizeof(struct exception_type));

	if (ex == NULL)
	{
		return NULL;
	}

	// TODO: Copy

	ex->id = thread_id_get_current();

	return ex;
}

const char * exception_message(exception ex)
{
	if (ex == NULL)
	{
		return NULL;
	}

	return ex->message;
}

const char * exception_label(exception ex)
{
	if (ex == NULL)
	{
		return NULL;
	}

	return ex->label;
}

int exception_code(exception ex)
{
	if (ex == NULL)
	{
		return 0;
	}

	return ex->code;
}

const char * exception_stacktrace(exception ex)
{
	if (ex == NULL)
	{
		return NULL;
	}

	return ex->stacktrace;
}

int exception_thrown(exception ex)
{
	if (ex == NULL)
	{
		return 1;
	}

	return ex->thrown;
}

void exception_destroy(exception ex)
{
	if (ex != NULL)
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
	}
}
