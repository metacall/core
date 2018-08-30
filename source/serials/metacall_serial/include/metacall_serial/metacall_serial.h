/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

#ifndef METACALL_SERIAL_H
#define METACALL_SERIAL_H 1

/* -- Headers -- */

#include <metacall_serial/metacall_serial_api.h>

#include <serial/serial_interface.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Instance of interface implementation
*
*  @return
*    Returns pointer to interface to be used by implementation
*
*/
METACALL_SERIAL_API serial_interface metacall_serial_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(metacall_serial_impl_interface_singleton);

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*
*/
METACALL_SERIAL_API const char * metacall_serial_print_info(void);

DYNLINK_SYMBOL_EXPORT(metacall_serial_print_info);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_SERIAL_H */
