/********************************************************************************/
/*	Dynamic Link Library by Parra Studios										*/
/*	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>	*/
/*																				*/
/*	A library for dynamic loading and linking shared objects at run-time.		*/
/*																				*/
/********************************************************************************/

#ifndef DYNLINK_IMPL_INTERFACE_H
#define DYNLINK_IMPL_INTERFACE_H 1

/* -- Headers -- */

#include <dynlink/dynlink_api.h>

#include <dynlink/dynlink_impl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Type definitions -- */

typedef const char * (*dynlink_impl_interface_extension)(void);
typedef dynlink_impl (*dynlink_impl_interface_load)(dynlink);
typedef int (*dynlink_impl_interface_symbol)(dynlink, dynlink_impl, dynlink_symbol_name, dynlink_symbol_addr *);
typedef int (*dynlink_impl_interface_unload)(dynlink, dynlink_impl);

typedef struct dynlink_impl_interface_type
{
	dynlink_impl_interface_extension extension;
	dynlink_impl_interface_load load;
	dynlink_impl_interface_symbol symbol;
	dynlink_impl_interface_unload unload;

} * dynlink_impl_interface;

typedef dynlink_impl_interface (*dynlink_impl_interface_singleton)(void);

#ifdef __cplusplus
}
#endif

#endif /* DYNLINK_IMPL_INTERFACE_H */
