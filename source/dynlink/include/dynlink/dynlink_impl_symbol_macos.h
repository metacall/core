/*
 *	Dynamic Link Library by Parra Studios
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef DYNLINK_IMPL_SYMBOL_MACOS_H
#define DYNLINK_IMPL_SYMBOL_MACOS_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define DYNLINK_SYMBOL_PREFIX \
	dynlink_symbol_

/* -- Macros -- */

#define DYNLINK_SYMBOL_EXPORT(name)                                                 \
	DYNLINK_API struct dynlink_symbol_addr_macos_type DYNLINK_SYMBOL_NAME(name) = { \
		(dynlink_symbol_addr_macos_impl)&name                                       \
	}

#define DYNLINK_SYMBOL_GET(name) \
	((dynlink_symbol_addr_macos)(name))->symbol

/* -- Type definitions -- */

typedef void (*dynlink_symbol_addr_macos_impl)(void);

typedef struct dynlink_symbol_addr_macos_type
{
	dynlink_symbol_addr_macos_impl symbol;
} * dynlink_symbol_addr_macos;

typedef dynlink_symbol_addr_macos dynlink_symbol_addr;

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_IMPL_SYMBOL_MACOS_H */
