/*
*	Serial Library by Parra Studios
*	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple serialization and deserialization formats.
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
