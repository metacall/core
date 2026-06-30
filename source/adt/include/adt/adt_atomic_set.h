/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#ifndef ADT_ATOMIC_SET_H
#define ADT_ATOMIC_SET_H 1

/*
 * Atomic Set
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
 *   - Lock free implementation suitable for concurrent access.
 *   - Opaque API with minimal memory overhead.
 *   - Supports thread safe insertion, lookup, removal, iteration, and clearing.
 *   - It has a initial fixed size and it is never reallocated.
 *   - Once an element is deleted it stays in the set but with invalid state, it can be reinserted.
 *   - The memory is contigously allocated along with the atomic_set struct.
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

struct atomic_set_type;

struct atomic_set_element_type;

struct atomic_set_storage_type;

struct atomic_set_iterator_type;

/* -- Type Definitions -- */

typedef struct atomic_set_type *atomic_set;

typedef struct atomic_set_element_type *atomic_set_element;

typedef struct atomic_set_storage_type *atomic_set_storage;

typedef struct atomic_set_iterator_type *atomic_set_iterator;

/* -- Member Data -- */

struct atomic_set_iterator_type
{
	atomic_set s;
	// TODO
};

/* -- Methods -- */

ADT_API atomic_set atomic_set_create(size_t capacity);

ADT_API size_t atomic_set_capacity(atomic_set s);

ADT_API size_t atomic_set_size(atomic_set s);

ADT_API int atomic_set_insert(atomic_set s, const char *key, void *value);

ADT_API void *atomic_set_get(atomic_set s, const char *key);

ADT_API void *atomic_set_remove(atomic_set s, const char *key);

ADT_API int atomic_set_clear(atomic_set s);

ADT_API void atomic_set_destroy(atomic_set s);

ADT_API void atomic_set_iterator_begin(atomic_set_iterator it, atomic_set s);

ADT_API const char *atomic_set_iterator_key(atomic_set_iterator it);

ADT_API void *atomic_set_iterator_value(atomic_set_iterator it);

ADT_API void atomic_set_iterator_next(atomic_set_iterator it);

ADT_API int atomic_set_iterator_end(atomic_set_iterator it);

#ifdef __cplusplus
}
#endif

#endif /* ADT_ATOMIC_SET_H */
