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

#include <adt/adt_map.h>
#include <adt/adt_set.h>

#include <reflect/reflect_class.h>
#include <reflect/reflect_value_type.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct class_type
{
	char *name;
	class_impl impl;
	class_interface interface;
	size_t ref_count;
	set static_attributes;
	set attributes;
	map static_methods;
	map methods;
};

static value class_metadata_name(klass cls);

klass class_create(const char *name, class_impl impl, class_impl_interface_singleton singleton)
{
	klass cls = malloc(sizeof(struct class_type));

	if (cls == NULL)
	{
		return NULL;
	}

	if (name != NULL)
	{
		size_t cls_name_size = strlen(name) + 1;

		cls->name = malloc(sizeof(char) * cls_name_size);

		if (cls->name == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid class name allocation <%s>", name);

			free(cls);

			return NULL;
		}

		memcpy(cls->name, name, cls_name_size);
	}
	else
	{
		cls->name = NULL;
	}

	cls->impl = impl;
	cls->ref_count = 0;
	cls->interface = singleton ? singleton() : NULL;
	cls->methods = map_create(&hash_callback_str, &comparable_callback_str);
	cls->static_methods = map_create(&hash_callback_str, &comparable_callback_str);
	cls->attributes = set_create(&hash_callback_str, &comparable_callback_str);
	cls->static_attributes = set_create(&hash_callback_str, &comparable_callback_str);

	if (cls->interface != NULL && cls->interface->create != NULL)
	{
		if (cls->interface->create(cls, impl) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid class (%s) create callback <%p>", cls->name, cls->interface->create);

			free(cls->name);
			map_destroy(cls->methods);
			map_destroy(cls->static_methods);
			set_destroy(cls->attributes);
			set_destroy(cls->static_attributes);
			free(cls);

			return NULL;
		}
	}

	return cls;
}

int class_increment_reference(klass cls)
{
	if (cls == NULL)
	{
		return 1;
	}

	if (cls->ref_count == SIZE_MAX)
	{
		return 1;
	}

	++cls->ref_count;

	return 0;
}

int class_decrement_reference(klass cls)
{
	if (cls == NULL)
	{
		return 1;
	}

	if (cls->ref_count == 0)
	{
		return 1;
	}

	--cls->ref_count;

	return 0;
}

class_impl class_impl_get(klass cls)
{
	return cls->impl;
}

const char *class_name(klass cls)
{
	if (cls != NULL)
	{
		return cls->name;
	}

	return NULL;
}

value class_metadata_name(klass cls)
{
	static const char class_str[] = "name";

	value name = value_create_array(NULL, 2);

	value *name_array;

	if (name == NULL)
	{
		return NULL;
	}

	name_array = value_to_array(name);

	name_array[0] = value_create_string(class_str, sizeof(class_str) - 1);

	if (name_array[0] == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	name_array[1] = value_create_string(cls->name, strlen(cls->name));

	if (name_array[1] == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	return name;
}

value class_metadata(klass cls)
{
	// TODO: Implement inspect of (non-)static methods / attributes
	(void)cls;

	value name, c;

	value *c_map;

	/* Create class name array */
	name = class_metadata_name(cls);

	if (name == NULL)
	{
		return NULL;
	}

	/* Create class map (name) */
	c = value_create_map(NULL, 1);

	if (c == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	c_map = value_to_map(c);

	c_map[0] = name;

	return c;
}

object class_new(klass cls, const char *name, class_args args, size_t argc)
{
	if (cls != NULL && cls->interface != NULL && cls->interface->constructor != NULL)
	{
		object obj = cls->interface->constructor(cls, cls->impl, name, args, argc);

		if (obj == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid class (%s) constructor <%p>", cls->name, cls->interface->static_set);

			return NULL;
		}

		return obj;
	}

	return NULL;
}

value class_static_get(klass cls, const char *key)
{
	if (cls != NULL && cls->interface != NULL && cls->interface->static_get != NULL && key != NULL)
	{
		attribute attr = set_get(cls->static_attributes, (set_key)key);

		if (attr != NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Attribute %s in class %s is not defined", key, cls->name);
			return NULL;
		}

		value v = cls->interface->static_get(cls, cls->impl, attr);

		if (v == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid class (%s) static_get callback <%p>", cls->name, cls->interface->static_get);
		}

		return v;
	}

	return NULL;
}

int class_static_set(klass cls, const char *key, value v)
{
	if (cls != NULL && cls->interface != NULL && cls->interface->static_set != NULL && key != NULL && v != NULL)
	{
		attribute attr = set_get(cls->static_attributes, (set_key)key);

		if (attr != NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Attribute %s in class %s is not defined", key, cls->name);
			return 3;
		}

		if (cls->interface->static_set(cls, cls->impl, attr, v) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid class (%s) static_set callback <%p>", cls->name, cls->interface->static_set);
			return 2;
		}

		return 0;
	}

	return 1;
}

vector class_static_methods(klass cls, const char *key)
{
	if (cls == NULL || key == NULL)
	{
		return NULL;
	}

	return map_get(cls->static_methods, (map_key)key);
}

vector class_methods(klass cls, const char *key)
{
	if (cls == NULL || key == NULL)
	{
		return NULL;
	}

	return map_get(cls->methods, (map_key)key);
}

method class_static_method(klass cls, const char *key, type_id ret, type_id args[], size_t size)
{
	vector v = class_static_methods(cls, key);

	if (v != NULL)
	{
		size_t iterator, method_size = vector_size(v);

		for (iterator = 0; iterator < method_size; ++iterator)
		{
			method m = vector_at_type(v, iterator, method);

			if (signature_compare(method_signature(m), ret, args, size) == 0)
			{
				vector_destroy(v);
				return m;
			}
		}

		vector_destroy(v);
	}

	return NULL;
}

method class_method(klass cls, const char *key, type_id ret, type_id args[], size_t size)
{
	vector v = class_methods(cls, key);

	if (v != NULL)
	{
		size_t iterator, method_size = vector_size(v);

		for (iterator = 0; iterator < method_size; ++iterator)
		{
			method m = vector_at_type(v, iterator, method);

			if (signature_compare(method_signature(m), ret, args, size) == 0)
			{
				vector_destroy(v);
				return m;
			}
		}

		vector_destroy(v);
	}

	return NULL;
}

attribute class_static_attribute(klass cls, const char *key)
{
	if (cls == NULL || key == NULL)
	{
		return NULL;
	}

	return set_get(cls->static_attributes, (set_key)key);
}

attribute class_attribute(klass cls, const char *key)
{
	if (cls == NULL || key == NULL)
	{
		return NULL;
	}

	return set_get(cls->attributes, (set_key)key);
}

int class_register_static_method(klass cls, method m)
{
	if (cls == NULL || m == NULL)
	{
		return 1;
	}

	return map_insert(cls->static_methods, (map_key)method_name(m), m);
}

int class_register_method(klass cls, method m)
{
	if (cls == NULL || m == NULL)
	{
		return 1;
	}

	return map_insert(cls->methods, (map_key)method_name(m), m);
}

int class_register_static_attribute(klass cls, attribute attr)
{
	if (cls == NULL || attr == NULL)
	{
		return 1;
	}

	return set_insert(cls->static_attributes, (set_key)attribute_name(attr), attr);
}

int class_register_attribute(klass cls, attribute attr)
{
	if (cls == NULL || attr == NULL)
	{
		return 1;
	}

	return set_insert(cls->attributes, (set_key)attribute_name(attr), attr);
}

value class_static_call(klass cls, method m, class_args args, size_t argc)
{
	if (cls != NULL && cls->interface != NULL && cls->interface->static_invoke != NULL && m != NULL)
	{
		value v = cls->interface->static_invoke(cls, cls->impl, m, args, argc);

		if (v == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid class (%s) static_invoke callback (%s) <%p>", cls->name, method_name(m), cls->interface->static_invoke);

			return NULL;
		}

		return v;
	}

	return NULL;
}

value class_static_await(klass cls, method m, class_args args, size_t size, class_resolve_callback resolve_callback, class_reject_callback reject_callback, void *context)
{
	if (cls != NULL && cls->interface != NULL && cls->interface->static_invoke != NULL && m != NULL)
	{
		value v = cls->interface->static_await(cls, cls->impl, m, args, size, resolve_callback, reject_callback, context);

		if (v == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid class (%s) static_await callback (%s) <%p>", cls->name, method_name(m), cls->interface->static_await);

			return NULL;
		}

		return v;
	}

	return NULL;
}

void class_destroy(klass cls)
{
	if (cls != NULL)
	{
		if (class_decrement_reference(cls) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid reference counter in class: %s", cls->name ? cls->name : "<anonymous>");
		}

		if (cls->ref_count == 0)
		{
			if (cls->name == NULL)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy anonymous class <%p>", (void *)cls);
			}
			else
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy class %s <%p>", cls->name, (void *)cls);
			}

			if (cls->interface != NULL && cls->interface->destroy != NULL)
			{
				cls->interface->destroy(cls, cls->impl);
			}

			if (cls->name != NULL)
			{
				free(cls->name);
			}

			if (cls->methods != NULL)
			{
				map_destroy(cls->methods);
			}

			if (cls->static_methods != NULL)
			{
				map_destroy(cls->static_methods);
			}

			if (cls->attributes != NULL)
			{
				set_destroy(cls->attributes);
			}

			if (cls->static_attributes != NULL)
			{
				set_destroy(cls->static_attributes);
			}

			free(cls);
		}
	}
}
