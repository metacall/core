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

#include <adt/adt_map.h>
#include <adt/adt_set.h>
#include <adt/adt_vector.h>

#include <reflect/reflect_class.h>
#include <reflect/reflect_value_type.h>

#include <reflect/reflect_accessor.h>

#include <threading/threading_atomic_ref_count.h>

#include <reflect/reflect_memory_tracker.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct class_type
{
	char *name;
	enum accessor_type_id accessor;
	class_impl impl;
	class_interface interface;
	threading_atomic_ref_count_type ref;
	vector constructors;
	map methods;
	map static_methods;
	set attributes;
	set static_attributes;
};

struct class_metadata_iterator_args_type
{
	value v;
	size_t count;
};

typedef struct class_metadata_iterator_args_type *class_metadata_iterator_args;

reflect_memory_tracker(class_stats);

static value class_metadata_name(klass cls);
static value class_metadata_constructors(klass cls);
static int class_metadata_methods_impl_cb_iterate(map m, map_key key, map_value val, map_cb_iterate_args args);
static value class_metadata_methods_impl(const char name[], size_t size, map methods);
static value class_metadata_methods(klass cls);
static value class_metadata_static_methods(klass cls);
static int class_metadata_attributes_impl_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);
static value class_metadata_attributes_impl(const char name[], size_t size, set attributes);
static value class_metadata_attributes(klass cls);
static value class_metadata_static_attributes(klass cls);
static method class_get_method_type_safe(vector v, type_id ret, type_id args[], size_t size);
static int class_attributes_destroy_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);
static int class_methods_destroy_cb_iterate(map m, map_key key, map_value val, map_cb_iterate_args args);
static void class_constructors_destroy(klass cls);

klass class_create(const char *name, enum accessor_type_id accessor, class_impl impl, class_impl_interface_singleton singleton)
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
	cls->accessor = accessor;
	threading_atomic_ref_count_initialize(&cls->ref);
	cls->interface = singleton ? singleton() : NULL;
	cls->constructors = vector_create_type(constructor);
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
			vector_destroy(cls->constructors);
			map_destroy(cls->methods);
			map_destroy(cls->static_methods);
			set_destroy(cls->attributes);
			set_destroy(cls->static_attributes);
			free(cls);

			return NULL;
		}
	}

	reflect_memory_tracker_allocation(class_stats);

	return cls;
}

int class_increment_reference(klass cls)
{
	if (cls == NULL)
	{
		return 1;
	}

	if (threading_atomic_ref_count_increment(&cls->ref) == 1)
	{
		return 1;
	}

	reflect_memory_tracker_increment(class_stats);

	return 0;
}

int class_decrement_reference(klass cls)
{
	if (cls == NULL)
	{
		return 1;
	}

	if (threading_atomic_ref_count_decrement(&cls->ref) == 1)
	{
		return 1;
	}

	reflect_memory_tracker_decrement(class_stats);

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

value class_metadata_constructors(klass cls)
{
	static const char constructors_str[] = "constructors";
	size_t iterator, size = vector_size(cls->constructors);
	value *v_array, v = value_create_array(NULL, 2);
	value *ctors_array;

	if (v == NULL)
	{
		return NULL;
	}

	v_array = value_to_array(v);
	v_array[0] = value_create_string(constructors_str, sizeof(constructors_str) - 1);

	if (v_array[0] == NULL)
	{
		value_type_destroy(v);
		return NULL;
	}

	v_array[1] = value_create_array(NULL, size);

	if (v_array[1] == NULL)
	{
		value_type_destroy(v);
		return NULL;
	}

	ctors_array = value_to_array(v_array[1]);

	for (iterator = 0; iterator < size; ++iterator)
	{
		ctors_array[iterator] = constructor_metadata(vector_at_type(cls->constructors, iterator, constructor));
	}

	return v;
}

int class_metadata_methods_impl_cb_iterate(map m, map_key key, map_value val, map_cb_iterate_args args)
{
	class_metadata_iterator_args iterator = (class_metadata_iterator_args)args;
	value *v_array = value_to_array(iterator->v);

	(void)m;
	(void)key;

	v_array[iterator->count++] = method_metadata((method)val);

	return 0;
}

value class_metadata_methods_impl(const char name[], size_t size, map methods)
{
	value v = value_create_array(NULL, 2);
	value *v_array;

	if (v == NULL)
	{
		return NULL;
	}

	v_array = value_to_array(v);
	v_array[0] = value_create_string(name, size - 1);

	if (v_array[0] == NULL)
	{
		goto error_value;
	}

	v_array[1] = value_create_array(NULL, map_size(methods));

	if (v_array[1] == NULL)
	{
		goto error_value;
	}

	struct class_metadata_iterator_args_type iterator;

	iterator.v = v_array[1];
	iterator.count = 0;

	map_iterate(methods, &class_metadata_methods_impl_cb_iterate, &iterator);

	return v;
error_value:
	value_type_destroy(v);
	return NULL;
}

value class_metadata_methods(klass cls)
{
	static const char name[] = "methods";
	return class_metadata_methods_impl(name, sizeof(name), cls->methods);
}

value class_metadata_static_methods(klass cls)
{
	static const char name[] = "static_methods";
	return class_metadata_methods_impl(name, sizeof(name), cls->static_methods);
}

int class_metadata_attributes_impl_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	class_metadata_iterator_args iterator = (class_metadata_iterator_args)args;
	value *v_array = value_to_array(iterator->v);

	(void)s;
	(void)key;

	v_array[iterator->count++] = attribute_metadata((attribute)val);

	return 0;
}

value class_metadata_attributes_impl(const char name[], size_t size, set attributes)
{
	value v = value_create_array(NULL, 2);
	value *v_array;

	if (v == NULL)
	{
		return NULL;
	}

	v_array = value_to_array(v);
	v_array[0] = value_create_string(name, size - 1);

	if (v_array[0] == NULL)
	{
		goto error_value;
	}

	v_array[1] = value_create_array(NULL, set_size(attributes));

	if (v_array[1] == NULL)
	{
		goto error_value;
	}

	struct class_metadata_iterator_args_type iterator;

	iterator.v = v_array[1];
	iterator.count = 0;

	set_iterate(attributes, &class_metadata_attributes_impl_cb_iterate, &iterator);

	return v;
error_value:
	value_type_destroy(v);
	return NULL;
}

value class_metadata_attributes(klass cls)
{
	static const char name[] = "attributes";
	return class_metadata_attributes_impl(name, sizeof(name), cls->attributes);
}

value class_metadata_static_attributes(klass cls)
{
	static const char name[] = "static_attributes";
	return class_metadata_attributes_impl(name, sizeof(name), cls->static_attributes);
}

value class_metadata(klass cls)
{
	/* The structure of the metadata is:
	* {
	*	name: "ClassName",
	*	constructors: [{}],
	*	methods: [{}],
	*	static_methods: [{}],
	*	attributes: {
	*		"attr1": {}
	*	},
	*	static_attributes: {
	*		"static_attr1": {}
	*	}
	* }
	*/

	/* Create class map (name + constructors + methods + static_methods + attributes + static_attributes) */
	value *c_map, c = value_create_map(NULL, 6);

	if (c == NULL)
	{
		return NULL;
	}

	c_map = value_to_map(c);

	/* Create class name array */
	c_map[0] = class_metadata_name(cls);

	if (c_map[0] == NULL)
	{
		goto error_value;
	}

	/* Create constructors array */
	c_map[1] = class_metadata_constructors(cls);

	if (c_map[1] == NULL)
	{
		goto error_value;
	}

	/* Create class methods array */
	c_map[2] = class_metadata_methods(cls);

	if (c_map[2] == NULL)
	{
		goto error_value;
	}

	/* Create class static_methods array */
	c_map[3] = class_metadata_static_methods(cls);

	if (c_map[3] == NULL)
	{
		goto error_value;
	}

	/* Create class attributes array */
	c_map[4] = class_metadata_attributes(cls);

	if (c_map[4] == NULL)
	{
		goto error_value;
	}

	/* Create class static_attributes array */
	c_map[5] = class_metadata_static_attributes(cls);

	if (c_map[5] == NULL)
	{
		goto error_value;
	}

	return c;

error_value:
	value_type_destroy(c);
	return NULL;
}

object class_new(klass cls, const char *name, constructor ctor, class_args args, size_t argc)
{
	if (cls != NULL && cls->interface != NULL && cls->interface->constructor != NULL)
	{
		object obj = cls->interface->constructor(cls, cls->impl, name, ctor, args, argc);

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
		struct accessor_type accessor;
		attribute attr = set_get(cls->static_attributes, (set_key)key);

		if (attr == NULL)
		{
			switch (cls->accessor)
			{
				case ACCESSOR_TYPE_STATIC: {
					log_write("metacall", LOG_LEVEL_ERROR, "Static attribute %s in class %s is not defined", key, cls->name);
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

		value v = cls->interface->static_get(cls, cls->impl, &accessor);

		if (v == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid class %s get of static attribute %s", cls->name, key);
		}

		return v;
	}

	return NULL;
}

int class_static_set(klass cls, const char *key, value v)
{
	if (cls != NULL && cls->interface != NULL && cls->interface->static_set != NULL && key != NULL && v != NULL)
	{
		struct accessor_type accessor;
		attribute attr = set_get(cls->static_attributes, (set_key)key);

		if (attr == NULL)
		{
			switch (cls->accessor)
			{
				case ACCESSOR_TYPE_STATIC: {
					log_write("metacall", LOG_LEVEL_ERROR, "Static attribute %s in class %s is not defined", key, cls->name);
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

		if (cls->interface->static_set(cls, cls->impl, &accessor, v) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid class %s set of static attribute %s", cls->name, key);
			return 2;
		}

		return 0;
	}

	return 1;
}

vector class_constructors(klass cls)
{
	if (cls == NULL)
	{
		return NULL;
	}

	return cls->constructors;
}

constructor class_default_constructor(klass cls)
{
	if (cls == NULL)
	{
		return NULL;
	}

	return (constructor)vector_at(cls->constructors, 0);
}

constructor class_constructor(klass cls, type_id args[], size_t size)
{
	/* This method tries to find a valid constructor with correct types,
	* if this cannot be achieved, we return the default constructor and
	* we let the loader handle the new invokation with variadic arguments */
	if (cls == NULL)
	{
		return NULL;
	}

	size_t iterator, constructor_size = vector_size(cls->constructors);

	if (constructor_size == 0)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Class %s does not have any constructor", cls->name);
		return NULL;
	}
	else if (constructor_size == 1)
	{
		return vector_at_type(cls->constructors, 0, constructor);
	}

	for (iterator = 0; iterator < constructor_size; ++iterator)
	{
		constructor ctor = vector_at_type(cls->constructors, iterator, constructor);

		if (constructor_compare(ctor, args, size) == 0)
		{
			return ctor;
		}
	}

	/* Return default constructor if there is not a match */
	return vector_at_type(cls->constructors, 0, constructor);
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

method class_get_method_type_safe(vector v, type_id ret, type_id args[], size_t size)
{
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

method class_static_method(klass cls, const char *key, type_id ret, type_id args[], size_t size)
{
	return class_get_method_type_safe(class_static_methods(cls, key), ret, args, size);
}

method class_method(klass cls, const char *key, type_id ret, type_id args[], size_t size)
{
	return class_get_method_type_safe(class_methods(cls, key), ret, args, size);
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

int class_register_constructor(klass cls, constructor ctor)
{
	if (cls == NULL || ctor == NULL)
	{
		return 1;
	}

	vector_push_back(cls->constructors, &ctor);

	return 0;
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
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid class %s invoke of static method %s", cls->name, method_name(m));

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
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid class %s await of method %s", cls->name, method_name(m));

			return NULL;
		}

		return v;
	}

	return NULL;
}

int class_attributes_destroy_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
	(void)key;
	(void)args;

	if (val != NULL)
	{
		attribute attr = val;

		attribute_destroy(attr);
	}

	return 0;
}

int class_methods_destroy_cb_iterate(map m, map_key key, map_value val, map_cb_iterate_args args)
{
	(void)m;
	(void)key;
	(void)args;

	method_destroy((method)val);

	return 0;
}

void class_constructors_destroy(klass cls)
{
	size_t iterator, size = vector_size(cls->constructors);

	for (iterator = 0; iterator < size; ++iterator)
	{
		constructor ctor = vector_at_type(cls->constructors, iterator, constructor);

		constructor_destroy(ctor);
	}
}

void class_stats_debug(void)
{
	reflect_memory_tracker_print(class_stats, "CLASSES");
}

void class_destroy(klass cls)
{
	if (cls != NULL)
	{
		if (class_decrement_reference(cls) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid reference counter in class: %s", cls->name ? cls->name : "<anonymous>");
		}

		if (threading_atomic_ref_count_load(&cls->ref) == 0)
		{
			/* TODO: Disable logs here until log is completely thread safe and async signal safe */

			/*
			if (cls->name == NULL)
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy anonymous class <%p>", (void *)cls);
			}
			else
			{
				log_write("metacall", LOG_LEVEL_DEBUG, "Destroy class %s <%p>", cls->name, (void *)cls);
			}
			*/

			class_constructors_destroy(cls);

			set_iterate(cls->attributes, &class_attributes_destroy_cb_iterate, NULL);
			set_iterate(cls->static_attributes, &class_attributes_destroy_cb_iterate, NULL);

			map_iterate(cls->methods, &class_methods_destroy_cb_iterate, NULL);
			map_iterate(cls->static_methods, &class_methods_destroy_cb_iterate, NULL);

			if (cls->interface != NULL && cls->interface->destroy != NULL)
			{
				cls->interface->destroy(cls, cls->impl);
			}

			if (cls->name != NULL)
			{
				free(cls->name);
			}

			if (cls->constructors != NULL)
			{
				vector_destroy(cls->constructors);
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

			threading_atomic_ref_count_destroy(&cls->ref);

			free(cls);

			reflect_memory_tracker_deallocation(class_stats);
		}
	}
}
