/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
