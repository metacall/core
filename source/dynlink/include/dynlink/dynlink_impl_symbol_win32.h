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

#include <preprocessor/preprocessor_concatenation.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define DYNLINK_SYMBOL_PREFIX

/* -- Macros -- */

#define DYNLINK_SYMBOL_EXPORT(name) \
	DYNLINK_NO_EXPORT struct \
	{ \
		char name; \
	} PREPROCESSOR_CONCAT(dynlink_no_export_, name)

#define DYNLINK_SYMBOL_GET(name) name

/* -- Type definitions -- */

typedef void (*dynlink_symbol_addr_win32)(void);

typedef dynlink_symbol_addr_win32 dynlink_symbol_addr;

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_IMPL_SYMBOL_WIN32_H */
