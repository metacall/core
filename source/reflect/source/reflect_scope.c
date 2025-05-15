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

#include <reflect/reflect_class.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_value_type.h>

#include <adt/adt_set.h>
#include <adt/adt_vector.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct scope_type
{
	char *name;		   /**< Scope name */
	set objects;	   /**< Map of scope objects indexed by name string */
	vector call_stack; /**< Scope call stack */
};

static int scope_metadata_array(scope sp, value v_array[3]);

static value scope_metadata_name(scope sp);

static value scope_export_value(const char *key, value val);

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

int scope_metadata_array(scope sp, value v_array[3])
{
	size_t functions_size = 0, classes_size = 0, objects_size = 0;
	value functions_value, classes_value, objects_value;
	value *functions, *classes, *objects;
	set_iterator it;

	for (it = set_iterator_begin(sp->objects); set_iterator_end(&it) != 0; set_iterator_next(it))
	{
		type_id id = value_type_id(set_iterator_value(it));

		if (id == TYPE_FUNCTION)
		{
			functions_size++;
		}
		else if (id == TYPE_CLASS)
		{
			classes_size++;
		}
		else if (id == TYPE_OBJECT)
		{
			objects_size++;
		}
	}

	functions_value = value_create_array(NULL, functions_size);

	if (functions_value == NULL)
	{
		goto functions_error;
	}

	functions = value_to_array(functions_value);

	classes_value = value_create_array(NULL, classes_size);

	if (classes_value == NULL)
	{
		goto classes_error;
	}

	classes = value_to_array(classes_value);

	objects_value = value_create_array(NULL, objects_size);

	if (objects_value == NULL)
	{
		goto objects_error;
	}

	objects = value_to_array(objects_value);

	/* Reuse counters to fill the arrays */
	classes_size = 0;
	functions_size = 0;
	objects_size = 0;

	for (it = set_iterator_begin(sp->objects); set_iterator_end(&it) != 0; set_iterator_next(it))
	{
		value v = set_iterator_value(it);
		type_id id = value_type_id(v);

		if (id == TYPE_FUNCTION)
		{
			functions[functions_size++] = function_metadata(value_to_function(v));
		}
		else if (id == TYPE_CLASS)
		{
			classes[classes_size++] = class_metadata(value_to_class(v));
		}
		else if (id == TYPE_OBJECT)
		{
			objects[objects_size++] = object_metadata(value_to_object(v));
		}
	}

	v_array[0] = functions_value;
	v_array[1] = classes_value;
	v_array[2] = objects_value;

	return 0;

objects_error:
	value_destroy(classes_value);
classes_error:
	value_destroy(functions_value);
functions_error:
	return 1;
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

value scope_export_value(const char *key, value val)
{
	value *v_array, v = value_create_array(NULL, 2);

	if (v == NULL)
	{
		goto array_create_error;
	}

	v_array = value_to_array(v);

	v_array[0] = value_create_string(key, strlen(key));

	if (v_array[0] == NULL)
	{
		goto string_create_error;
	}

	v_array[1] = value_type_copy(val);

	if (v_array[1] == NULL)
	{
		goto value_copy_error;
	}

	return v;

value_copy_error:
	value_type_destroy(v_array[0]);
string_create_error:
	value_type_destroy(v);
array_create_error:
	return NULL;
}

value scope_export(scope sp)
{
	value *values, export = value_create_map(NULL, scope_size(sp));
	size_t values_it;
	set_iterator it;

	if (export == NULL)
	{
		return NULL;
	}

	values = value_to_map(export);

	for (it = set_iterator_begin(sp->objects), values_it = 0; set_iterator_end(&it) != 0; set_iterator_next(it))
	{
		value v = scope_export_value(set_iterator_key(it), set_iterator_value(it));

		values[values_it++] = v;
	}

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

void scope_destroy(scope sp)
{
	if (sp != NULL)
	{
		set_iterator it;

		for (it = set_iterator_begin(sp->objects); set_iterator_end(&it) != 0; set_iterator_next(it))
		{
			value_type_destroy(set_iterator_value(it));
		}

		set_destroy(sp->objects);

		vector_destroy(sp->call_stack);

		free(sp->name);

		free(sp);
	}
}
