/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#ifndef METACALL_LINK_H
#define METACALL_LINK_H 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Initialize link detours and allocate shared memory
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_link_initialize(void);

// TODO: Implement dlsym hook function table

/**
*  @brief
*    Unregister link detours and destroy shared memory
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_link_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_LINK_H */
