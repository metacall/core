#ifndef HASH_MAP_H
#define HASH_MAP_H 1

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hash_map_type * hash_map;

typedef unsigned int hash_map_hash;

typedef void * hash_map_key;

typedef void * hash_map_value;

typedef void * hash_map_cb_iterate_args;

typedef hash_map_hash (*hash_map_cb_hash)(hash_map_key);

typedef int (*hash_map_cb_compare)(hash_map_key, hash_map_key);

typedef int (*hash_map_cb_iterate)(hash_map, hash_map_key, hash_map_value, hash_map_cb_iterate_args);

hash_map hash_map_create(hash_map_cb_hash hash_cb, hash_map_cb_compare compare_cb);

int hash_map_insert(hash_map map, hash_map_key key, hash_map_value value);

hash_map_value hash_map_get(hash_map map, hash_map_key key);

hash_map_value hash_map_remove(hash_map map, hash_map_key key);

void hash_map_iterate(hash_map map, hash_map_cb_iterate iterate_cb, hash_map_cb_iterate_args args);

void hash_map_destroy(hash_map map);

#ifdef __cplusplus
}
#endif

#endif // HASH_MAP_H
