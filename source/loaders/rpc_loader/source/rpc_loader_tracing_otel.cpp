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

/* OpenTelemetry-backed tracing implementation.
 * Compiled when OPTION_RPC_TRACING is ON.
 * Uses OStreamSpanExporter (console) for v1 */

#include <rpc_loader/rpc_loader_tracing.h>

#include <opentelemetry/exporters/ostream/span_exporter_factory.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/noop.h>
#include <opentelemetry/trace/scope.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/tracer.h>

#include <memory>

namespace trace_api = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace trace_exp = opentelemetry::exporter::trace;

static const char *TRACER_NAME = "metacall.rpc_loader";

static opentelemetry::nostd::shared_ptr<trace_api::Tracer> get_tracer()
{
	return trace_api::Provider::GetTracerProvider()->GetTracer(TRACER_NAME, "0.1.0");
}

struct rpc_trace_scope::impl
{
	opentelemetry::nostd::shared_ptr<trace_api::Span> span;
};

rpc_trace_scope::rpc_trace_scope(const char *function_name, const char *target_url, bool is_async)
{
	pimpl = new impl();

	auto tracer = get_tracer();
	pimpl->span = tracer->StartSpan(function_name);

	pimpl->span->SetAttribute("rpc.system", "metacall");
	pimpl->span->SetAttribute("rpc.method", function_name);
	pimpl->span->SetAttribute("metacall.target", target_url);
	pimpl->span->SetAttribute("metacall.loader", "rpc");
	pimpl->span->SetAttribute("metacall.async", is_async);
}

void rpc_trace_scope::set_error(const char *error_message)
{
	if (pimpl != nullptr && pimpl->span != nullptr)
	{
		pimpl->span->SetStatus(trace_api::StatusCode::kError, error_message);
		pimpl->span->SetAttribute("error.message", error_message);
	}
}

void rpc_trace_scope::set_attribute(const char *key, const char *value)
{
	if (pimpl != nullptr && pimpl->span != nullptr)
	{
		pimpl->span->SetAttribute(key, value);
	}
}

rpc_trace_scope::~rpc_trace_scope()
{
	if (pimpl != nullptr)
	{
		if (pimpl->span != nullptr)
		{
			pimpl->span->End();
		}
		delete pimpl;
	}
}

void rpc_tracing_initialize(void)
{
	auto exporter = trace_exp::OStreamSpanExporterFactory::Create();

	auto processor = trace_sdk::SimpleSpanProcessorFactory::Create(std::move(exporter));

	std::vector<std::unique_ptr<trace_sdk::SpanProcessor>> processors;
	processors.push_back(std::move(processor));

	auto provider = trace_sdk::TracerProviderFactory::Create(std::move(processors));

	std::shared_ptr<trace_api::TracerProvider> shared_provider(std::move(provider));
	trace_api::Provider::SetTracerProvider(shared_provider);
}

void rpc_tracing_shutdown(void)
{
	auto noop = opentelemetry::nostd::shared_ptr<trace_api::TracerProvider>(new trace_api::NoopTracerProvider());
	trace_api::Provider::SetTracerProvider(noop);
}
