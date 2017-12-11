/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

/* -- Headers -- */

#include <adt/adt_trie.h>
#include <adt/adt_set.h>

#include <log/log.h>

/* -- Definitions -- */

#define TRIE_CAPACITY_MIN	((size_t)0x10)

/* -- Forward Declarations -- */

struct trie_node_ref_type;

struct trie_node_free_type;

struct trie_node_type;

struct trie_node_set_iterator_args_type;

struct trie_node_append_iterator_args_type;

struct trie_node_suffixes_iterator_args_type;

/* -- Type Definitions -- */

typedef struct trie_node_ref_type * trie_node_ref;

typedef struct trie_node_free_type * trie_node_free;

typedef struct trie_node_type * trie_node;

typedef struct trie_node_set_iterator_args_type * trie_node_set_iterator_args;

typedef struct trie_node_append_iterator_args_type * trie_node_append_iterator_args;

typedef struct trie_node_suffixes_iterator_args_type * trie_node_suffixes_iterator_args;

/* -- Member Data -- */

struct trie_node_ref_type
{
	trie_key	key;		/**< Pointer to the key of the node */
	size_t		index;		/**< Reference to node in trie set */
};

struct trie_node_free_type
{
	trie_node_free	next;		/**< Reference to the next node in the list */
	size_t		index;		/**< Reference to node in trie set */
};

struct trie_node_type
{
	size_t	parent_index;		/**< Reference to parent trie node */
	size_t	self_index;		/**< Reference to itself inside trie node list */
	trie_key	key;		/**< Pointer to key of the node */
	trie_value	value;		/**< Pointer to data of the node */
	set		childs;		/**< Set with references to child trie nodes (trie_key -> trie_node_ref) */
};

struct trie_type
{
	trie_node	root;		/**< Trie root node */
	trie_node	node_list;	/**< Array of trie nodes */
	size_t		size;		/**< Size of current nodes inside node list */
	size_t		capacity;	/**< Size of allocated nodes in memory */
	trie_node_free	free_node_list;	/**< List of free nodes in array (size_t) */
	size_t		key_limit;	/**< Maximum number of childs per trie node (0 == disabled) */
	size_t		depth_limit;	/**< Maximum number of depth levels in a trie (0 == disabled) */
	trie_cb_hash	hash_cb;	/**< Hash callback for node insertion */
	trie_cb_compare	compare_cb;	/**< Compare callback for value comparison */
};

struct trie_node_set_iterator_args_type
{
	trie t;				/**< Pointer to trie */
	trie_cb_iterate iterate_cb;	/**< Pointer to iterator callback */
	trie_cb_iterate_args args;	/**< Pointer to iterator arguments */
};

struct trie_node_append_iterator_args_type
{
	trie dest;			/**< Pointer to destination trie */
	vector prefixes;		/**< Vector containing prefixes for each iteration */
};

struct trie_node_suffixes_iterator_args_type
{
	trie suffix_trie;		/**< Pointer to new suffix trie */
	vector prefixes;		/**< Vector containing prefixes for each iteration */
};

/* -- Private Methods -- */

trie_node trie_node_get(trie t, vector keys);

trie_node trie_node_insert(trie t, trie_node parent, trie_key key, trie_value value);

void trie_node_iterate_recursive(trie t, trie_node n, trie_cb_iterate iterate_cb, trie_cb_iterate_args args);

void trie_node_iterate(trie t, trie_node n, trie_cb_iterate iterate_cb, trie_cb_iterate_args args);

int trie_node_clear(trie t, trie_node n);

/* -- Methods -- */

trie trie_create(trie_cb_hash hash_cb, trie_cb_compare compare_cb)
{
	return trie_create_reserve(TRIE_CAPACITY_MIN, 0, 0, hash_cb, compare_cb);
}

trie trie_create_reserve(size_t capacity, size_t key_limit, size_t depth_limit, trie_cb_hash hash_cb, trie_cb_compare compare_cb)
{
	trie t;

	size_t iterator;

	if (hash_cb == NULL && compare_cb == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Trie invalid callback");

		return NULL;
	}

	t = malloc(sizeof(struct trie_type));

	if (t == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Trie bad allocation");

		return NULL;
	}

	t->size = 1;
	t->capacity = (capacity < TRIE_CAPACITY_MIN) ? TRIE_CAPACITY_MIN : capacity;
	t->key_limit = key_limit;
	t->depth_limit = depth_limit;
	t->hash_cb = hash_cb;
	t->compare_cb = compare_cb;

	t->node_list = malloc(t->capacity * sizeof(struct trie_node_type));

	if (t->node_list == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Trie bad node list creation");

		free(t);

		return NULL;
	}

	for (iterator = 0; iterator < t->capacity; ++iterator)
	{
		trie_node n = &t->node_list[iterator];

		n->parent_index = 0;
		n->self_index = 0;
		n->key = NULL;
		n->value = NULL;
		n->childs = NULL;
	}

	t->root = &t->node_list[0];

	t->free_node_list = NULL;

	return t;
}

size_t trie_size(trie t)
{
	if (t != NULL)
	{
		return t->size - 1;
	}

	return 0;
}

size_t trie_capacity(trie t)
{
	if (t != NULL)
	{
		return t->capacity;
	}

	return 0;
}

trie_node trie_node_insert(trie t, trie_node parent, trie_key key, trie_value value)
{
	trie_node child;
	trie_node_ref child_ref;

	if (t == NULL || parent == NULL || key == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Trie invalid node insertion parameters");

		return NULL;
	}

	child_ref = malloc(sizeof(struct trie_node_ref_type));

	if (child_ref == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Trie node insert bad reference node allocation");

		return NULL;
	}

	child_ref->key = key;

	if (parent->childs == NULL)
	{
		parent->childs = set_create(t->hash_cb, t->compare_cb);

		if (parent->childs == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Trie invalid child set allocation");

			free(child_ref);

			return NULL;
		}
	}

	if (t->free_node_list != NULL)
	{
		trie_node_free node_free = t->free_node_list;

		t->free_node_list = node_free->next;

		child_ref->index = node_free->index;

		child = &t->node_list[child_ref->index];

		free(node_free);

		++t->size;
	}
	else
	{
		if ((t->size + 1) >= t->capacity)
		{
			register void * node_list;

			size_t capacity = t->capacity << 1;

			size_t iterator;

			node_list = realloc(t->node_list, capacity * sizeof(struct trie_node_type));

			if (node_list == NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Trie bad node list reallocation");

				free(child_ref);

				return NULL;
			}

			t->node_list = node_list;

			t->capacity = capacity;

			t->root = &t->node_list[0];

			for (iterator = t->size + 1; iterator < t->capacity; ++iterator)
			{
				trie_node n = &t->node_list[iterator];

				n->parent_index = 0;
				n->self_index = 0;
				n->key = NULL;
				n->value = NULL;
				n->childs = NULL;
			}
		}

		child_ref->index = t->size;

		child = &t->node_list[child_ref->index];

		++t->size;
	}

	if (child != NULL)
	{
		if (set_insert(parent->childs, key, child_ref) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Trie invalid child insertion");

			free(child_ref);

			return NULL;
		}

		child->parent_index = parent->self_index;
		child->self_index = child_ref->index;
		child->key = key;
		child->value = value;
		child->childs = NULL;

		return child;
	}

	free(child_ref);

	return NULL;
}

int trie_insert(trie t, vector keys, trie_value value)
{
	if (t != NULL)
	{
		size_t iterator, size = vector_size(keys);

		trie_node current_node = t->root;

		for (iterator = 0; current_node != NULL && iterator < size; ++iterator)
		{
			trie_key * key_ptr = vector_at(keys, iterator);

			trie_node next_node = NULL;

			if (current_node->childs != NULL)
			{
				trie_node_ref node_ref = set_get(current_node->childs, *key_ptr);

				if (node_ref != NULL)
				{
					next_node = &t->node_list[node_ref->index];
				}
			}

			if (next_node == NULL)
			{
				if (iterator == (size - 1))
				{
					next_node = trie_node_insert(t, current_node, *key_ptr, value);
				}
				else
				{
					next_node = trie_node_insert(t, current_node, *key_ptr, NULL);
				}

				if (next_node == NULL)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Trie invalid node insertion");

					return 1;
				}
			}

			current_node = next_node;
		}

		return 0;
	}

	return 1;
}

trie_node trie_node_get(trie t, vector keys)
{
	if (t != NULL)
	{
		size_t iterator, size = vector_size(keys);

		trie_node current_node = t->root;

		for (iterator = 0; current_node != NULL && iterator < size; ++iterator)
		{
			trie_key * key_ptr = vector_at(keys, iterator);

			trie_node next_node = NULL;

			if (current_node->childs != NULL)
			{
				trie_node_ref node_ref = set_get(current_node->childs, *key_ptr);

				if (node_ref != NULL)
				{
					next_node = &t->node_list[node_ref->index];
				}
			}

			if (iterator == (size - 1) && next_node != NULL)
			{
				return next_node;
			}

			current_node = next_node;
		}
	}

	return NULL;
}

trie_value trie_get(trie t, vector keys)
{
	if (t != NULL)
	{
		trie_node n = trie_node_get(t, keys);

		if (n != NULL)
		{
			return n->value;
		}
	}

	return NULL;
}

trie_value trie_remove(trie t, vector keys)
{
	if (t != NULL)
	{
		trie_node n = trie_node_get(t, keys);

		if (n != NULL)
		{
			trie_value value = n->value;

			if (trie_node_clear(t, n) == 0)
			{
				return value;
			}
		}
	}

	return NULL;
}

int trie_node_childs_cb_iterator(set s, set_key key, set_value value, set_cb_iterate_args args)
{
	if (s != NULL && key != NULL && value != NULL && args != NULL)
	{
		trie_node_set_iterator_args iterator_args = args;

		trie_node_ref ref_node = value;

		trie_node current_node = &iterator_args->t->node_list[ref_node->index];

		trie_node_iterate(iterator_args->t, current_node, iterator_args->iterate_cb, iterator_args->args);

		return 0;
	}

	return 1;
}

void trie_node_iterate_recursive(trie t, trie_node n, trie_cb_iterate iterate_cb, trie_cb_iterate_args args)
{
	if (t != NULL && n != NULL && iterate_cb != NULL)
	{
		struct trie_node_set_iterator_args_type child_args;

		child_args.t = t;
		child_args.iterate_cb = iterate_cb;
		child_args.args = args;

		iterate_cb(t, n->key, n->value, args);

		set_iterate(n->childs, &trie_node_childs_cb_iterator, &child_args);
	}
}

void trie_iterate_recursive(trie t, trie_cb_iterate iterate_cb, trie_cb_iterate_args args)
{
	if (t != NULL && iterate_cb != NULL)
	{
		trie_node_iterate_recursive(t, t->root, iterate_cb, args);
	}
}

void trie_node_iterate(trie t, trie_node n, trie_cb_iterate iterate_cb, trie_cb_iterate_args args)
{
	if (t != NULL && n != NULL && iterate_cb != NULL)
	{
		vector node_stack = vector_create(sizeof(trie_node));

		vector_push_back(node_stack, &n);

		while (vector_size(node_stack) > 0)
		{
			trie_node * back_ptr = vector_back(node_stack);

			vector_pop_back(node_stack);

			if (back_ptr != NULL && *back_ptr != NULL)
			{
				trie_node back = *back_ptr;

				if (back->childs != NULL)
				{
					set_iterator it;
					for (it = set_iterator_begin(back->childs); set_iterator_end(&it) > 0; set_iterator_next(it))
					{
						trie_node_ref ref_node = set_iterator_get_value(it);

						trie_node current_node = &t->node_list[ref_node->index];

						vector_push_back(node_stack, &current_node);
					}
				}

				iterate_cb(t, back->key, back->value, args);
			}
		}

		vector_destroy(node_stack);
	}
}

void trie_iterate(trie t, trie_cb_iterate iterate_cb, trie_cb_iterate_args args)
{
	if (t != NULL && iterate_cb != NULL)
	{
		trie_node_iterate(t, t->root, iterate_cb, args);
	}
}

int trie_node_append_cb_iterator(trie t, trie_key key, trie_value value, trie_cb_iterate_args args)
{
	if (t != NULL && key != NULL && value != NULL && args != NULL)
	{
		trie_node_append_iterator_args iterator_args = args;

		vector_clear(iterator_args->prefixes);

		if (trie_prefixes(t, key, iterator_args->prefixes) == 0)
		{
			return trie_insert(iterator_args->dest, iterator_args->prefixes, value);
		}
	}

	return 1;
}

int trie_append(trie dest, trie src)
{
	if (dest != NULL && src != NULL)
	{
		struct trie_node_append_iterator_args_type args;

		args.dest = dest;
		args.prefixes = vector_create(sizeof(trie_key));

		trie_iterate(src, &trie_node_append_cb_iterator, &args);

		vector_destroy(args.prefixes);

		return 0;
	}

	return 1;
}

int trie_node_clear(trie t, trie_node n)
{
	if (t != NULL && n != NULL)
	{
		vector node_stack = vector_create(sizeof(trie_node));

		vector_push_back(node_stack, &n);

		while (vector_size(node_stack) > 0)
		{
			trie_node * back_ptr = vector_back(node_stack);

			vector_pop_back(node_stack);

			if (back_ptr != NULL && *back_ptr != NULL)
			{
				trie_node back = *back_ptr;

				trie_node_free free_node;

				if (back->childs != NULL)
				{
					set_iterator it;

					for (it = set_iterator_begin(back->childs); set_iterator_end(&it) > 0; set_iterator_next(it))
					{
						trie_node_ref ref_node = set_iterator_get_value(it);

						trie_node current_node = &t->node_list[ref_node->index];

						vector_push_back(node_stack, &current_node);

						free(ref_node);
					}

					set_destroy(back->childs);
				}

				free_node = malloc(sizeof(struct trie_node_free_type));

				if (free_node == NULL)
				{
					log_write("metacall", LOG_LEVEL_ERROR, "Trie invalid free node allocation");

					vector_destroy(node_stack);

					return 1;
				}

				free_node->next = t->free_node_list;

				free_node->index = back->self_index;

				t->free_node_list = free_node;

				--t->size;
			}
		}

		vector_destroy(node_stack);

		return 0;
	}

	return 1;
}

int trie_clear(trie t)
{
	if (t != NULL && t->root != NULL)
	{
		return trie_node_clear(t, t->root);
	}

	return 1;
}

trie_node trie_node_find(trie t, trie_key key)
{
	if (t != NULL && key != NULL)
	{
		vector node_stack = vector_create(sizeof(trie_node));

		vector_push_back(node_stack, &t->root);

		while (vector_size(node_stack) > 0)
		{
			trie_node * back_ptr = vector_back(node_stack);

			vector_pop_back(node_stack);

			if (back_ptr != NULL && *back_ptr != NULL)
			{
				trie_node back = *back_ptr;

				set_iterator it = NULL;

				if (back->childs != NULL)
				{
					for (it = set_iterator_begin(back->childs); set_iterator_end(&it) > 0; set_iterator_next(it))
					{
						trie_node_ref ref_node = set_iterator_get_value(it);

						trie_node current_node = &t->node_list[ref_node->index];

						vector_push_back(node_stack, &current_node);
					}
				}

				if (back->key != NULL && t->compare_cb(back->key, key) == 0)
				{
					while (set_iterator_end(&it) > 0)
					{
						set_iterator_next(it);
					}

					vector_destroy(node_stack);

					return back;
				}
			}
		}

		vector_destroy(node_stack);
	}

	return NULL;
}

int trie_prefixes(trie t, trie_key key, vector prefixes)
{
	if (t != NULL && key != NULL && prefixes != NULL)
	{
		trie_node node_iterator = trie_node_find(t, key);

		if (node_iterator != NULL)
		{
			size_t index_iterator;

			for (index_iterator = node_iterator->self_index; index_iterator > 0; index_iterator = node_iterator->parent_index)
			{
				node_iterator = &t->node_list[index_iterator];

				vector_push_front(prefixes, &node_iterator->key);
			}

			return 0;
		}
	}

	return 1;
}

int trie_node_suffixes_cb_iterator(trie t, trie_key key, trie_value value, trie_cb_iterate_args args)
{
	if (t != NULL && key != NULL && value != NULL && args != NULL)
	{
		trie_node_suffixes_iterator_args iterator_args = args;

		vector_clear(iterator_args->prefixes);

		if (trie_prefixes(t, key, iterator_args->prefixes) == 0)
		{
			return trie_insert(iterator_args->suffix_trie, iterator_args->prefixes, value);
		}
	}

	return 1;
}

trie trie_suffixes(trie t, trie_key key)
{
	if (t != NULL && key != NULL)
	{
		trie_node node_iterator = trie_node_find(t, key);

		if (node_iterator != NULL)
		{
			trie suffix_trie = trie_create(t->hash_cb, t->compare_cb);

			struct trie_node_suffixes_iterator_args_type suffix_args;

			if (suffix_trie == NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Trie invalid suffix trie creation");

				return NULL;
			}

			suffix_args.suffix_trie = suffix_trie;

			suffix_args.prefixes = vector_create(sizeof(trie_key));

			if (suffix_args.prefixes == NULL)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "Trie invalid prefix vector creation");

				trie_destroy(suffix_trie);

				return NULL;
			}

			trie_node_iterate(t, node_iterator, &trie_node_suffixes_cb_iterator, &suffix_args);

			vector_destroy(suffix_args.prefixes);

			return suffix_trie;
		}
	}

	return NULL;
}

void trie_destroy(trie t)
{
	if (t != NULL)
	{
		if (trie_clear(t) != 0)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Trie invalid destruction");
		}

		if (t->node_list != NULL)
		{
			free(t->node_list);
		}

		while (t->free_node_list != NULL)
		{
			trie_node_free free_node = t->free_node_list;

			t->free_node_list = t->free_node_list->next;

			free(free_node);
		}

		free(t);
	}
}
