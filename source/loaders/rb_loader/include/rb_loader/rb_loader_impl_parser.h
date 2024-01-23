/*
 *	Loader Library by Parra Studios
 *	A plugin for loading ruby code at run-time into a process.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef RB_LOADER_IMPL_PARSER_H
#define RB_LOADER_IMPL_PARSER_H 1

#include <adt/adt_set.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RB_LOADER_IMPL_PARSER_FUNC	0x40
#define RB_LOADER_IMPL_PARSER_KEY	0x40
#define RB_LOADER_IMPL_PARSER_TYPE	0x20
#define RB_LOADER_IMPL_PARSER_PARAM 0x10

typedef struct rb_function_parameter_parser_type
{
	int index;
	char name[RB_LOADER_IMPL_PARSER_KEY];
	char type[RB_LOADER_IMPL_PARSER_TYPE];

} * rb_function_parameter_parser;

typedef struct rb_function_parser_type
{
	char name[RB_LOADER_IMPL_PARSER_FUNC];
	struct rb_function_parameter_parser_type params[RB_LOADER_IMPL_PARSER_PARAM];
	size_t params_size;

} * rb_function_parser;

int rb_loader_impl_key_parse(const char *source, set function_map);

void rb_loader_impl_key_print(set function_map);

void rb_loader_impl_key_clear(set function_map);

#ifdef __cplusplus
}
#endif

#endif /* RB_LOADER_IMPL_PARSER_H */
