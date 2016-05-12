/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#ifndef DYNLINK_IMPL_SYMBOL_WIN32_H
#define DYNLINK_IMPL_SYMBOL_WIN32_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */

#define DYNLINK_SYMBOL_EXPORT(name) \
	DYNLINK_API static dynlink_symbol_addr_win32 DYNLINK_NAME(name) = (dynlink_symbol_addr_win32)&name

#define DYNLINK_SYMBOL_GET(name) name

/* -- Type definitions -- */

typedef void (*dynlink_symbol_addr_win32)(void);

typedef dynlink_symbol_addr_win32 dynlink_symbol_addr;

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_IMPL_SYMBOL_WIN32_H */
