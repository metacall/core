/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_IMPL_EXPORT_H
#define LOADER_IMPL_EXPORT_H 1

#include <dynlink/dynlink.h>

#include <loader/loader_impl_descriptor.h>
#include <loader/loader_impl_interface.h>
#include <loader/loader_impl_print.h>

#ifdef __cplusplus
extern "C" {
#endif
	/*
#define LOADER_IMPL_EXPORT_TAG loader_impl_export_tag

#define LOADER_IMPL_EXPORT(descriptor, interface) \
	DYNLINK_SYMBOL_EXPORT(LOADER_IMPL_EXPORT_TAG)
	*/

typedef void (*loader_impl_print_info)(void); /* TODO: move to print module */

typedef struct loader_impl_export_type
{
	loader_impl_descriptor descriptor;		/**< Loader descriptor reference */
	loader_impl_interface interface;		/**< Loader interface reference */
	loader_impl_print_info print;			/**< Loader print information callback */

} * loader_impl_export;

typedef loader_impl_export (*loader_impl_export_singleton)(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_EXPORT_H */
