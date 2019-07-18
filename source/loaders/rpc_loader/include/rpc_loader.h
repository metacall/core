/*
 *	Loader Library by Declan Nnadozie
 *	A plugin for loading rpc code at run-time into a process.
 *
 *	Corpcright (C) 2016 - 2019 Declan Nnadozie <zedonbiz@gmail.com>
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

#ifndef rpc_LOADER_H
#define rpc_LOADER_H 1

#include <rpc_loader/rpc_loader_api.h>

#include <loader/loader_impl_interface.h>

#include <dynlink/dynlink.h>

#ifdef __cplusplus
extern "C" {
#endif

RPC_LOADER_API loader_impl_interface rpc_loader_impl_interface_singleton(void);

DYNLINK_SYMBOL_EXPORT(rpc_loader_impl_interface_singleton);

RPC_LOADER_API const char * rpc_loader_print_info(void);

DYNLINK_SYMBOL_EXPORT(rpc_loader_print_info);

#ifdef __cplusplus
}
#endif

#endif /* rpc_LOADER_H */
