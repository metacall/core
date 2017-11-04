/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <stdio.h>

/* -- Enumerations -- */

enum metacall_allocator_id
{
	METACALL_ALLOCATOR_STD,
	METACALL_ALLOCATOR_NGINX
};

/* -- Type Definitions -- */

typedef struct metacall_allocator_std_type * metacall_allocator_std;

typedef struct metacall_allocator_nginx_type * metacall_allocator_nginx;

/* -- Member Data -- */

struct metacall_allocator_std_type
{
	void * (*std_malloc)(size_t);
	void * (*std_realloc)(void *, size_t);
	void (*std_free)(void *);
};

struct metacall_allocator_nginx_type
{
	void * pool;
	void * (*nginx_palloc)(void *, size_t);
	int (*nginx_pfree)(void *, void *);
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
METACALL_API void * metacall_allocator(enum metacall_allocator_id allocator_id, void * ctx);

#ifdef __cplusplus
}
#endif

#endif  /* METACALL_ALLOCATOR_H */
