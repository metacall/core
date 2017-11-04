/*
 *	Memory Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic cross-platform memory utility.
 *
 */

#ifndef MEMORY_H
#define MEMORY_H 1

/* -- Headers -- */

#include <memory/memory_api.h>

#include <memory/memory_allocator.h>
#include <memory/memory_allocator_std.h>
#include <memory/memory_allocator_nginx.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

MEMORY_API const char * memory_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_H */
