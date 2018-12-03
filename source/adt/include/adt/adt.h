/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#ifndef ADT_H
#define ADT_H 1

#include <adt/adt_api.h>

#include <adt/adt_comparable.h>
#include <adt/adt_hash.h>
#include <adt/adt_vector.h>
#include <adt/adt_trie.h>

#ifdef __cplusplus
extern "C" {
#endif

ADT_API const char * adt_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* ADT_H */
