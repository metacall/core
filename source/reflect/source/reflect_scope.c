/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <reflect/reflect_scope.h>
#include <reflect/reflect_value_type.h>

#include <adt/adt_set.h>
#include <adt/adt_vector.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

struct scope_metadata_array_cb_iterator_type;

struct scope_export_cb_iterator_type;

typedef struct scope_metadata_array_cb_iterator_type * scope_metadata_array_cb_iterator;

typedef struct scope_export_cb_iterator_type * scope_export_cb_iterator;

struct scope_type
{
	char * name;			/**< Scope name */
	set objects;			/**< Map of scope objects indexed by name string */
	vector call_stack;		/**< Scope call stack */

};

struct scope_metadata_array_cb_iterator_type
{
	size_t iterator;
	value * values;
};

struct scope_export_cb_iterator_type
{
	size_t iterator;
	value * values;
};

static int scope_metadata_array_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static int scope_export_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

static value scope_metadata_array(scope sp);

static value scope_metadata_map(scope sp);

static value scope_metadata_name(scope sp);

static int scope_destroy_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args);

scope scope_create(const char * name)
{
	if (name != NULL)
	{
		scope sp = malloc(sizeof(struct scope_type));

		if (sp != NULL)
		{
			size_t sp_name_size = strlen(name) + 1;

			size_t * call_stack_head = NULL;

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

int scope_define(scope sp, const char * key, scope_object obj)
{
	if (sp != NULL && key != NULL && obj != NULL)
	{
		/* TODO: Support for other scope objects (e.g: class) */
		if (set_insert(sp->objects, (set_key)key, (set_value)obj) == 0)
		{
			/* TODO: Support for polyphormism */
			function func = (function)obj;

			if (function_increment_reference(func) != 0)
			{
				set_remove(sp->objects, (set_key)key);

				/* TODO: Log about the error */

				return 1;
			}

			return 0;
		}
	}

	return 1;
}

int scope_metadata_array_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	scope_metadata_array_cb_iterator metadata_iterator = (scope_metadata_array_cb_iterator)args;

	/* TODO: Support to other scope objects (e.g: class) */
	(void)s;
	(void)key;

	metadata_iterator->values[metadata_iterator->iterator] = function_metadata((function)val);

	if (metadata_iterator->values[metadata_iterator->iterator] != NULL)
	{
		++metadata_iterator->iterator;
	}

	return 0;
}

value scope_metadata_array(scope sp)
{
	struct scope_metadata_array_cb_iterator_type metadata_iterator;

	value v = value_create_array(NULL, scope_size(sp));

	if (v == NULL)
	{
		return NULL;
	}

	metadata_iterator.iterator = 0;

	metadata_iterator.values = value_to_array(v);

	set_iterate(sp->objects, &scope_metadata_array_cb_iterate, (set_cb_iterate_args)&metadata_iterator);

	return v;
}

value scope_metadata_map(scope sp)
{
	static const char funcs[] = "funcs";

	value * v_ptr, v = value_create_array(NULL, 2);

	if (v == NULL)
	{
		return NULL;
	}

	v_ptr = value_to_array(v);

	/* TODO: Support to other scope objects (e.g: class) */
	v_ptr[0] = value_create_string(funcs, sizeof(funcs) - 1);

	if (v_ptr[0] == NULL)
	{
		value_type_destroy(v);
	}

	v_ptr[1] = scope_metadata_array(sp);

	if (v_ptr[1] == NULL)
	{
		value_type_destroy(v);
	}

	return v;
}

value scope_metadata_name(scope sp)
{
	static const char name[] = "name";

	value * v_ptr, v = value_create_array(NULL, 2);

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
	value * v_map, v = value_create_map(NULL, 2);

	if (v == NULL)
	{
		return NULL;
	}

	v_map = value_to_map(v);

	v_map[0] = scope_metadata_name(sp);

	if (v_map[0] == NULL)
	{
		value_type_destroy(v);
	}

	v_map[1] = scope_metadata_map(sp);

	if (v_map[1] == NULL)
	{
		value_type_destroy(v);
	}

	return v;
}

int scope_export_cb_iterate(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	scope_export_cb_iterator export_iterator = (scope_export_cb_iterator)args;

	const char * key_str = (const char *)key;

	/* TODO: Support to other scope objects (e.g: class) */
	function f = (function)val;

	value * v_array, v = value_create_array(NULL, 2);

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

	v_array[1] = value_create_function(f);

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

scope_object scope_get(scope sp, const char * key)
{
	if (sp != NULL && key != NULL)
	{
		return (scope_object)set_get(sp->objects, (set_key)key);
	}

	return NULL;
}

scope_object scope_undef(scope sp, const char * key)
{
	if (sp != NULL && key != NULL)
	{
		return (scope_object)set_remove(sp->objects, (set_key)key);
	}

	return NULL;
}

int scope_append(scope dest, scope src)
{
	return set_append(dest->objects, src->objects);
}

int scope_remove(scope dest, scope src)
{
	return set_disjoint(dest->objects, src->objects);
}

size_t * scope_stack_return(scope sp)
{
	if (sp != NULL && sp->call_stack != NULL)
	{
		size_t call_stack_size = vector_size(sp->call_stack);

		if (call_stack_size >= sizeof(size_t))
		{
			size_t return_position = call_stack_size - 1 - sizeof(size_t);

			void ** return_ptr = vector_at(sp->call_stack, return_position);

			if (return_ptr != NULL && *return_ptr != NULL)
			{
				size_t * size_return_ptr = *return_ptr;

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
		scope_stack_ptr * return_ptr = NULL;

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
			void ** prev_ptr = vector_at(sp->call_stack, prev_size);

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

void * scope_stack_get(scope sp, scope_stack_ptr stack_ptr)
{
	if (sp != NULL)
	{
		void ** ref_ptr = vector_at(sp->call_stack, stack_ptr);

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
		scope_stack_ptr * return_ptr = scope_stack_return(sp);

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
		/* TODO: Support for polyphormism */

		function func = (function)val;

		if (function_decrement_reference(func) != 0)
		{
			/* TODO: Log about the error, possible memory leak */
		}

		function_destroy(func);

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
