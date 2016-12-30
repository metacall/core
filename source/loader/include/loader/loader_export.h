/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_IMPL_EXPORT_H
#define LOADER_IMPL_EXPORT_H 1

#include <loader/loader_impl_descriptor.h>
#include <loader/loader_impl_interface.h>
#include <loader/loader_impl_print.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct loader_impl_export_type
{
	loader_impl_descriptor_singleton descriptor;		/**< Loader descriptor reference */
	loader_impl_interface_singleton interface;			/**< Loader interface reference */
	loader_impl_print_singleton print;					/**< Loader print information callback */

} * loader_impl_export;

typedef loader_impl_export (*loader_impl_export_singleton)(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_EXPORT_H */
