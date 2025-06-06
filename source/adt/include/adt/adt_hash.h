/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#ifndef ADT_HASH_H
#define ADT_HASH_H 1

#include <adt/adt_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdint.h>

/* -- Type Definitions -- */

typedef uintptr_t hash;

typedef void *hash_key;

typedef hash (*hash_callback)(const hash_key);

/* -- Methods -- */

ADT_API hash hash_callback_str(const hash_key key);

ADT_API hash hash_callback_ptr(const hash_key key);

#ifdef __cplusplus
}
#endif

#endif /* ADT_HASH_H */
