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

#include <log/log.h>

#include <reflect/reflect_method.h>

#include <stdlib.h>
#include <string.h>

struct method_type
{
	klass cls;
	char *name;
	signature s;
	method_impl impl;
	enum class_visibility_id visibility;
	enum async_id async;
	method_interface iface;
};

method method_create(klass cls, const char *name, size_t args_count, method_impl impl, enum class_visibility_id visibility, enum async_id async, method_impl_interface_singleton singleton)
{
	method m = malloc(sizeof(struct method_type));

	if (m == NULL)
	{
		return NULL;
	}

	if (name != NULL)
	{
		size_t size = strlen(name) + 1;

		m->name = malloc(sizeof(char) * size);

		if (m->name != NULL)
		{
			memcpy(m->name, name, size);
		}
	}
	else
	{
		m->name = NULL;
	}

	m->s = signature_create(args_count);

	if (m->s == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid method signature allocation");

		if (m->name)
		{
			free(m->name);
		}

		free(m);

		return NULL;
	}

	m->cls = cls;
	m->impl = impl;
	m->visibility = visibility;
	m->async = async;
	m->iface = singleton ? singleton() : NULL;

	return m;
}

klass method_class(method m)
{
	return m->cls;
}

char *method_name(method m)
{
	return m->name;
}

signature method_signature(method m)
{
	return m->s;
}

method_impl method_data(method m)
{
	return m->impl;
}

enum class_visibility_id method_visibility(method m)
{
	return m->visibility;
}

enum async_id method_async(method m)
{
	return m->async;
}

void method_destroy(method m)
{
	if (m)
	{
		if (m->iface && m->iface->destroy)
		{
			m->iface->destroy(m, m->impl);
		}

		if (m->name)
		{
			free(m->name);
		}

		free(m);
	}
}
