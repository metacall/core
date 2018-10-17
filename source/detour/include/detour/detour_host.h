/*
 *	Detour Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library providing detours, function hooks and trampolines.
 *
 */

#ifndef DETOUR_HOST_H
#define DETOUR_HOST_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct detour_host_type;

/* -- Type Definitions -- */

typedef struct detour_host_type * detour_host;

/* -- Member Data -- */

/**
*  @brief
*    Structure holding host context from detour
*/
struct detour_host_type
{
	void * log;
};

#ifdef __cplusplus
}
#endif

#endif /* DETOUR_HOST_H */
