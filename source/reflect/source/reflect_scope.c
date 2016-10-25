/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/reflect_scope.h>

#include <adt/hash_map.h>
#include <adt/vector.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

typedef struct scope_type
{
	char * name;			/**< Scope name */
	hash_map map;			/**< Map of scope objects indexed by name string */
	vector call_stack;		/**< Scope call stack */

} * scope;

static int scope_print_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args);

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

			sp->map = hash_map_create(&hash_callback_str, &comparable_callback_str);

			if (sp->map == NULL)
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

				hash_map_destroy(sp->map);

				free(sp->name);

				free(sp);

				return NULL;
			}

			if (vector_resize(sp->call_stack, sizeof(size_t)) != 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Scope create call stack bad allocation");

				vector_destroy(sp->call_stack);

				hash_map_destroy(sp->map);

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
		return hash_map_size(sp->map);
	}

	return 0;
}

int scope_define(scope sp, const char * key, scope_object obj)
{
	if (sp != NULL && key != NULL && obj != NULL)
	{
		return hash_map_insert(sp->map, (hash_map_key)key, (hash_map_value)obj);
	}

	return 1;
}

static int scope_print_cb_iterate(hash_map map, hash_map_key key, hash_map_value val, hash_map_cb_iterate_args args)
{
	if (map != NULL && key != NULL && val != NULL && args == NULL)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "Key (%s) -> Value (%p)", (char *)key, val);

		return 0;
	}

	return 1;
}

void scope_print(scope sp)
{
	log_write("metacall", LOG_LEVEL_DEBUG, "Scope (%s):", sp->name);

	hash_map_iterate(sp->map, &scope_print_cb_iterate, NULL);
}

scope_object scope_get(scope sp, const char * key)
{
	if (sp != NULL && key != NULL)
	{
		return (scope_object)hash_map_get(sp->map, (hash_map_key)key);
	}

	return NULL;
}

scope_object scope_undef(scope sp, const char * key)
{
	if (sp != NULL && key != NULL)
	{
		return (scope_object)hash_map_remove(sp->map, (hash_map_key)key);
	}

	return NULL;
}

int scope_append(scope dest, scope src)
{
	return hash_map_append(dest->map, src->map);
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

void scope_destroy(scope sp)
{
	if (sp)
	{
		vector_destroy(sp->call_stack);

		hash_map_destroy(sp->map);

		free(sp->name);

		free(sp);
	}
}
