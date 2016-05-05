/************************************************************************/
/*	Abstract Data Type Library by Parra Studios							*/
/*	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																		*/
/*	A abstract data type library providing generic containers.			*/
/*																		*/
/************************************************************************/

#ifndef ADT_HASH_MAP_STR_H
#define ADT_HASH_MAP_STR_H 1

#include <adt/adt_api.h>

#include <adt/hash_map.h>

#ifdef __cplusplus
extern "C" {
#endif

ADT_API hash_map_hash hash_map_cb_hash_str(hash_map_key key);

ADT_API int hash_map_cb_compare_str(hash_map_key key_a, hash_map_key key_b);

#ifdef __cplusplus
}
#endif

#endif /* ADT_HASH_MAP_STR_H */
