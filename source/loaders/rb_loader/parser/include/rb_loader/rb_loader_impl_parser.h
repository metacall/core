/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading ruby code at run-time into a process.
 *
 */

#ifndef RB_LOADER_IMPL_PARSER_H
#define RB_LOADER_IMPL_PARSER_H 1

#include <rb_loader/rb_loader_api.h>

#include <adt/adt_set.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RB_LOADER_IMPL_PARSER_FUNC	0x40
#define RB_LOADER_IMPL_PARSER_KEY	0x40
#define RB_LOADER_IMPL_PARSER_TYPE	0x20
#define RB_LOADER_IMPL_PARSER_PARAM	0x10

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

RB_LOADER_API int rb_loader_impl_key_parse(const char * source, set function_map);

RB_LOADER_API void rb_loader_impl_key_print(set function_map);

RB_LOADER_API int rb_loader_impl_key_clear(set function_map);

#ifdef __cplusplus
}
#endif

#endif /* RB_LOADER_IMPL_PARSER_H */
