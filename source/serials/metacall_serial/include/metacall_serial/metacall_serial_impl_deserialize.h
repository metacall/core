/*
 *	Serial Library by Parra Studios
 *	A cross-platform library for managing multiple serialization and deserialization formats.
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

#ifndef METACALL_SERIAL_IMPL_DESERIALIZE_H
#define METACALL_SERIAL_IMPL_DESERIALIZE_H 1

/* -- Headers -- */

#include <metacall_serial/metacall_serial_api.h>

#include <serial/serial_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Type Definitions -- */

typedef int (*metacall_deserialize_impl_ptr)(value *, const char *, size_t);

/* -- Methods -- */

/**
*  @brief
*    Provides pointer to function for deserialize depending on type id @id
*
*  @param[in] id
*    Type of wanted deserialize function pointer
*
*  @return
*    Deserialize function pointer assigned to type id @id
*/
METACALL_SERIAL_API metacall_deserialize_impl_ptr metacall_serial_impl_deserialize_func(type_id id);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_SERIAL_IMPL_DESERIALIZE_H */
