/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

/* -- Headers -- */

#include <metacall/metacall_error.h>

#include <reflect/reflect_value_type.h>

#include <log/log.h>

/* -- Methods -- */

void *metacall_error_throw(const char *label, int64_t code, const char *stacktrace, const char *message, ...)
{
	va_list args, args_copy;
	int length;
	char *buffer;
	exception ex;
	throwable th;

	va_start(args, message);
	va_copy(args_copy, args);
	length = vsnprintf(NULL, 0, message, args_copy);
	va_end(args_copy);

	if (length < 0)
	{
		va_end(args);
		return NULL; /* TODO: Return a generic static error here */
	}

	buffer = malloc(length + 1);

	if (!buffer)
	{
		va_end(args);
		return NULL; /* TODO: Return a generic static error here */
	}

	vsnprintf(buffer, length + 1, message, args);
	va_end(args);

	ex = exception_create_message_const(buffer, label, code, stacktrace);
	th = throwable_create(value_create_exception(ex));
	return value_create_throwable(th);
}

int metacall_error_from_value(void *v, metacall_exception ex)
{
	if (v == NULL || ex == NULL)
	{
		return 1;
	}

	if (type_id_throwable(value_type_id(v)) == 0)
	{
		v = throwable_value(value_to_throwable(v));
	}

	if (type_id_exception(value_type_id(v)) != 0)
	{
		return 1;
	}

	void *ex_impl = value_to_exception(v);

	ex->message = exception_message(ex_impl);
	ex->label = exception_label(ex_impl);
	ex->code = exception_error_code(ex_impl);
	ex->stacktrace = exception_stacktrace(ex_impl);

	return 0;
}

int metacall_error_last(metacall_exception ex)
{
	// TODO
	(void)ex;

	return 1;
}

void metacall_error_clear(void)
{
	// TODO
}
