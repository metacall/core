/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
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

#ifndef REFLECT_H
#define REFLECT_H 1

#include <reflect/reflect_api.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_value.h>
#include <reflect/reflect_value_type.h>
#include <reflect/reflect_value_type_demotion.h>
#include <reflect/reflect_value_type_promotion.h>
#include <reflect/reflect_value_type_cast.h>
#include <reflect/reflect_value_type_id_size.h>
#include <reflect/reflect_type.h>
#include <reflect/reflect_type_id.h>
#include <reflect/reflect_signature.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_future.h>

#ifdef __cplusplus
extern "C" {
#endif

REFLECT_API const char * reflect_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* REFLECT_H */
