/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef DYNLINK_IMPL_SYMBOL_WIN32_H
#define DYNLINK_IMPL_SYMBOL_WIN32_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#include <preprocessor/preprocessor_concatenation.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */
#if defined(__MING32__) || defined(__MINGW64__)
	#define DYNLINK_SYMBOL_PREFIX \
		dynlink_symbol_

	/* -- Macros -- */

	#define DYNLINK_SYMBOL_EXPORT(name) \
		DYNLINK_NO_EXPORT struct dynlink_symbol_addr_win32_type DYNLINK_SYMBOL_NAME(name) = { \
			(dynlink_symbol_addr_win32_impl)&name \
		}
#else
	#define DYNLINK_SYMBOL_PREFIX

	/* -- Macros -- */

	#define DYNLINK_SYMBOL_EXPORT(name) \
		DYNLINK_NO_EXPORT struct \
		{ \
			char name; \
		} PREPROCESSOR_CONCAT(dynlink_no_export_, name)
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
	#define DYNLINK_SYMBOL_GET(name) \
		(((dynlink_symbol_addr_win32)(name))->symbol)
#else
	#define DYNLINK_SYMBOL_GET(name) name
#endif

/* -- Type definitions -- */

#if defined(__MINGW32__) || defined(__MINGW64__)
/* MinGW-compatible definition */
typedef void (*dynlink_symbol_addr_win32_impl)(void);

typedef struct dynlink_symbol_addr_win32_type
{
	dynlink_symbol_addr_win32_impl symbol;
} * dynlink_symbol_addr_win32;

#else
typedef void (*dynlink_symbol_addr_win32)(void);
#endif

typedef dynlink_symbol_addr_win32 dynlink_symbol_addr;

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_IMPL_SYMBOL_WIN32_H */
