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

#include <adt/adt_set.h>

#include <reflect/reflect_object.h>
#include <reflect/reflect_value_type.h>

#include <reflect/reflect_accessor.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct object_type
{
	char *name;
	enum accessor_type_id accessor;
	object_impl impl;
	object_interface interface;
	size_t ref_count;
	klass cls;
};

static struct
{
	uint64_t allocations;
	uint64_t deallocations;
	uint64_t increments;
	uint64_t decrements;
} object_stats = { 0, 0, 0, 0 };

static value object_metadata_name(object obj);

object object_create(const char *name, enum accessor_type_id accessor, object_impl impl, object_impl_interface_singleton singleton, klass cls)
{
	object obj = malloc(sizeof(struct object_type));

	if (obj == NULL)
	{
		return NULL;
	}

	if (name != NULL)
	{
		size_t obj_name_size = strlen(name) + 1;

		obj->name = malloc(sizeof(char) * obj_name_size);

		if (obj->name == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid object name allocation <%s>", name);

			free(obj);

			return NULL;
		}

		memcpy(obj->name, name, obj_name_size);
	}
	else
	{
		obj->name = NULL;
	}

	obj->impl = impl;
	obj->accessor = accessor;
	obj->ref_count = 0;
	obj->interface = singleton ? singleton() : NULL;

	obj->cls = cls;

	if (obj->interface != NULL && obj->interface->create != NULL)
	{
		if (obj->interface->create(obj, impl) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid object (%s) create callback <%p>", obj->name, obj->interface->create);

			free(obj->name);
			free(obj);

			return NULL;
		}
	}

	++object_stats.allocations;

	return obj;
}

int object_increment_reference(object obj)
{
	if (obj == NULL)
	{
		return 1;
	}

	if (obj->ref_count == SIZE_MAX)
	{
		return 1;
	}

	++obj->ref_count;
	++object_stats.increments;

	return 0;
}

int object_decrement_reference(object obj)
{
	if (obj == NULL)
	{
		return 1;
	}

	if (obj->ref_count == 0)
	{
		return 1;
	}

	--obj->ref_count;
	++object_stats.decrements;

	return 0;
}

object_impl object_impl_get(object obj)
{
	return obj->impl;
}

vector object_methods(object obj, const char *key)
{
	if (obj == NULL || key == NULL)
	{
		return NULL;
	}

	return class_methods(obj->cls, key);
}

method object_method(object obj, const char *key, type_id ret, type_id args[], size_t size)
{
	if (obj == NULL || key == NULL)
	{
		return NULL;
	}

	return class_method(obj->cls, key, ret, args, size);
}

const char *object_name(object obj)
{
	if (obj != NULL)
	{
		return obj->name;
	}

	return NULL;
}

value object_metadata_name(object obj)
{
	static const char object_str[] = "name";

	value name = value_create_array(NULL, 2);

	value *name_array;

	if (name == NULL)
	{
		return NULL;
	}

	name_array = value_to_array(name);

	name_array[0] = value_create_string(object_str, sizeof(object_str) - 1);

	if (name_array[0] == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	name_array[1] = value_create_string(obj->name, strlen(obj->name));

	if (name_array[1] == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	return name;
}

value object_metadata(object obj)
{
	value name, f;

	value *f_map;

	/* Create object name array */
	name = object_metadata_name(obj);

	if (name == NULL)
	{
		return NULL;
	}

	/* Create object map (name) */
	f = value_create_map(NULL, 1);

	if (f == NULL)
	{
		value_type_destroy(name);

		return NULL;
	}

	f_map = value_to_map(f);

	f_map[0] = name;

	return f;
}

value object_get(object obj, const char *key)
{
	if (obj != NULL && obj->interface != NULL && obj->interface->get != NULL)
	{
		struct accessor_type accessor;
		attribute attr = class_attribute(obj->cls, key);

		if (attr == NULL)
		{
			switch (obj->accessor)
			{
				case ACCESSOR_TYPE_STATIC: {
					log_write("metacall", LOG_LEVEL_ERROR, "Attribute %s in object %s is not defined", key, obj->name);
					return NULL;
				}

				case ACCESSOR_TYPE_DYNAMIC: {
					accessor.data.key = key;
				}
			}

			accessor.id = ACCESSOR_TYPE_DYNAMIC;
		}
		else
		{
			accessor.data.attr = attr;
			accessor.id = ACCESSOR_TYPE_STATIC;
		}

		value v = obj->interface->get(obj, obj->impl, &accessor);

		if (v == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid object %s get of attribute %s", obj->name, key);
		}

		return v;
	}

	return NULL;
}

int object_set(object obj, const char *key, value v)
{
	if (obj != NULL && obj->interface != NULL && obj->interface->set != NULL)
	{
		struct accessor_type accessor;
		attribute attr = class_attribute(obj->cls, key);

		if (attr == NULL)
		{
			switch (obj->accessor)
			{
				case ACCESSOR_TYPE_STATIC: {
					log_write("metacall", LOG_LEVEL_ERROR, "Attribute %s in object %s is not defined", key, obj->name);
					return 3;
				}

				case ACCESSOR_TYPE_DYNAMIC: {
					accessor.data.key = key;
				}
			}

			accessor.id = ACCESSOR_TYPE_DYNAMIC;
		}
		else
		{
			accessor.data.attr = attr;
			accessor.id = ACCESSOR_TYPE_STATIC;
		}

		if (obj->interface->set(obj, obj->impl, &accessor, v) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid class %s set of static attribute %s", obj->name, key);
			return 2;
		}

		return 0;
	}

	return 1;
}

value object_call(object obj, method m, object_args args, size_t argc)
{
	if (obj != NULL && obj->interface != NULL && obj->interface->method_invoke != NULL)
	{
		value v = obj->interface->method_invoke(obj, obj->impl, m, args, argc);

		if (v == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid object %s invoke of method %s", obj->name, method_name(m));
			return NULL;
		}

		return v;
	}

	return NULL;
}

value object_await(object obj, method m, object_args args, size_t size, object_resolve_callback resolve_callback, object_reject_callback reject_callback, void *context)
{
	if (obj != NULL && obj->interface != NULL && obj->interface->method_await != NULL)
	{
		value v = obj->interface->method_await(obj, obj->impl, m, args, size, resolve_callback, reject_callback, context);

		if (v == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid object %s await of method %s", obj->name, method_name(m));

			return NULL;
		}

		return v;
	}

	return NULL;
}

int object_delete(object obj)
{
	if (obj != NULL && obj->interface != NULL && obj->interface->destructor != NULL)
	{
		int error = obj->interface->destructor(obj, obj->impl);

		if (error != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid object %s destructor", obj->name);

			return 2;
		}

		return 0;
	}

	return 1;
}

void object_stats_debug(void)
{
#if !(!defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(__DEBUG) || defined(__DEBUG__))
	if (object_stats.allocations != object_stats.deallocations || object_stats.increments != object_stats.decrements)
#endif
	{
		printf("----------------- OBJECTS -----------------\n");
		printf("Allocations: %" PRIuS "\n", object_stats.allocations);
		printf("Deallocations: %" PRIuS "\n", object_stats.deallocations);
		printf("Increments: %" PRIuS "\n", object_stats.increments);
		printf("Decrements: %" PRIuS "\n", object_stats.decrements);
		fflush(stdout);
	}
}

void object_destroy(object obj)
{
	if (obj != NULL)
	{
		if (object_decrement_reference(obj) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid reference counter in object: %s", obj->name ? obj->name : "<anonymous>");
		}

		if (obj->ref_count == 0)
		{
			if (obj->name == NULL)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy anonymous object <%p>", (void *)obj);
			}
			else
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy object %s <%p>", obj->name, (void *)obj);
			}

			if (obj->interface != NULL && obj->interface->destroy != NULL)
			{
				obj->interface->destroy(obj, obj->impl);
			}

			if (obj->name != NULL)
			{
				free(obj->name);
			}

			free(obj);

			++object_stats.deallocations;
		}
	}
}
