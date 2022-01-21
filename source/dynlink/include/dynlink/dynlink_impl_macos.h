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

#ifndef DYNLINK_IMPL_MACOS_H
#define DYNLINK_IMPL_MACOS_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#include <dynlink/dynlink_impl_symbol_macos.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward declarations -- */

struct dynlink_impl_interface_type;

/* -- Type definitions -- */

typedef struct dynlink_impl_interface_type *dynlink_impl_interface;

/* -- Methods -- */

/**
 *  @brief
 *    MacOS dynamic link shared object implementation singleton
 *
 *  @return
 *    A pointer to the dynamically linked shared object implementation singleton
 */
DYNLINK_API dynlink_impl_interface dynlink_impl_interface_singleton_macos(void);

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_IMPL_MACOS_H */
