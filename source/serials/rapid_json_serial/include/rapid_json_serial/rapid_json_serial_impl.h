/*
 *	Serial Library by Parra Studios
 *	A cross-platform library for managing multiple serialization and deserialization formats.
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

#ifndef RAPID_JSON_SERIAL_IMPL_H
#define RAPID_JSON_SERIAL_IMPL_H 1

/* -- Headers -- */

#include <rapid_json_serial/rapid_json_serial_api.h>

#include <serial/serial_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Retrieve extension supported by RapidJSON implementation
*
*  @return
*    Returns constant string representing serial extension
*
*/
RAPID_JSON_SERIAL_API const char * rapid_json_serial_impl_extension(void);

/**
*  @brief
*    Initialize RapidJSON document implementation
*
*  @return
*    Returns pointer to serial document implementation on success, null pointer otherwise
*
*/
RAPID_JSON_SERIAL_API serial_impl_handle rapid_json_serial_impl_initialize(memory_allocator allocator, serial_host host);

/**
*  @brief
*    Serialize with RapidJSON document implementation @impl
*
*  @param[in] handle
*    Pointer to the serial document implementation
*
*  @param[in] v
*    Reference to the value is going to be serialized
*
*  @param[out] size
*    Size in bytes of the return buffer
*
*  @return
*    String with the value serialized on correct serialization, null otherwise
*
*/
RAPID_JSON_SERIAL_API char * rapid_json_serial_impl_serialize(serial_impl_handle handle, value v, size_t * size);

/**
*  @brief
*    Deserialize with RapidJSON document implementation @handle
*
*  @param[in] handle
*    Pointer to the serial document implementation
*
*  @param[in] buffer
*    Reference to the string is going to be deserialized
*
*  @param[in] size
*    Size in bytes of the string @buffer
*
*  @return
*    Pointer to value deserialized on correct serialization, null otherwise
*
*/
RAPID_JSON_SERIAL_API value rapid_json_serial_impl_deserialize(serial_impl_handle handle, const char * buffer, size_t size);

/**
*  @brief
*    Destroy RapidJSON document implementation
*
*  @return
*    Returns zero on correct destruction, distinct from zero otherwise
*
*/
RAPID_JSON_SERIAL_API int rapid_json_serial_impl_destroy(serial_impl_handle handle);

#ifdef __cplusplus
}
#endif

#endif /* RAPID_JSON_SERIAL_IMPL_H */
