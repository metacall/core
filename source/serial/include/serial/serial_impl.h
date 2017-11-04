/*
*	Serial Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple serialization and deserialization formats.
*
*/

#ifndef SERIAL_IMPL_H
#define SERIAL_IMPL_H 1

/* -- Headers -- */

#include <serial/serial_api.h>

#include <memory/memory.h>

#include <reflect/reflect.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct serial_impl_type;

/* -- Type Definitions -- */

typedef struct serial_impl_type * serial_impl;

/* -- Methods -- */

/**
*  @brief
*    Create serial implementation
*
*  @return
*    Returns pointer to serial implementation correct creation, null otherwise
*
*/
SERIAL_API serial_impl serial_impl_create(void);

/**
*  @brief
*    Retrieve extension supported by the serial implementation
*
*  @param[in] impl
*    Pointer to serial implementation instance
*
*  @return
*    Returns constant string representing serial extension
*
*/
SERIAL_API const char * serial_impl_extension(serial_impl impl);

/**
*  @brief
*    Load serial implementation @impl
*
*  @param[in] impl
*    Pointer to the serial implementation to be loaded
*
*  @param[in] path
*    Path where dependency is located
*
*  @param[in] name
*    Dependency name to be injected
*
*  @return
*    Returns zero on correct loading, distinct from zero otherwise
*
*/
SERIAL_API int serial_impl_load(serial_impl impl, const char * path, const char * name);

/**
*  @brief
*    Convert a value @v to a serialized string using serial implementation @impl
*
*  @param[in] impl
*    Reference to the serial implementation will be used to serialize value @v
*
*  @param[in] v
*    Reference to the value is going to be serialized
*
*  @param[out] size
*    Size in bytes of the return buffer
*
*  @param[in] allocator
*    Allocator to be used serialize @v
*
*  @return
*    String with the value serialized on correct serialization, null otherwise
*
*/
SERIAL_API char * serial_impl_serialize(serial_impl impl, value v, size_t * size, memory_allocator allocator);

/**
*  @brief
*    Convert a string @buffer to a deserialized value using serial implementation @impl
*
*  @param[in] impl
*    Reference to the serial implementation will be used to deserialize string @buffer
*
*  @param[in] buffer
*    Reference to the string is going to be deserialized
*
*  @param[in] size
*    Size in bytes of the string @buffer
*
*  @param[in] allocator
*    Allocator to be used deserialize @buffer
*
*  @return
*    Pointer to value deserialized on correct serialization, null otherwise
*
*/
SERIAL_API value serial_impl_deserialize(serial_impl impl, const char * buffer, size_t size, memory_allocator allocator);

/**
*  @brief
*    Unload serial implementation @impl
*
*  @param[in] impl
*    Pointer to the serial implementation to be unloaded
*
*  @return
*    Returns zero on correct unloading, distinct from zero otherwise
*
*/
SERIAL_API int serial_impl_unload(serial_impl impl);

/**
*  @brief
*    Destroy serial implementation
*
*  @param[in] impl
*    Pointer to the serial implementation to be destroyed
*
*  @return
*    Returns zero on correct destruction, distinct from zero otherwise
*
*/
SERIAL_API int serial_impl_destroy(serial_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_IMPL_H */
