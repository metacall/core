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

#ifndef RPC_LOADER_TRACING_H
#define RPC_LOADER_TRACING_H 1

#include <rpc_loader/rpc_loader_api.h>

/* RAII tracing scope for RPC loader calls.
 *
 * When OPTION_RPC_TRACING is enabled, this emits OpenTelemetry spans.
 * When disabled, all methods are compiled as no-ops (zero overhead).
 */
class rpc_trace_scope
{
public:
	rpc_trace_scope(const char *function_name, const char *target_url, bool is_async);

	void set_error(const char *error_message);

	~rpc_trace_scope();

	rpc_trace_scope(const rpc_trace_scope &) = delete;
	rpc_trace_scope &operator=(const rpc_trace_scope &) = delete;

private:
	struct impl;
	impl *pimpl;
};

/* One-time global initialization / shutdown for the tracing subsystem.
 * Called from rpc_loader_impl_initialize / rpc_loader_impl_destroy.
 * No-ops when tracing is disabled at compile time. */
void rpc_tracing_initialize(void);
void rpc_tracing_shutdown(void);

#endif /* RPC_LOADER_TRACING_H */
