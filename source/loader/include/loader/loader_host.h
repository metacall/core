/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_HOST_H
#define LOADER_HOST_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct loader_host_type;

/* -- Type Definitions -- */

typedef struct loader_host_type * loader_host;

/* -- Member Data -- */

/**
*  @brief
*    Structure holding host context from loader
*/
struct loader_host_type
{
	void * log;
};

#ifdef __cplusplus
}
#endif

#endif /* LOADER_HOST_H */
