/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#ifndef LOADER_MANAGER_IMPL_H
#define LOADER_MANAGER_IMPL_H 1

/* -- Headers -- */

#include <loader/loader_api.h>

#include <loader/loader_impl.h>

#include <plugin/plugin_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdint.h>

/* -- Member Data -- */

struct loader_initialization_order_type
{
	uint64_t id;
	plugin p;
	int being_deleted;
};

struct loader_manager_impl_type
{
	plugin host;				 /* Points to the internal host loader (it stores functions registered by the user) */
	vector initialization_order; /* Stores the loader implementations by order of initialization (used for destruction) */
	uint64_t init_thread_id;	 /* Stores the thread id of the thread that initialized metacall */
	vector script_paths;		 /* Vector of search path for the scripts */
	set destroy_map;			 /* Tracks the list of destroyed runtimes during destruction of the manager (loader_impl -> NULL) */
};

/* -- Type Definitions -- */

typedef struct loader_initialization_order_type *loader_initialization_order;

typedef struct loader_manager_impl_type *loader_manager_impl;

/* -- Methods  -- */

LOADER_API loader_manager_impl loader_manager_impl_initialize(void);

LOADER_API plugin_manager_interface loader_manager_impl_iface(void);

LOADER_API void loader_manager_impl_set_destroyed(loader_manager_impl manager_impl, loader_impl impl);

LOADER_API int loader_manager_impl_is_destroyed(loader_manager_impl manager_impl, loader_impl impl);

LOADER_API void loader_manager_impl_destroy(loader_manager_impl manager_impl);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_MANAGER_IMPL_H */
