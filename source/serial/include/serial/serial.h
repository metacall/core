/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

#ifndef SERIAL_H
#define SERIAL_H 1

/* -- Headers -- */

#include <serial/serial_api.h>

#include <reflect/reflect.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct serial_type;

/* -- Type Definitions -- */

typedef struct serial_type * serial;

/* -- Methods -- */

/**
*  @brief
*    Initialize serial module
*
*  @return
*    Return zero correct initialization, distinct from zero otherwise
*
*/
SERIAL_API int serial_initialize(void);

/**
*  @brief
*    Create serial by @name
*
*  @param[in] name
*    Plugin will be used to serialize and deserialize
*
*  @return
*    Pointer to serial on correct initialization, null otherwise
*
*/
SERIAL_API serial serial_create(const char * name);

/**
*  @brief
*    Get extension of serial
*
*  @param[in] s
*    Reference to the serial
*
*  @return
*    Static const string with serial extension
*
*/
SERIAL_API const char * serial_extension(serial s);

/**
*  @brief
*    Get name of serial
*
*  @param[in] s
*    Reference to the serial
*
*  @return
*    Static const string with serial name
*
*/
SERIAL_API const char * serial_name(serial s);

/**
*  @brief
*    Convert a value @v to a serialized string using serial @s
*
*  @param[in] s
*    Reference to the serial will be used to serialize value @v
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
SERIAL_API const char * serial_serialize(serial s, value v, size_t * size);

/**
*  @brief
*    Convert a string @buffer to a deserialized value using serial @s
*
*  @param[in] s
*    Reference to the serial will be used to deserialize string @buffer
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
SERIAL_API value serial_deserialize(serial s, const char * buffer, size_t size);

/**
*  @brief
*    Destroy serial by handle @s
*
*/
SERIAL_API int serial_clear(serial s);

/**
*  @brief
*    Destroy all serials
*
*/
SERIAL_API void serial_destroy(void);

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*
*/
SERIAL_API const char * serial_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_H */
