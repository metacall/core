/*
*	Serial Library by Parra Studios
*	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple serialization and deserialization formats.
*
*/

#ifndef METACALL_SERIAL_IMPL_SERIALIZE_H
#define METACALL_SERIAL_IMPL_SERIALIZE_H 1

/* -- Headers -- */

#include <metacall_serial/metacall_serial_api.h>

#include <serial/serial_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Type Definitions -- */

typedef void (*metacall_serialize_impl_ptr)(value, char *, size_t, const char *, size_t *);

/* -- Methods -- */

/**
*  @brief
*    Provides value format depending on type id @id
*
*  @param[in] id
*    Type of wanted format
*
*  @return
*    Print format assigned to type id @id
*/
METACALL_SERIAL_API const char * metacall_serial_impl_serialize_format(type_id id);

/**
*  @brief
*    Provides pointer to function for serialize depending on type id @id
*
*  @param[in] id
*    Type of wanted serialize function pointer
*
*  @return
*    Serialize function pointer assigned to type id @id
*/
METACALL_SERIAL_API metacall_serialize_impl_ptr metacall_serial_impl_serialize_func(type_id id);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_SERIAL_IMPL_SERIALIZE_H */
