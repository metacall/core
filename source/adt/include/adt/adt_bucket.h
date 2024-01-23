/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#ifndef ADT_BUCKET_H
#define ADT_BUCKET_H 1

/* -- Headers -- */

#include <adt/adt_api.h>

#include <adt/adt_comparable.h>
#include <adt/adt_vector.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Forward Declarations -- */

struct pair_type;
struct bucket_type;

/* -- Type Definitions -- */

typedef struct pair_type *pair;
typedef struct bucket_type *bucket;

/* -- Member Data -- */

struct pair_type
{
	void *key;
	void *value;
};

struct bucket_type
{
	size_t count;
	size_t capacity;
	pair pairs;
};

/* -- Methods -- */

ADT_API size_t bucket_capacity(size_t prime);

ADT_API bucket bucket_create(size_t size);

ADT_API int bucket_alloc_pairs(bucket b, size_t capacity);

ADT_API int bucket_realloc_pairs(bucket b, size_t new_capacity);

ADT_API pair bucket_get_pair(bucket b, comparable_callback compare_cb, void *key);

ADT_API vector bucket_get_pairs_value(bucket b, comparable_callback compare_cb, void *key);

ADT_API int bucket_insert(bucket b, void *key, void *value);

ADT_API int bucket_remove(bucket b, comparable_callback compare_cb, void *key, void **value);

#ifdef __cplusplus
}
#endif

#endif /* ADT_BUCKET_H */
