/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#ifndef ADT_SET_BUCKET_H
#define ADT_SET_BUCKET_H 1

/* -- Headers -- */

#include <adt/adt_api.h>

#include <adt/adt_comparable.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Forward Declarations -- */

struct set_pair_type;
struct set_bucket_type;

/* -- Type Definitions -- */

typedef struct set_pair_type *set_pair;
typedef struct set_bucket_type *set_bucket;

/* -- Member Data -- */

struct set_pair_type
{
	void *key;
	void *value;
};

struct set_bucket_type
{
	size_t count;
	size_t capacity;
	set_pair pairs;
};

/* -- Methods -- */

ADT_API size_t set_bucket_capacity(size_t prime);

ADT_API set_bucket set_bucket_create(size_t size);

ADT_API int set_bucket_alloc_pairs(set_bucket bucket, size_t capacity);

ADT_API int set_bucket_realloc_pairs(set_bucket bucket, size_t new_capacity);

ADT_API set_pair set_bucket_get_pair(set_bucket bucket, comparable_callback compare_cb, void *key);

ADT_API int set_bucket_insert(set_bucket bucket, void *key, void *value);

ADT_API int set_bucket_remove(set_bucket bucket, comparable_callback compare_cb, void *key, void **value);

#ifdef __cplusplus
}
#endif

#endif /* ADT_SET_BUCKET_H */
