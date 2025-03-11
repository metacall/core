/*
 *	Logger Library by Parra Studios
 *	A generic logger library providing application execution reports.
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

#include <metacall/metacall_version.h>

#include <log/log.h>
#include <log/log_impl.h>
#include <log/log_singleton.h>

#include <stdarg.h>

/* -- Methods -- */

void *log_instance(void)
{
	return log_singleton_instance();
}

size_t log_size(void)
{
	return log_singleton_size();
}

int log_create(const char *name)
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

int log_define(const char *name, log_policy policy)
{
	log_impl impl = log_singleton_get(name);

	enum log_aspect_id aspect_id = log_policy_aspect_id(policy);

	log_aspect aspect = log_impl_aspect(impl, aspect_id);

	if (aspect == NULL)
	{
		log_aspect_interface aspect_iface = log_impl_aspect_interface(aspect_id);

		/* TODO: review aspect constructor */

		aspect = log_aspect_create(impl, aspect_iface, NULL);

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

int log_configure_impl(const char *name, size_t size, ...)
{
	log_impl impl = log_singleton_get(name);

	size_t iterator;

	va_list variable_args;

	if (impl == NULL)
	{
		if (log_create(name) != 0)
		{
			return 1;
		}
	}

	va_start(variable_args, size);

	for (iterator = 0; iterator < size; ++iterator)
	{
		log_policy policy = va_arg(variable_args, log_policy);

		if (log_define(name, policy) != 0)
		{
			va_end(variable_args);

			return 1;
		}
	}

	va_end(variable_args);

	return 0;
}

int log_level(const char *name, const char *level, size_t length)
{
	enum log_level_id id = log_level_to_enum(level, length);

	if (id < LOG_LEVEL_SIZE)
	{
		log_impl impl = log_singleton_get(name);

		if (impl != NULL)
		{
			log_impl_verbosity(impl, id);
		}

		return 0;
	}

	return 1;
}

int log_write_impl(const char *name, const size_t line, const char *func, const char *file, const enum log_level_id level, const char *message)
{
	log_impl impl = log_singleton_get(name);

	struct log_record_ctor_type record_ctor;

	enum log_level_id impl_level;

	if (impl == NULL)
	{
		return 0;
	}

	impl_level = log_impl_level(impl);

	if (level < impl_level)
	{
		return 0;
	}

	record_ctor.line = line;
	record_ctor.func = func;
	record_ctor.file = file;
	record_ctor.level = level;
	record_ctor.message = message;
	record_ctor.variable_args = NULL;

	return log_impl_write(impl, &record_ctor);
}

int log_write_impl_va(const char *name, const size_t line, const char *func, const char *file, const enum log_level_id level, const char *message, ...)
{
	log_impl impl = log_singleton_get(name);

	struct log_record_ctor_type record_ctor;

	int result;

	enum log_level_id impl_level;

	struct log_record_va_list_type variable_args;

	if (impl == NULL)
	{
		return 0;
	}

	impl_level = log_impl_level(impl);

	if (impl == NULL)
	{
		return 1;
	}

	if (level < impl_level)
	{
		return 0;
	}

	va_start(variable_args.data, message);

	record_ctor.line = line;
	record_ctor.func = func;
	record_ctor.file = file;
	record_ctor.level = level;
	record_ctor.message = message;
	record_ctor.variable_args = &variable_args;

	result = log_impl_write(impl, &record_ctor);

	va_end(variable_args.data);

	return result;
}

int log_clear(const char *name)
{
	log_impl impl = log_singleton_get(name);

	if (impl == NULL)
	{
		return 1;
	}

	return log_impl_clear(impl);
}

int log_delete(const char *name)
{
	log_impl impl = log_singleton_remove(name);

	if (impl == NULL)
	{
		return 1;
	}

	return log_impl_destroy(impl);
}

const char *log_print_info(void)
{
	static const char log_info[] =
		"Logger Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

#ifdef LOG_STATIC_DEFINE
		"Compiled as static library type\n"
#else
		"Compiled as shared library type\n"
#endif

		"\n";

	return log_info;
}
