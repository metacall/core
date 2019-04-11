/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#ifndef DYNLINK_SYMBOL_H
#define DYNLINK_SYMBOL_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#include <dynlink/dynlink_type.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define DYNLINK_SYMBOL_NAME_SIZE 0xFF

/* -- Type Definitions -- */

typedef char dynlink_symbol_name_man[DYNLINK_SYMBOL_NAME_SIZE];

/* -- Methods -- */

/**
*  @brief
*    Get convert a symbol to name mangled for cross-platform dynamic loading
*
*  @param[in] symbol_name
*    Reference to name of the of dynamically linked shared object symbol
*
*  @param[out] symbol_mangled
*    Reference to mangled name of the of dynamically linked shared object symbol
*
*  @return
*    Returns zero if @symbol_name was correctly mangled
*/
DYNLINK_API int dynlink_symbol_name_mangle(dynlink_symbol_name symbol_name, dynlink_symbol_name_man symbol_mangled);

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_H */
