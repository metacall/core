/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#ifndef DYNLINK_TYPE_H
#define DYNLINK_TYPE_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward declarations -- */

struct dynlink_type;

/* -- Type definitions -- */

typedef struct dynlink_type *		dynlink;		/**< Dynamically linked shared object handle */
typedef const char *			dynlink_path;		/**< Dynamically linked shared object name */
typedef const char *			dynlink_name;		/**< Dynamically linked shared object name */
typedef const char *			dynlink_symbol_name;	/**< Dynamically linked shared object symbol name */

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_TYPE_H */
