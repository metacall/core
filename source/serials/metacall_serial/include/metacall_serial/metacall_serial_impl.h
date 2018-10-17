/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

#ifndef METACALL_SERIAL_IMPL_H
#define METACALL_SERIAL_IMPL_H 1

/* -- Headers -- */

#include <metacall_serial/metacall_serial_api.h>

#include <serial/serial_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Retrieve extension supported by MetaCall Native Format implementation
*
*  @return
*    Returns constant string representing serial extension
*
*/
METACALL_SERIAL_API const char * metacall_serial_impl_extension(void);

/**
*  @brief
*    Initialize MetaCall Native Format document implementation
*
*  @return
*    Returns pointer to serial document implementation on success, null pointer otherwise
*
*/
METACALL_SERIAL_API serial_impl_handle metacall_serial_impl_initialize(memory_allocator allocator, serial_host host);

/**
*  @brief
*    Serialize with MetaCall Native Format document implementation @impl
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
METACALL_SERIAL_API char * metacall_serial_impl_serialize(serial_impl_handle handle, value v, size_t * size);

/**
*  @brief
*    Deserialize with MetaCall Native Format document implementation @handle
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
METACALL_SERIAL_API value metacall_serial_impl_deserialize(serial_impl_handle handle, const char * buffer, size_t size);

/**
*  @brief
*    Destroy MetaCall Native Format document implementation
*
*  @return
*    Returns zero on correct destruction, distinct from zero otherwise
*
*/
METACALL_SERIAL_API int metacall_serial_impl_destroy(serial_impl_handle handle);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_SERIAL_IMPL_H */
