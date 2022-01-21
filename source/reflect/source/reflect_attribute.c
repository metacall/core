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

#include <reflect/reflect_attribute.h>

#include <reflect/reflect_value_type.h>

#include <stdlib.h>
#include <string.h>

struct attribute_type
{
	klass cls;
	char *name;
	type t;
	attribute_impl impl;
	enum class_visibility_id visibility;
	attribute_interface iface;
};

static value attribute_metadata_name(attribute attr);
static value attribute_metadata_visibility(attribute attr);

attribute attribute_create(klass cls, const char *name, type t, attribute_impl impl, enum class_visibility_id visibility, attribute_impl_interface_singleton singleton)
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
	attr->iface = singleton ? singleton() : NULL;

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

value attribute_metadata_name(attribute attr)
{
	static const char name_str[] = "name";
	value *name_array, name = value_create_array(NULL, 2);

	if (name == NULL)
	{
		return NULL;
	}

	name_array = value_to_array(name);
	name_array[0] = value_create_string(name_str, sizeof(name_str) - 1);

	if (name_array[0] == NULL)
	{
		value_type_destroy(name);
		return NULL;
	}

	name_array[1] = value_create_string(attr->name, strlen(attr->name));

	if (name_array[1] == NULL)
	{
		value_type_destroy(name);
		return NULL;
	}

	return name;
}

value attribute_metadata_visibility(attribute attr)
{
	static const char visibility_str[] = "visibility";
	value *visibility_array, visibility = value_create_array(NULL, 2);

	if (visibility == NULL)
	{
		return NULL;
	}

	visibility_array = value_to_array(visibility);
	visibility_array[0] = value_create_string(visibility_str, sizeof(visibility_str) - 1);

	if (visibility_array[0] == NULL)
	{
		value_type_destroy(visibility);
		return NULL;
	}

	visibility_array[1] = class_visibility_value(attr->visibility);

	if (visibility_array[1] == NULL)
	{
		value_type_destroy(visibility);
		return NULL;
	}

	return visibility;
}

value attribute_metadata(attribute attr)
{
	/* The structure of the attribute is:
	 * {
	 *	"name": "attr1",
	 *	"type": { "name": "", "id": 18 },
	 *	"visibility": "public"
	 * }
	 */
	value *v_map, v = value_create_map(NULL, 3);

	if (v == NULL)
	{
		return NULL;
	}

	v_map = value_to_map(v);

	v_map[0] = attribute_metadata_name(attr);

	if (v_map[0] == NULL)
	{
		value_type_destroy(v);
		return NULL;
	}

	v_map[1] = type_metadata(attr->t);

	if (v_map[1] == NULL)
	{
		value_type_destroy(v);
		return NULL;
	}

	v_map[2] = attribute_metadata_visibility(attr);

	if (v_map[2] == NULL)
	{
		value_type_destroy(v);
		return NULL;
	}

	return v;
}

void attribute_destroy(attribute attr)
{
	if (attr)
	{
		if (attr->iface && attr->iface->destroy)
		{
			attr->iface->destroy(attr, attr->impl);
		}

		if (attr->name)
		{
			free(attr->name);
		}

		free(attr);
	}
}
