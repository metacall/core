/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#ifndef DYNLINK_IMPL_NAME_H
#define DYNLINK_IMPL_NAME_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#ifdef __cplusplus
extern "C" {
#endif
	
/* -- Definitions -- */

#define DYNLINK_NAME_IMPL_SIZE			0xFF					/**< Dynamically linked shared object name size */

/* -- Type definitions -- */

typedef char dynlink_name_impl[DYNLINK_NAME_IMPL_SIZE];			/**< Allocated copy of dynamically linked shared object name */

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_IMPL_NAME_H */
