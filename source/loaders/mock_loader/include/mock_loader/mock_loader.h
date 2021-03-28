/*
 *	Loader Library by Parra Studios
 *	A plugin for loading mock code at run-time into a process.
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

#ifndef MOCK_LOADER_H
#define MOCK_LOADER_H 1

#include <mock_loader/mock_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

MOCK_LOADER_API loader_impl_interface mock_loader_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(mock_loader_impl_interface_singleton);

MOCK_LOADER_API const char *mock_loader_print_info(void);

DYNLINK_SYMBOL_EXPORT(mock_loader_print_info);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_LOADER_H */
