
/*
 *	Loader Library by Parra Studios
 *	A plugin for loading javascript code at run-time into a process.
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

#ifndef JS_LOADER_IMPL_GUARD_HPP
#define JS_LOADER_IMPL_GUARD_HPP 1

#include <js_loader/js_loader_api.h>

#ifndef __cplusplus
extern "C++" {
#endif

#include <map>
#include <string>
#include <vector>

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

JS_LOADER_API bool js_loader_impl_guard_parse(std::string &source, std::map<std::string, js_function *> &result, std::string &output);

#ifndef __cplusplus
}
#endif

#endif /* JS_LOADER_IMPL_GUARD_HPP */
