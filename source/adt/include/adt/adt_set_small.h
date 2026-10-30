/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#ifndef ADT_SET_SMALL_H
#define ADT_SET_SMALL_H 1

/*
 * Small Set
 *
 * Minimal high-performance associative container specialized for
 * storing small sets of key-value pairs. It maps char[8] strings to
 * generic pointers of type void*. The 8 byte strings are a limitation
 * for implementing fast lookups.
 *
 * Designed specifically for small sets where low overhead and high
 * performance are preferred over a fully featured hash table.
 *
 * Features:
 *   - 8-character maximum string keys.
 *   - Generic pointer values (void *).
 *   - Opaque API with minimal memory overhead.
 *   - It has a initial fixed size and it is never reallocated.
 *   - The memory is contigously allocated along with the set_small struct.
 *
 * The container does not assume ownership of stored values, lifetime
 * management remains the responsibility of the caller.
 */

/* -- Headers -- */

#include <adt/adt_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Forward Declarations -- */

struct set_small_type;

struct set_small_element_type;

struct set_small_storage_type;

struct set_small_iterator_type;

/* -- Type Definitions -- */

typedef struct set_small_type *set_small;

typedef struct set_small_element_type *set_small_element;

typedef struct set_small_storage_type *set_small_storage;

typedef struct set_small_iterator_type *set_small_iterator;

/* -- Member Data -- */

struct set_small_iterator_type
{
	set_small s;
	size_t index;
};

/* -- Methods -- */

ADT_API set_small set_small_create(size_t capacity);

ADT_API size_t set_small_capacity(set_small s);

ADT_API size_t set_small_size(set_small s);

ADT_API int set_small_insert(set_small s, const char *key, void *value);

ADT_API void *set_small_get(set_small s, const char *key);

ADT_API void *set_small_remove(set_small s, const char *key);

ADT_API int set_small_clear(set_small s);

ADT_API void set_small_destroy(set_small s);

ADT_API void set_small_iterator_begin(set_small_iterator it, set_small s);

ADT_API const char *set_small_iterator_key(set_small_iterator it);

ADT_API void *set_small_iterator_value(set_small_iterator it);

ADT_API void set_small_iterator_next(set_small_iterator it);

ADT_API int set_small_iterator_end(set_small_iterator it);

#ifdef __cplusplus
}
#endif

#endif /* ADT_SET_SMALL_H */
