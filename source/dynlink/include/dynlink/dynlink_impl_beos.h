/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#ifndef DYNLINK_IMPL_BEOS_H
#define DYNLINK_IMPL_BEOS_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#include <dynlink/dynlink_impl_symbol_beos.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward declarations -- */

struct dynlink_impl_interface_type;

/* -- Type definitions -- */

typedef struct dynlink_impl_interface_type * dynlink_impl_interface;

/* -- Methods -- */

/**
*  @brief
*    BeOS/Haiku image add-on object implementation singleton
*
*  @return
*    A pointer to the image add-on object implementation singleton
*/
DYNLINK_API dynlink_impl_interface dynlink_impl_interface_singleton_beos(void);

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_IMPL_BEOS_H */
