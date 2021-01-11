/*
 *	Loader Library by Parra Studios
 *	A library for loading executable code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef LOADER_IMPL_PRINT_H
#define LOADER_IMPL_PRINT_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*loader_impl_print_info)(void);

typedef struct loader_impl_print_type
{
	loader_impl_print_info print_info;

} * loader_impl_print;

typedef loader_impl_print (*loader_impl_print_singleton)(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_PRINT_H */
