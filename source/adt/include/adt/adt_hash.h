/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#ifndef ADT_HASH_H
#define ADT_HASH_H 1

#include <adt/adt_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Type Definitions -- */

typedef unsigned int hash;

typedef void * hash_key;

typedef hash (*hash_callback)(const hash_key);

/* -- Methods -- */

ADT_API hash hash_callback_str(const hash_key key);

#ifdef __cplusplus
}
#endif

#endif /* ADT_HASH_H */
