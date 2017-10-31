/*
*	Serial Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/* -- Methods -- */

/**
*  @brief
*    Convert to string value @v to a parsed value
*
*  @param[in] v
*    Reference to the value to be parsed
*
*  @param[in] id
*    Result type will have the returned value when parsed
*
*  @return
*    Reference to the new parsed value (and destroy
*    old string value @v from memory) if success, null (and
*    destroy is not performed) otherwise
*/
//METACALL_SERIAL_API value value_type_parse(value v, type_id id);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_SERIAL_IMPL_DESERIALIZE_H */
