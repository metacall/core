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

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */

#define DYNLINK_SYMBOL_EXPORT(type, name) /* TODO */

#define DYNLINK_SYMBOL_GET(name) /* TODO */

/* -- Type definitions -- */

typedef void * dynlink_symbol_addr_macos; /* TODO */

typedef dynlink_symbol_addr_macos dynlink_symbol_addr;

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
