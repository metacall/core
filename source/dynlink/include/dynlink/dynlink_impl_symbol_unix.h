/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#ifndef DYNLINK_IMPL_SYMBOL_UNIX_H
#define DYNLINK_IMPL_SYMBOL_UNIX_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */

#define DYNLINK_SYMBOL_EXPORT(name) \
	DYNLINK_API static struct dynlink_symbol_addr_unix_type = DYNLINK_SYMBOL_NAME(name) = \
	{ \
		(dynlink_symbol_addr_unix_impl)&name \
	}

#define DYNLINK_SYMBOL_GET(name) \
	((dynlink_symbol_addr_unix)(name))->symbol

/* -- Type definitions -- */

typedef void (*dynlink_symbol_addr_unix_impl)(void);

typedef struct dynlink_symbol_addr_unix_type
{
	dynlink_symbol_addr_unix_impl symbol;
} * dynlink_symbol_addr_unix;

typedef dynlink_symbol_addr_unix dynlink_symbol_addr;

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_IMPL_SYMBOL_UNIX_H */
