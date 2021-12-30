/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#include <benchmark/benchmark.h>

#include <log/log.h>
#include <metacall/metacall_loaders.h>

static int stream_write(void *, const char *, const size_t)
{
	// Disable stream write so we do not count stdout on the benchmark
	return 0;
}

static int stream_flush(void *)
{
	// Disable stream flush so we do not count stdout on the benchmark
	return 0;
}

class log_bench : public benchmark::Fixture
{
public:
	void SetUp(benchmark::State &state)
	{
		if (log_configure("metacall",
				log_policy_format_text(),
				log_policy_schedule_sync(),
				log_policy_storage_sequential(),
				log_policy_stream_custom(NULL, &stream_write, &stream_flush)) != 0)
		{
			state.SkipWithError("Error creating the log");
		}
	}

	void TearDown(benchmark::State &)
	{
	}
};

BENCHMARK_DEFINE_F(log_bench, call_macro)
(benchmark::State &state)
{
	const int64_t call_count = 10000;

	for (auto _ : state)
	{
		for (int64_t it = 0; it < call_count; ++it)
		{
			benchmark::DoNotOptimize(log_write("metacall", LOG_LEVEL_ERROR, "Message"));
		}
	}

	state.SetLabel("Log Benchmark - Call Macro");
	state.SetItemsProcessed(call_count);
}

BENCHMARK_REGISTER_F(log_bench, call_macro)
	->Threads(1)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(3);

BENCHMARK_DEFINE_F(log_bench, call_va)
(benchmark::State &state)
{
	const int64_t call_count = 10000;

	for (auto _ : state)
	{
		for (int64_t it = 0; it < call_count; ++it)
		{
			benchmark::DoNotOptimize(log_write_impl_va("metacall", LOG_PREPROCESSOR_LINE, log_record_function(), __FILE__, LOG_LEVEL_ERROR, "Message"));
		}
	}

	state.SetLabel("Log Benchmark - Call Variadic");
	state.SetItemsProcessed(call_count);
}

BENCHMARK_REGISTER_F(log_bench, call_va)
	->Threads(1)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(3);

BENCHMARK_MAIN();
