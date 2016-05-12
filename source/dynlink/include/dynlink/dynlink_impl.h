/*
 *	Dynamic Link Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for dynamic loading and linking shared objects at run-time.
 *
 */

#ifndef DYNLINK_IMPL_H
#define DYNLINK_IMPL_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#include <dynlink/dynlink_type.h>
#include <dynlink/dynlink_impl_type.h>
#include <dynlink/dynlink_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Dynamically linked shared object handle extension implementation
*
*  @return
*    A const string reference to the extension depending on the OS implementation
*/
DYNLINK_API const char * dynlink_impl_extension(void);

/**
*  @brief
*    Load a dynamically linked shared object implementation
*
*  @param[in] name
*    Pointer to the dynamically linked shared object handle
*
*  @return
*    A pointer to the dynamically linked shared object implementation
*/
DYNLINK_API dynlink_impl dynlink_impl_load(dynlink handle);

/**
*  @brief
*    Get a symbol address of dynamically linked shared object by name implementation
*
*  @param[in] handle
*    Handle of dynamically linked shared object
*
*  @param[in] impl
*    Pointer to the dynamically linked shared object implementation
*
*  @param[in] symbol_name
*    Name of the of dynamically linked shared object symbol
*
*  @param[out] symbol_address
*    Pointer to the address of the of dynamically linked shared object symbol
*
*  @return
*    Returns zero on correct dynamic linking, distinct from zero otherwise
*/
DYNLINK_API int dynlink_impl_symbol(dynlink handle, dynlink_impl impl, dynlink_symbol_name symbol_name, dynlink_symbol_addr * symbol_address);

/**
*  @brief
*    Unloads a dynamically linked shared object implementation by reference
*
*  @param[in] handle
*    Handle of dynamically linked shared object
*
*  @param[in] impl
*    Pointer to the dynamically linked shared object implementation
*/
DYNLINK_API void dynlink_impl_unload(dynlink handle, dynlink_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_IMPL_H */
