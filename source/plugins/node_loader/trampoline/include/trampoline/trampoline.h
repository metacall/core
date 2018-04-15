/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading nodejs code at run-time into a process.
 *
 */

#ifndef NODE_LOADER_TRAMPOLINE_H
#define NODE_LOADER_TRAMPOLINE_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef void * (*node_loader_trampoline_register_ptr)(void *);

#ifdef __cplusplus
}
#endif

#endif /* NODE_LOADER_TRAMPOLINE_H */
