/*
 *	Reflect Library by Parra Studios
 *	A library for provide reflection and metadata representation.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef REFLECT_TEMPLATE_H
#define REFLECT_TEMPLATE_H

#include <reflect/reflect_api.h>

#include <reflect/reflect_class.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_template_decl.h>
#include <reflect/reflect_type.h>

#ifdef __cplusplus
extern "C" {
#endif

enum template_id
{
	TEMPLATE_TYPE_INVALID = 0,
	TEMPLATE_TYPE_FUNCTION = 1,
	TEMPLATE_TYPE_CLASS = 2
};

enum template_argument_id
{
	TEMPLATE_ARGUMENT_TYPE = 0,
	TEMPLATE_ARGUMENT_TEMPLATE = 1
};

typedef struct template_argument_type
{
	const char *name;
	enum template_argument_id kind;

	union
	{
		type t;
		reflect_template tpl;
	} data;

} template_argument;

typedef int (*template_impl_interface_create)(reflect_template, template_impl);

typedef function (*template_impl_interface_instantiate_function)(reflect_template, template_impl, template_argument[], size_t);

typedef klass (*template_impl_interface_instantiate_class)(reflect_template, template_impl, template_argument[], size_t);

typedef void (*template_impl_interface_destroy)(reflect_template, template_impl);

typedef struct template_interface_type
{
	template_impl_interface_create create;
	template_impl_interface_instantiate_function instantiate_function;
	template_impl_interface_instantiate_class instantiate_class;
	template_impl_interface_destroy destroy;

} *template_interface;

typedef template_interface (*template_impl_interface_singleton)(void);

REFLECT_API reflect_template template_create(const char *name, enum template_id type, template_impl impl, template_impl_interface_singleton singleton);

REFLECT_API int template_increment_reference(reflect_template tpl);

REFLECT_API int template_decrement_reference(reflect_template tpl);

REFLECT_API const char *template_name(reflect_template tpl);

REFLECT_API enum template_id template_type(reflect_template tpl);

REFLECT_API int template_add_parameter(reflect_template tpl, const char *parameter);

REFLECT_API size_t template_parameter_count(reflect_template tpl);

REFLECT_API const char *template_parameter(reflect_template tpl, size_t index);

REFLECT_API function template_instantiate_function(reflect_template tpl, template_argument args[], size_t size);

REFLECT_API klass template_instantiate_class(reflect_template tpl, template_argument args[], size_t size);

REFLECT_API void template_destroy(reflect_template tpl);

/**
 *  @brief
 *    Print debug statistics about template allocations and usage
 *
 *  @note
 *    Only produces output when the memory tracker is enabled
 *    (OPTION_MEMORY_TRACKER=ON at build time)
 */
REFLECT_API void template_stats_debug(void);

#ifdef __cplusplus
}
#endif

#endif
