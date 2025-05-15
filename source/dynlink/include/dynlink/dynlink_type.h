/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
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

typedef struct dynlink_type *dynlink;			  /**< Dynamically linked shared object handle */
typedef void *dynlink_impl;						  /**< Dynamically linked shared object implementation */
typedef char dynlink_path[PORTABILITY_PATH_SIZE]; /**< Allocated copy of dynamically linked shared object name */
typedef void (*dynlink_symbol_addr)(void);		  /**< Function pointer referring to a symbol address */

/* -- Macros -- */

#define dynlink_symbol_cast(type, symbol, result) \
	do \
	{ \
		union \
		{ \
			type ptr; \
			dynlink_symbol_addr fn; \
		} cast; \
\
		cast.ptr = (symbol); \
		(result) = cast.fn; \
\
	} while (0)

#define dynlink_symbol_uncast(fn, result) \
	do \
	{ \
		union \
		{ \
			void *ptr; \
			dynlink_symbol_addr fn; \
		} cast; \
\
		cast.fn = (fn); \
		(result) = cast.ptr; \
\
	} while (0)

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_TYPE_H */
