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

#include <serial/serial_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Retrieve extension supported by the serial implementation
*
*  @return
*    Returns constant string representing serial extension
*
*/
SERIAL_API const char * serial_impl_extension(void);

/**
*  @brief
*    Initialize serial object implementation
*
*  @param[in] name
*    Dependency name to be injected
*
*  @return
*    Returns zero on correct initialization, distinct from zero otherwise
*
*/
SERIAL_API int serial_impl_initialize(const char * name);

/**
*  @brief
*    Load serial object implementation @s
*
*  @param[in] config
*    Pointer to the serial object to be loaded
*
*  @return
*    Returns zero on correct loading, distinct from zero otherwise
*
*/
SERIAL_API int serial_impl_load(serial s);

/**
*  @brief
*    Unload serial object implementation @s
*
*  @param[in] s
*    Pointer to the config object to be unloaded
*
*  @return
*    Returns zero on correct unloading, distinct from zero otherwise
*
*/
SERIAL_API int serial_impl_unload(serial s);

/**
*  @brief
*    Destroy serial object implementation
*
*  @return
*    Returns zero on correct destruction, distinct from zero otherwise
*
*/
SERIAL_API int serial_impl_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_IMPL_H */
