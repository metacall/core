/*
 *	Serial Library by Parra Studios
 *	A cross-platform library for managing multiple serialization and deserialization formats.
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

#ifndef METACALL_SERIAL_H
#define METACALL_SERIAL_H 1

/* -- Headers -- */

#include <metacall_serial/metacall_serial_api.h>

#include <serial/serial_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Instance of interface implementation
*
*  @return
*    Returns pointer to interface to be used by implementation
*
*/
METACALL_SERIAL_API serial_interface metacall_serial_impl_interface_singleton(void);

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*
*/
METACALL_SERIAL_API const char *metacall_serial_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_SERIAL_H */
