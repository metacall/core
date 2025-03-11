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

#include <log/log.h>

#include <reflect/reflect_method.h>
#include <reflect/reflect_value_type.h>

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

static value method_metadata_name(method m);
static value method_metadata_async(method m);
static value method_metadata_signature(method m);
static value method_metadata_visibility(method m);

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

value method_metadata_name(method m)
{
	static const char m_str[] = "name";

	value name = value_create_array(NULL, 2);

	value *name_array;

	if (name == NULL)
	{
		return NULL;
	}

	name_array = value_to_array(name);

	name_array[0] = value_create_string(m_str, sizeof(m_str) - 1);

	if (name_array[0] == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	name_array[1] = value_create_string(m->name, strlen(m->name));

	if (name_array[1] == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	return name;
}

value method_metadata_async(method m)
{
	static const char async_str[] = "async";

	value async = value_create_array(NULL, 2);

	value *async_array;

	if (async == NULL)
	{
		return NULL;
	}

	async_array = value_to_array(async);

	async_array[0] = value_create_string(async_str, sizeof(async_str) - 1);

	if (async_array[0] == NULL)
	{
		value_type_destroy(async);

		return NULL;
	}

	async_array[1] = value_create_bool(m->async == SYNCHRONOUS ? 0L : 1L);

	if (async_array[1] == NULL)
	{
		value_type_destroy(async);

		return NULL;
	}

	return async;
}

value method_metadata_signature(method m)
{
	static const char sig_str[] = "signature";

	value sig = value_create_array(NULL, 2);

	value *sig_array;

	if (sig == NULL)
	{
		return NULL;
	}

	sig_array = value_to_array(sig);

	sig_array[0] = value_create_string(sig_str, sizeof(sig_str) - 1);

	if (sig_array[0] == NULL)
	{
		value_type_destroy(sig);

		return NULL;
	}

	sig_array[1] = signature_metadata(m->s);

	if (sig_array[1] == NULL)
	{
		value_type_destroy(sig);

		return NULL;
	}

	return sig;
}

value method_metadata_visibility(method m)
{
	return class_visibility_value_pair(m->visibility);
}

value method_metadata(method m)
{
	value name, sig, async, visibility, metatada;
	value *m_map;

	/* Create method name array */
	name = method_metadata_name(m);

	if (name == NULL)
	{
		goto error_name;
	}

	/* Create signature array */
	sig = method_metadata_signature(m);

	if (sig == NULL)
	{
		goto error_signature;
	}

	/* Create method async array */
	async = method_metadata_async(m);

	if (async == NULL)
	{
		goto error_async;
	}

	/* Create method visibility array */
	visibility = method_metadata_visibility(m);

	if (visibility == NULL)
	{
		goto error_visibility;
	}

	/* Create method map (name + signature + async + visibility) */
	metatada = value_create_map(NULL, 4);

	if (metatada == NULL)
	{
		goto error_method;
	}

	m_map = value_to_map(metatada);

	m_map[0] = name;
	m_map[1] = sig;
	m_map[2] = async;
	m_map[3] = visibility;

	return metatada;

error_method:
	value_type_destroy(visibility);
error_visibility:
	value_type_destroy(async);
error_async:
	value_type_destroy(sig);
error_signature:
	value_type_destroy(name);
error_name:
	return NULL;
}

void method_destroy(method m)
{
	if (m)
	{
		if (m->iface && m->iface->destroy)
		{
			m->iface->destroy(m, m->impl);
		}

		if (m->s)
		{
			signature_destroy(m->s);
		}

		if (m->name)
		{
			free(m->name);
		}

		free(m);
	}
}
