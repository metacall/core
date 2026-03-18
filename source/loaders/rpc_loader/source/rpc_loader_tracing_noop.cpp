/*
 *	Loader Library by Parra Studios
 *	A plugin for loading rpc endpoints at run-time into a process.
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

/* No-op tracing implementation.
 * Compiled when OPTION_RPC_TRACING is OFF (the default).
 * All methods are empty so that would mean zero runtime overhead. */

#include <rpc_loader/rpc_loader_tracing.h>

struct rpc_trace_scope::impl
{
};

rpc_trace_scope::rpc_trace_scope(const char *function_name, const char *target_url, bool is_async)
{
	(void)function_name;
	(void)target_url;
	(void)is_async;
	pimpl = nullptr;
}

void rpc_trace_scope::set_error(const char *error_message)
{
	(void)error_message;
}

rpc_trace_scope::~rpc_trace_scope()
{
}

void rpc_tracing_initialize(void)
{
}

void rpc_tracing_shutdown(void)
{
}
