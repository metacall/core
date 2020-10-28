/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#ifndef ADT_SET_H
#define ADT_SET_H 1

/* -- Headers -- */

#include <adt/adt_api.h>

#include <adt/adt_hash.h>
#include <adt/adt_comparable.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Forward Declarations -- */

struct set_type;

struct set_iterator_type;

/* -- Type Definitions -- */

typedef struct set_type * set;

typedef hash set_hash;

typedef comparable set_key;

typedef void * set_value;

typedef void * set_cb_iterate_args;

typedef hash_callback set_cb_hash;

typedef comparable_callback set_cb_compare;

typedef int (*set_cb_iterate)(set, set_key, set_value, set_cb_iterate_args);

typedef struct set_iterator_type * set_iterator;

/* -- Methods -- */

ADT_API set set_create(set_cb_hash hash_cb, set_cb_compare compare_cb);

ADT_API size_t set_size(set s);

ADT_API int set_insert(set s, set_key key, set_value value);

ADT_API int set_insert_array(set s, set_key keys[], set_value values[], size_t size);

ADT_API set_value set_get(set s, set_key key);

ADT_API int set_contains(set s, set_key key);

ADT_API int set_contains_any(set dest, set src);

ADT_API set_value set_remove(set s, set_key key);

ADT_API void set_iterate(set s, set_cb_iterate iterate_cb, set_cb_iterate_args args);

ADT_API int set_append(set dest, set src);

ADT_API int set_disjoint(set dest, set src);

ADT_API int set_clear(set s);

ADT_API void set_destroy(set s);

ADT_API set_iterator set_iterator_begin(set s);

ADT_API set_key set_iterator_get_key(set_iterator it);

ADT_API set_value set_iterator_get_value(set_iterator it);

ADT_API void set_iterator_next(set_iterator it);

ADT_API int set_iterator_end(set_iterator * it);

#ifdef __cplusplus
}
#endif

#endif /* ADT_SET_H */
