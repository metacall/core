/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading ruby code at run-time into a process.
 *
 */

#ifndef RB_LOADER_IMPL_KEY_H
#define RB_LOADER_IMPL_KEY_H 1

#include <rb_loader/rb_loader_api.h>

#include <adt/adt_set.h>

#ifdef __cplusplus
extern "C" {
#endif
/*#include <string>
#include <vector>
#include <map>

struct js_parameter;

typedef std::vector<js_parameter> parameter_list;

struct js_parameter
{
	int index;
	std::string name;
	std::string type;
};

struct js_function
{
	std::string name;
	std::string return_type;
	parameter_list parameters;
};*/


RB_LOADER_API int rb_loader_impl_key_parse(const char * source, set function_map);

#ifdef __cplusplus
}
#endif

#endif /* RB_LOADER_IMPL_KEY_H */
