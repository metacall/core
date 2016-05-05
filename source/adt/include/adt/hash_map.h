/************************************************************************/
/*	Abstract Data Type Library by Parra Studios							*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A abstract data type library providing generic containers.			*/
/*																		*/
/************************************************************************/

#ifndef ADT_HASH_MAP_H
#define ADT_HASH_MAP_H 1

#include <adt/adt_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

typedef struct hash_map_type * hash_map;

typedef unsigned int hash_map_hash;

typedef void * hash_map_key;

typedef void * hash_map_value;

typedef void * hash_map_cb_iterate_args;

typedef hash_map_hash (*hash_map_cb_hash)(hash_map_key);

typedef int (*hash_map_cb_compare)(hash_map_key, hash_map_key);

typedef int (*hash_map_cb_iterate)(hash_map, hash_map_key, hash_map_value, hash_map_cb_iterate_args);

ADT_API hash_map hash_map_create(hash_map_cb_hash hash_cb, hash_map_cb_compare compare_cb);

ADT_API int hash_map_insert(hash_map map, hash_map_key key, hash_map_value value);

ADT_API hash_map_value hash_map_get(hash_map map, hash_map_key key);

ADT_API hash_map_value hash_map_remove(hash_map map, hash_map_key key);

ADT_API void hash_map_iterate(hash_map map, hash_map_cb_iterate iterate_cb, hash_map_cb_iterate_args args);

ADT_API int hash_map_clear(hash_map map);

ADT_API void hash_map_destroy(hash_map map);

#ifdef __cplusplus
}
#endif

#endif /* ADT_HASH_MAP_H */
