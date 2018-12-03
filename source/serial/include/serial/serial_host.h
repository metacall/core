/*
 *	Serial Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple serialization and deserialization formats.
 *
 */

#ifndef SERIAL_HOST_H
#define SERIAL_HOST_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct serial_host_type;

/* -- Type Definitions -- */

typedef struct serial_host_type * serial_host;

/* -- Member Data -- */

/**
*  @brief
*    Structure holding host context from serial
*/
struct serial_host_type
{
	void * log;
};

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_HOST_H */
