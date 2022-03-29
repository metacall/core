/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef DYNLINK_TYPE_H
#define DYNLINK_TYPE_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#include <portability/portability_library_path.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward declarations -- */

struct dynlink_type;

/* -- Type definitions -- */

typedef struct dynlink_type *dynlink;						   /**< Dynamically linked shared object handle */
typedef const char *dynlink_path;							   /**< Dynamically linked shared object path */
typedef const char *dynlink_name;							   /**< Dynamically linked shared object name */
typedef const char *dynlink_symbol_name;					   /**< Dynamically linked shared object symbol name */
typedef portability_library_path_str dynlink_library_path_str; /**< Dynamically linked shared object symbol name */

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_TYPE_H */
