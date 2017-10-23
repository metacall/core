/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
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
*    Initialize RapidJSON implementation
*
*  @return
*    Returns pointer to serial implementation on success, null pointer otherwise
*
*/
RAPID_JSON_SERIAL_API serial_impl rapid_json_serial_impl_initialize(void);

/**
*  @brief
*    Serialize with RapidJSON implementation @impl
*
*  @param[in] impl
*    Pointer to the serial implementation
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
RAPID_JSON_SERIAL_API const char * rapid_json_serial_impl_serialize(serial_impl impl, value v, size_t * size);

/**
*  @brief
*    Deserialize with RapidJSON implementation @impl
*
*  @param[in] impl
*    Pointer to the serial implementation
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
RAPID_JSON_SERIAL_API value rapid_json_serial_impl_deserialize(serial_impl impl, const char * buffer, size_t size);

/**
*  @brief
*    Destroy RapidJSON implementation
*
*  @return
*    Returns zero on correct destruction, distinct from zero otherwise
*
*/
RAPID_JSON_SERIAL_API int rapid_json_serial_impl_destroy(serial_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* RAPID_JSON_SERIAL_IMPL_H */
