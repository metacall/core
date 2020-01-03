/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H 1

/* -- Headers -- */

#include <serial/serial_api.h>

#include <serial/serial_impl_handle.h>
#include <serial/serial_host.h>

#include <memory/memory.h>

#include <reflect/reflect.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct serial_interface_type;

/* -- Type Definitions -- */

typedef const char * (*serial_interface_extension)(void);

typedef serial_impl_handle (*serial_interface_initialize)(memory_allocator, serial_host);

typedef char * (*serial_interface_serialize)(serial_impl_handle, value, size_t *);

typedef value (*serial_interface_deserialize)(serial_impl_handle, const char *, size_t);

typedef int (*serial_interface_destroy)(serial_impl_handle);

typedef struct serial_interface_type * serial_interface;

typedef serial_interface (*serial_interface_singleton)(void);

/* -- Member Data -- */

struct serial_interface_type
{
	serial_interface_extension extension;
	serial_interface_initialize initialize;
	serial_interface_serialize serialize;
	serial_interface_deserialize deserialize;
	serial_interface_destroy destroy;
};

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_INTERFACE_H */
