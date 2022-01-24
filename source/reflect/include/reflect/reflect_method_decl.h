/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#ifndef REFLECT_METHOD_DECL_H
#define REFLECT_METHOD_DECL_H 1

#ifdef __cplusplus
extern "C" {
#endif

struct method_type;

typedef void *method_impl;

typedef struct method_type *method;

typedef void (*method_impl_interface_destroy)(method, method_impl);

typedef struct method_interface_type
{
	method_impl_interface_destroy destroy;

} * method_interface;

typedef method_interface (*method_impl_interface_singleton)(void);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_METHOD_DECL_H */
