/*
 *	Backtrace Plugin by Parra Studios
 *	A plugin implementing backtracing functionality for MetaCall Core.
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

#include <backtrace_plugin/backtrace_plugin.h>

#include <backward.hpp>

#include <log/log.h>

static backward::SignalHandling signal_handling;

int backtrace_plugin(void *loader, void *handle)
{
	(void)loader;
	(void)handle;

	if (signal_handling.loaded() == false)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Backtrace plugin failed to load, you need unwind/libunwind for stacktracing and libbfd/libdw/libdwarf for the debug information. Install the required libraries and recompile to utilise the backtrace plugin. For more information visit https://github.com/bombela/backward-cpp");
		return 1;
	}

	return 0;
}
