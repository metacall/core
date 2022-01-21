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

#ifndef DYNLINK_IMPL_NAME_H
#define DYNLINK_IMPL_NAME_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define DYNLINK_NAME_IMPL_SIZE 0xFF /**< Dynamically linked shared object name size */

/* -- Type definitions -- */

typedef char dynlink_name_impl[DYNLINK_NAME_IMPL_SIZE]; /**< Allocated copy of dynamically linked shared object name */

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_IMPL_NAME_H */
