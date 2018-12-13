/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef LOADER_IMPL_DESCRIPTOR_H
#define LOADER_IMPL_DESCRIPTOR_H 1

#include <loader/loader_descriptor.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef loader_descriptor (*loader_impl_descriptor_instance)(void);

typedef struct loader_impl_descriptor_type
{
	loader_impl_descriptor_instance descriptor;

} * loader_impl_descriptor;

typedef loader_impl_descriptor (*loader_impl_descriptor_singleton)(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_DESCRIPTOR_H */
