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

#include <reflect/reflect_attribute.h>

#include <stdlib.h>
#include <string.h>

struct attribute_type
{
	klass cls;
	char *name;
	type t;
	attribute_impl impl;
	enum class_visibility_id visibility;
};

attribute attribute_create(klass cls, const char *name, type t, attribute_impl impl, enum class_visibility_id visibility)
{
	attribute attr = malloc(sizeof(struct attribute_type));

	if (attr == NULL)
	{
		return NULL;
	}

	if (name != NULL)
	{
		size_t size = strlen(name) + 1;

		attr->name = malloc(sizeof(char) * size);

		if (attr->name != NULL)
		{
			memcpy(attr->name, name, size);
		}
	}
	else
	{
		attr->name = NULL;
	}

	attr->cls = cls;
	attr->t = t;
	attr->impl = impl;
	attr->visibility = visibility;

	return attr;
}

klass attribute_class(attribute attr)
{
	return attr->cls;
}

char *attribute_name(attribute attr)
{
	return attr->name;
}

type attribute_type(attribute attr)
{
	return attr->t;
}

attribute_impl attribute_data(attribute attr)
{
	return attr->impl;
}

enum class_visibility_id attribute_visibility(attribute attr)
{
	return attr->visibility;
}

void attribute_destroy(attribute attr)
{
	if (attr)
	{
		if (attr->name)
		{
			free(attr->name);
		}

		free(attr);
	}
}
