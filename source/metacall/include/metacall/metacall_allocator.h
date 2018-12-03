/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#ifndef METACALL_ALLOCATOR_H
#define METACALL_ALLOCATOR_H 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>
#include <stdint.h>

/* -- Enumerations -- */

enum metacall_allocator_id
{
	METACALL_ALLOCATOR_STD,
	METACALL_ALLOCATOR_NGINX
};

/* -- Forward Declarations -- */

struct ngx_pool_s;

/* -- Type Definitions -- */

typedef struct metacall_allocator_std_type * metacall_allocator_std;

typedef struct ngx_pool_s ngx_pool_t;

typedef struct metacall_allocator_nginx_type * metacall_allocator_nginx;

/* -- Member Data -- */

struct metacall_allocator_std_type
{
	void * (*malloc)(size_t);
	void * (*realloc)(void *, size_t);
	void (*free)(void *);
};

struct metacall_allocator_nginx_type
{
	ngx_pool_t * pool;
	void * (*palloc)(ngx_pool_t *, size_t);
	void * (*pcopy)(void *, const void *, size_t);
	intptr_t (*pfree)(ngx_pool_t *, void *);
};

/* -- Methods -- */

/**
*  @brief
*    Create an allocator instance
*
*  @param[in] allocator_id
*    Type of allocator to be created
*
*  @param[in] ctx
*    Context of the allocator
*
*  @return
*    Pointer to allocator if success, null otherwise
*/
METACALL_API void * metacall_allocator_create(enum metacall_allocator_id allocator_id, void * ctx);

/**
*  @brief
*    Reserve memory from an allocator instance
*
*  @param[in] allocator
*    Pointer to allocator instance
*
*  @param[in] size
*    Size in bytes to be allocated
*
*  @return
*    Pointer to allocated data on success, null otherwise
*/
METACALL_API void * metacall_allocator_alloc(void * allocator, size_t size);

/**
*  @brief
*    Reallocate memory from an allocator instance
*
*  @param[in] allocator
*    Pointer to allocator instance
*
*  @param[in] data
*    Original pointer to data
*
*  @param[in] size
*    Original size in bytes
*
*  @param[in] new_size
*    New size in bytes to be reallocated
*
*  @return
*    Pointer to new reallocated data on success, null otherwise
*/
METACALL_API void * metacall_allocator_realloc(void * allocator, void * data, size_t size, size_t new_size);

/**
*  @brief
*    Free memory from an allocator instance
*
*  @param[in] allocator
*    Pointer to allocator instance
*
*  @param[in] data
*    Pointer to data to be freed
*/
METACALL_API void metacall_allocator_free(void * allocator, void * data);

/**
*  @brief
*    Destroy an allocator instance
*
*  @param[in] allocator
*    Pointer to allocator instance
*/
METACALL_API void metacall_allocator_destroy(void * allocator);

#ifdef __cplusplus
}
#endif

#endif  /* METACALL_ALLOCATOR_H */
