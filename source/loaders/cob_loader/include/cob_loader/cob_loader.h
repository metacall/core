/*
 *	Loader Library by Parra Studios
 *	A plugin for loading cobol code at run-time into a process.
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

#ifndef COB_LOADER_H
#define COB_LOADER_H 1

#include <cob_loader/cob_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

COB_LOADER_API loader_impl_interface cob_loader_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(cob_loader_impl_interface_singleton);

COB_LOADER_API const char *cob_loader_print_info(void);

DYNLINK_SYMBOL_EXPORT(cob_loader_print_info);

#ifdef __cplusplus
}
#endif

#endif /* COB_LOADER_H */
