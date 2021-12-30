/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#ifndef ADT_TRIE_H
#define ADT_TRIE_H 1

/* -- Headers -- */

#include <adt/adt_api.h>

#include <adt/adt_comparable.h>
#include <adt/adt_hash.h>
#include <adt/adt_vector.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Forward Declarations -- */

struct trie_type;

/* -- Type Definitions -- */

typedef struct trie_type *trie;

typedef hash trie_hash;

typedef comparable trie_key;

typedef void *trie_value;

typedef void *trie_cb_iterate_args;

typedef hash_callback trie_cb_hash;

typedef comparable_callback trie_cb_compare;

typedef int (*trie_cb_iterate)(trie, trie_key, trie_value, trie_cb_iterate_args);

/* -- Methods -- */

ADT_API trie trie_create(trie_cb_hash hash_cb, trie_cb_compare compare_cb);

ADT_API trie trie_create_reserve(size_t capacity, size_t key_limit, size_t depth_limit, trie_cb_hash hash_cb, trie_cb_compare compare_cb);

ADT_API size_t trie_size(trie t);

ADT_API size_t trie_capacity(trie t);

ADT_API int trie_insert(trie t, vector keys, trie_value value);

ADT_API trie_value trie_get(trie t, vector keys);

ADT_API trie_value trie_remove(trie t, vector keys);

ADT_API void trie_iterate_recursive(trie t, trie_cb_iterate iterate_cb, trie_cb_iterate_args args);

ADT_API void trie_iterate(trie t, trie_cb_iterate iterate_cb, trie_cb_iterate_args args);

ADT_API int trie_append(trie dest, trie src);

ADT_API int trie_clear(trie t);

ADT_API int trie_prefixes(trie t, trie_key key, vector prefixes);

ADT_API trie trie_suffixes(trie t, trie_key key);

ADT_API void trie_destroy(trie t);

#ifdef __cplusplus
}
#endif

#endif /* ADT_TRIE_H */
