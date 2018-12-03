
/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A plugin for loading javascript code at run-time into a process.
 *
 */

#ifndef JS_LOADER_IMPL_GUARD_HPP
#define JS_LOADER_IMPL_GUARD_HPP 1

#include <js_loader/js_loader_api.h>

#ifndef __cplusplus
extern "C++" {
#endif

#include <string>
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
};

JS_LOADER_API bool js_loader_impl_guard_parse(std::string & source, std::map<std::string, js_function *> & result, std::string & output);

#ifndef __cplusplus
}
#endif

#endif /* JS_LOADER_IMPL_GUARD_HPP */
