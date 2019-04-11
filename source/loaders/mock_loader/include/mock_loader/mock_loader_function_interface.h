/*
 *	Loader Library by Parra Studios
 *	A plugin for loading mock code at run-time into a process.
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

#ifndef MOCK_LOADER_FUNCTION_INTERFACE_H
#define MOCK_LOADER_FUNCTION_INTERFACE_H 1

#include <mock_loader/mock_loader_api.h>

#include <loader/loader_handle.h>

#include <reflect/function.h>

#ifdef __cplusplus
extern "C" {
#endif

struct loader_impl_mock_function_type;

typedef struct loader_impl_mock_function_type * loader_impl_mock_function;

struct loader_impl_mock_function_type
{
	loader_handle handle;
	void * function_mock_data;
};

MOCK_LOADER_API function_impl_interface_singleton mock_loader_impl_function_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_LOADER_FUNCTION_INTERFACE_H */
