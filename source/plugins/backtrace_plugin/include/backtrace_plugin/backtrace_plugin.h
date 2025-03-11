/*
 *	Backtrace Plugin by Parra Studios
 *	A plugin implementing backtracing functionality for MetaCall Core.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef BACKTRACE_PLUGIN_H
#define BACKTRACE_PLUGIN_H 1

#include <backtrace_plugin/backtrace_plugin_api.h>

#ifdef __cplusplus
extern "C" {
#endif

BACKTRACE_PLUGIN_API int backtrace_plugin(void *loader, void *handle);

#ifdef __cplusplus
}
#endif

#endif /* BACKTRACE_PLUGIN_H */
