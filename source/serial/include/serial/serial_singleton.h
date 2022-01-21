/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

#ifndef SERIAL_SINGLETON_H
#define SERIAL_SINGLETON_H 1

/* -- Headers -- */

#include <serial/serial_api.h>

#include <serial/serial.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct serial_singleton_type;

/* -- Type Definitions -- */

typedef struct serial_singleton_type *serial_singleton;

/* -- Methods -- */

/**
 *  @brief
 *    Initialize serial singleton
 *
 *  @return
 *    Returns zero on correct serial singleton initialization, distinct from zero otherwise
 *
 */
SERIAL_API int serial_singleton_initialize();

/**
 *  @brief
 *    Wrapper of serial singleton instance
 *
 *  @return
 *    Pointer to serial singleton instance
 *
 */
SERIAL_API serial_singleton serial_singleton_instance(void);

/**
 *  @brief
 *    Register serial into serials map
 *
 *  @param[in] s
 *    Pointer to serial
 *
 *  @return
 *    Returns zero on correct serial singleton insertion, distinct from zero otherwise
 *
 */
SERIAL_API int serial_singleton_register(serial s);

/**
 *  @brief
 *    Retrieve serial from serials map by @name
 *
 *  @param[in] name
 *    Index which references the serial to be retrieved
 *
 *  @return
 *    Returns pointer to serial if exists, null otherwise
 *
 */
SERIAL_API serial serial_singleton_get(const char *name);

/**
 *  @brief
 *    Retrieve serial library path where serials are located
 *
 *  @return
 *    Returns constant string representing serials library path
 *
 */
SERIAL_API const char *serial_singleton_path(void);

/**
 *  @brief
 *    Remove serial from serials map
 *
 *  @param[in] s
 *    Pointer to serial
 *
 *  @return
 *    Returns zero on correct serial singleton removing, distinct from zero otherwise
 *
 */
SERIAL_API int serial_singleton_clear(serial s);

/**
 *  @brief
 *    Destroy serial singleton
 *
 */
SERIAL_API void serial_singleton_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_SINGLETON_H */
