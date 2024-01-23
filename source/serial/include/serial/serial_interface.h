/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H 1

/* -- Headers -- */

#include <serial/serial_api.h>

#include <serial/serial_handle.h>

#include <memory/memory.h>

#include <reflect/reflect.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Member Data -- */

struct serial_interface_type
{
	const char *(*extension)(void);
	serial_handle (*initialize)(memory_allocator);
	char *(*serialize)(serial_handle, value, size_t *);
	value (*deserialize)(serial_handle, const char *, size_t);
	int (*destroy)(serial_handle);
};

/* -- Type Definitions -- */

typedef struct serial_interface_type *serial_interface;

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_INTERFACE_H */
