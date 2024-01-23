/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <reflect/reflect_class.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_value_type.h>

#include <adt/adt_set.h>
#include <adt/adt_vector.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct scope_metadata_array_cb_iterator_type;

struct scope_export_cb_iterator_type;

typedef struct scope_metadata_array_cb_iterator_type *scope_metadata_array_cb_iterator;

typedef struct scope_export_cb_iterator_type *scope_export_cb_iterator;

struct scope_type
{
	char *name;		   /**< Scope name */
	set objects;	   /**< Map of scope objects indexed by name string */
	vector call_stack; /**< Scope call stack */
};

struct scope_metadata_array_cb_iterator_type
{
	value *functions;
	value *classes;
	value *objects;

	size_t functions_size;
	size_t classes_size;
	size_t objects_size;
};

struct scope_export_cb_iterator_type
{
	size_t iterator;
	value *values;
};

static int scope_metadata_array_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static int scope_export_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static int scope_metadata_array(scope sp, value v_array[3]);

static value scope_metadata_name(scope sp);

static int scope_destroy_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

scope scope_create(const char *name)
{
	if (name != NULL)
	{
		scope sp = malloc(sizeof(struct scope_type));

		if (sp != NULL)
		{
			size_t sp_name_size = strlen(name) + 1;

			size_t *call_stack_head = NULL;

			sp->name = malloc(sizeof(char) * sp_name_size);

			if (sp->name == NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Scope create name bad allocation");

				free(sp);

				return NULL;
			}

			memcpy(sp->name, name, sp_name_size);

			sp->objects = set_create(&hash_callback_str, &comparable_callback_str);

			if (sp->objects == NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Scope create map bad allocation");

				free(sp->name);

				free(sp);

				return NULL;
			}

			sp->call_stack = vector_create(sizeof(char));

			if (sp->call_stack == NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Scope create call stack bad allocation");

				set_destroy(sp->objects);

				free(sp->name);

				free(sp);

				return NULL;
			}

			if (vector_resize(sp->call_stack, sizeof(size_t)) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Scope create call stack bad allocation");

				vector_destroy(sp->call_stack);

				set_destroy(sp->objects);

				free(sp->name);

				free(sp);

				return NULL;
			}

			call_stack_head = vector_front(sp->call_stack);

			if (call_stack_head != NULL)
			{
				const size_t head_index = 0;

				memcpy(call_stack_head, &head_index, sizeof(size_t));

				return sp;
			}

			log_write("metacall", LOG_LEVEL_ERROR, "Scope create bad call stack head reference");

			return NULL;
		}

		log_write("metacall", LOG_LEVEL_ERROR, "Scope create bad allocation");

		return NULL;
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Scope create invalid parameters");

	return NULL;
}

size_t scope_size(scope sp)
{
	if (sp != NULL)
	{
		return set_size(sp->objects);
	}

	return 0;
}

int scope_define(scope sp, const char *key, value val)
{
	if (sp != NULL && key != NULL && val != NULL)
	{
		if (set_contains(sp->objects, (set_key)key) == 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Scope failed to define a object with key '%s', this key as already been defined", (char *)key);

			return 1;
		}

		return set_insert(sp->objects, (set_key)key, (set_value)val);
	}

	return 1;
}

int scope_metadata_array_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	scope_metadata_array_cb_iterator metadata_iterator = (scope_metadata_array_cb_iterator)args;

	(void)s;
	(void)key;

	int type_id = value_type_id(val);

	if (type_id == TYPE_FUNCTION)
	{
		metadata_iterator->functions[metadata_iterator->functions_size++] = function_metadata(value_to_function(val));
	}
	else if (type_id == TYPE_CLASS)
	{
		metadata_iterator->classes[metadata_iterator->classes_size++] = class_metadata(value_to_class(val));
	}
	else if (type_id == TYPE_OBJECT)
	{
		metadata_iterator->objects[metadata_iterator->objects_size++] = object_metadata(value_to_object(val));
	}

	return 0;
}

int scope_metadata_array_cb_iterate_counter(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	scope_metadata_array_cb_iterator metadata_iterator = (scope_metadata_array_cb_iterator)args;

	(void)s;
	(void)key;

	type_id id = value_type_id(val);

	if (id == TYPE_FUNCTION)
	{
		metadata_iterator->functions_size++;
	}
	else if (id == TYPE_CLASS)
	{
		metadata_iterator->classes_size++;
	}
	else if (id == TYPE_OBJECT)
	{
		metadata_iterator->objects_size++;
	}

	return 0;
}

int scope_metadata_array(scope sp, value v_array[3])
{
	struct scope_metadata_array_cb_iterator_type metadata_iterator = {
		NULL, NULL, NULL, 0, 0, 0
	};

	set_iterate(sp->objects, &scope_metadata_array_cb_iterate_counter, (set_cb_iterate_args)&metadata_iterator);

	value functions_val = value_create_array(NULL, metadata_iterator.functions_size);

	if (functions_val == NULL)
	{
		return 1;
	}

	metadata_iterator.functions = value_to_array(functions_val);

	value classes_val = value_create_array(NULL, metadata_iterator.classes_size);

	if (classes_val == NULL)
	{
		value_destroy(functions_val);
		return 1;
	}

	metadata_iterator.classes = value_to_array(classes_val);

	value objects_val = value_create_array(NULL, metadata_iterator.objects_size);

	if (objects_val == NULL)
	{
		value_destroy(functions_val);
		value_destroy(classes_val);
		return 1;
	}

	metadata_iterator.objects = value_to_array(objects_val);

	/* Reuse counters to fill the arrays */
	metadata_iterator.classes_size = 0;
	metadata_iterator.functions_size = 0;
	metadata_iterator.objects_size = 0;

	set_iterate(sp->objects, &scope_metadata_array_cb_iterate, (set_cb_iterate_args)&metadata_iterator);

	v_array[0] = functions_val;
	v_array[1] = classes_val;
	v_array[2] = objects_val;

	return 0;
}

value scope_metadata_name(scope sp)
{
	static const char name[] = "name";

	value *v_ptr, v = value_create_array(NULL, 2);

	if (v == NULL)
	{
		return NULL;
	}

	v_ptr = value_to_array(v);

	v_ptr[0] = value_create_string(name, sizeof(name) - 1);

	if (v_ptr[0] == NULL)
	{
		value_type_destroy(v);
	}

	v_ptr[1] = value_create_string(sp->name, strlen(sp->name));

	if (v_ptr[1] == NULL)
	{
		value_type_destroy(v);
	}

	return v;
}

value scope_metadata(scope sp)
{
	value *v_map, v = value_create_map(NULL, 4);
	value v_array[3] = { NULL, NULL, NULL }; // 0: funcs, 1: cls, 2: obj

	if (v == NULL)
	{
		return NULL;
	}

	v_map = value_to_map(v);

	v_map[0] = scope_metadata_name(sp);

	if (v_map[0] == NULL)
	{
		value_type_destroy(v);
		return NULL;
	}

	/* Obtain all scope objects of each type (functions, classes and objects) */
	if (scope_metadata_array(sp, v_array) != 0)
	{
		value_type_destroy(v);
		return NULL;
	}

	/* Functions */
	static const char funcs[] = "funcs";
	value *v_funcs_ptr, v_funcs = value_create_array(NULL, 2);
	v_funcs_ptr = value_to_array(v_funcs);
	v_funcs_ptr[0] = value_create_string(funcs, sizeof(funcs) - 1);
	v_funcs_ptr[1] = v_array[0];
	v_map[1] = v_funcs;

	/* Classes */
	static const char classes[] = "classes";
	value *v_classes_ptr, v_classes = value_create_array(NULL, 2);
	v_classes_ptr = value_to_array(v_classes);
	v_classes_ptr[0] = value_create_string(classes, sizeof(classes) - 1);
	v_classes_ptr[1] = v_array[1];
	v_map[2] = v_classes;

	/* Objects */
	static const char objects[] = "objects";
	value *v_objects_ptr, v_objects = value_create_array(NULL, 2);
	v_objects_ptr = value_to_array(v_objects);
	v_objects_ptr[0] = value_create_string(objects, sizeof(objects) - 1);
	v_objects_ptr[1] = v_array[2];
	v_map[3] = v_objects;

	return v;
}

int scope_export_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	scope_export_cb_iterator export_iterator = (scope_export_cb_iterator)args;

	const char *key_str = (const char *)key;

	value *v_array, v = value_create_array(NULL, 2);

	(void)s;

	if (v == NULL)
	{
		return 0;
	}

	v_array = value_to_array(v);

	v_array[0] = value_create_string(key_str, strlen(key_str));

	if (v_array[0] == NULL)
	{
		value_type_destroy(v);

		return 0;
	}

	v_array[1] = value_type_copy(val);

	if (v_array[1] == NULL)
	{
		value_type_destroy(v);

		return 0;
	}

	export_iterator->values[export_iterator->iterator] = v;
	++export_iterator->iterator;

	return 0;
}

value scope_export(scope sp)
{
	struct scope_export_cb_iterator_type export_iterator;

	value export = value_create_map(NULL, scope_size(sp));

	if (export == NULL)
	{
		return NULL;
	}

	export_iterator.iterator = 0;
	export_iterator.values = value_to_map(export);

	set_iterate(sp->objects, &scope_export_cb_iterate, (set_cb_iterate_args)&export_iterator);

	return export;
}

value scope_get(scope sp, const char *key)
{
	if (sp != NULL && key != NULL)
	{
		return (value)set_get(sp->objects, (set_key)key);
	}

	return NULL;
}

value scope_undef(scope sp, const char *key)
{
	if (sp != NULL && key != NULL)
	{
		return (value)set_remove(sp->objects, (set_key)key);
	}

	return NULL;
}

int scope_append(scope dest, scope src)
{
	return set_append(dest->objects, src->objects);
}

int scope_contains(scope dest, scope src, char **duplicated)
{
	return set_contains_which(dest->objects, src->objects, (set_key *)duplicated);
}

int scope_remove(scope dest, scope src)
{
	return set_disjoint(dest->objects, src->objects);
}

size_t *scope_stack_return(scope sp)
{
	if (sp != NULL && sp->call_stack != NULL)
	{
		size_t call_stack_size = vector_size(sp->call_stack);

		if (call_stack_size >= sizeof(size_t))
		{
			size_t return_position = call_stack_size - 1 - sizeof(size_t);

			void **return_ptr = vector_at(sp->call_stack, return_position);

			if (return_ptr != NULL && *return_ptr != NULL)
			{
				size_t *size_return_ptr = *return_ptr;

				return size_return_ptr;
			}

			log_write("metacall", LOG_LEVEL_ERROR, "Scope stack return invalid reference");

			return NULL;
		}

		log_write("metacall", LOG_LEVEL_ERROR, "Scope stack return empty");

		return NULL;
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Scope stack return invalid parameters");

	return NULL;
}

scope_stack_ptr scope_stack_push(scope sp, size_t bytes)
{
	if (sp != NULL && sp->call_stack != NULL && bytes > 0)
	{
		scope_stack_ptr *return_ptr = NULL;

		scope_stack_ptr prev_size = vector_size(sp->call_stack);

		scope_stack_ptr next_size = prev_size + bytes + sizeof(size_t);

		if (vector_resize(sp->call_stack, next_size) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Scope stack push bad call stack size increase");

			return 0;
		}

		return_ptr = scope_stack_return(sp);

		if (return_ptr != NULL)
		{
			void **prev_ptr = vector_at(sp->call_stack, prev_size);

			if (prev_ptr != NULL && *prev_ptr != NULL)
			{
				*return_ptr = prev_size;

				return prev_size;
			}

			log_write("metacall", LOG_LEVEL_ERROR, "Scope stack push bad stack pointer reference");
		}

		log_write("metacall", LOG_LEVEL_ERROR, "Scope stack push bad stack return reference");

		if (vector_resize(sp->call_stack, prev_size) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Scope stack push bad call stack size decrease");
		}

		return 0;
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Scope stack push invalid parameters");

	return 1;
}

void *scope_stack_get(scope sp, scope_stack_ptr stack_ptr)
{
	if (sp != NULL)
	{
		void **ref_ptr = vector_at(sp->call_stack, stack_ptr);

		if (ref_ptr != NULL && *ref_ptr != NULL)
		{
			return *ref_ptr;
		}
	}

	return NULL;
}

int scope_stack_pop(scope sp)
{
	if (sp != NULL)
	{
		scope_stack_ptr *return_ptr = scope_stack_return(sp);

		if (vector_resize(sp->call_stack, *return_ptr) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Scope bad call stack size decrease");

			return 1;
		}

		return 0;
	}

	log_write("metacall", LOG_LEVEL_ERROR, "Scope stack pop invalid parameters");

	return 1;
}

int scope_destroy_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
	(void)key;
	(void)args;

	if (val != NULL)
	{
		value_type_destroy(val);

		return 0;
	}

	return 1;
}

void scope_destroy(scope sp)
{
	if (sp != NULL)
	{
		set_iterate(sp->objects, &scope_destroy_cb_iterate, NULL);

		set_destroy(sp->objects);

		vector_destroy(sp->call_stack);

		free(sp->name);

		free(sp);
	}
}
