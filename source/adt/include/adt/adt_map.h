/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#ifndef ADT_MAP_H
#define ADT_MAP_H 1

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

struct map_type;

struct map_iterator_type;

/* -- Type Definitions -- */

typedef struct map_type *map;

typedef hash map_hash;

typedef comparable map_key;

typedef void *map_value;

typedef void *map_cb_iterate_args;

typedef hash_callback map_cb_hash;

typedef comparable_callback map_cb_compare;

typedef int (*map_cb_iterate)(map, map_key, map_value, map_cb_iterate_args);

typedef struct map_iterator_type *map_iterator;

/* -- Methods -- */

ADT_API map map_create(map_cb_hash hash_cb, map_cb_compare compare_cb);

ADT_API size_t map_size(map m);

ADT_API int map_insert(map m, map_key key, map_value value);

ADT_API int map_insert_array(map m, map_key keys[], map_value values[], size_t size);

ADT_API vector map_get(map m, map_key key);

ADT_API vector map_get_keys(map m);

ADT_API vector map_get_values(map m);

ADT_API int map_contains(map m, map_key key);

ADT_API int map_contains_any(map dest, map src);

ADT_API map_value map_remove(map m, map_key key);

ADT_API vector map_remove_all(map m, map_key key);

ADT_API void map_iterate(map m, map_cb_iterate iterate_cb, map_cb_iterate_args args);

ADT_API int map_append(map dest, map src);

ADT_API int map_clear(map m);

ADT_API void map_destroy(map m);

ADT_API map_iterator map_iterator_begin(map m);

ADT_API map_key map_iterator_get_key(map_iterator it);

ADT_API map_value map_iterator_get_value(map_iterator it);

ADT_API void map_iterator_next(map_iterator it);

ADT_API int map_iterator_end(map_iterator *it);

#ifdef __cplusplus
}
#endif

#endif /* ADT_MAP_H */
