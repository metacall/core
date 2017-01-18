/*
 *	Configuration Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple configuration formats.
 *
 */

#ifndef CONFIGURATION_INTERFACE_H
#define CONFIGURATION_INTERFACE_H 1

/* -- Headers -- */

#include <configuration/configuration_api.h>

#include <configuration/configuration_object.h>
#include <configuration/configuration_impl_handle.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct configuration_interface_type;

/* -- Type Definitions -- */

typedef int (*configuration_interface_initialize)(void);

typedef configuration_impl (*configuration_interface_load)(configuration);

typedef value (*configuration_interface_get)(configuration, const char *, type_id);

typedef int (*configuration_interface_unload)(configuration);

typedef int (*configuration_interface_destroy)(void);

typedef struct configuration_interface_type * configuration_interface;

/* -- Member Data -- */

struct configuration_interface_type
{
	configuration_interface_initialize initialize;
	configuration_interface_load load;
	configuration_interface_get get;
	configuration_interface_unload unload;
	configuration_interface_destroy destroy;
};

/* -- Methods -- */

/**
*  @brief
*    Instance of interface implementation
*
*  @return
*    Returns ointer to interface to be used by implementation
*
*/
CONFIGURATION_API configuration_interface configuration_interface_instance(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_INTERFACE_H */
