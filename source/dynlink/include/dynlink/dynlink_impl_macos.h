/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#ifndef DYNLINK_IMPL_MACOS_H
#define DYNLINK_IMPL_MACOS_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#include <dynlink/dynlink_impl_symbol_macos.h>

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
*    MacOS dynamic link shared object implementation singleton
*
*  @return
*    A pointer to the dynamically linked shared object implementation singleton
*/
DYNLINK_API dynlink_impl_interface dynlink_impl_interface_singleton_macos(void);

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_IMPL_MACOS_H */
