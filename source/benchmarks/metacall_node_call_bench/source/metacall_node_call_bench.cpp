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

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

class metacall_node_call_bench : public benchmark::Fixture
{
public:
};

BENCHMARK_DEFINE_F(metacall_node_call_bench, call_va_args)(benchmark::State & state)
{
	const int64_t call_count = 100000;
	const int64_t call_size = sizeof(double) * 3; // (double, double) -> double
	const enum metacall_value_id int_mem_type_ids[] =
	{
		METACALL_DOUBLE, METACALL_DOUBLE
	};

	for (auto _ : state)
	{
		/* NodeJS */
		#if defined(OPTION_BUILD_LOADERS_NODE)
		{
			void * ret;

			for (int64_t it = 0; it < call_count; ++it)
			{
				benchmark::DoNotOptimize(ret = metacallt("int_mem_type", int_mem_type_ids, 0.0, 0.0));

				state.PauseTiming();

				if (ret == NULL)
				{
					state.SkipWithError("Null return value from int_mem_type");
				}

				if (metacall_value_to_double(ret) != 0.0)
				{
					state.SkipWithError("Invalid return value from int_mem_type");
				}

				metacall_value_destroy(ret);

				state.ResumeTiming();
			}
		}
		#endif /* OPTION_BUILD_LOADERS_NODE */
	}

	state.SetLabel("MetaCall NodeJS Call Benchmark - Variadic Argument Call");
	state.SetBytesProcessed(call_size * call_count);
	state.SetItemsProcessed(call_count);
}

BENCHMARK_REGISTER_F(metacall_node_call_bench, call_va_args)
	->Threads(1)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(3);

BENCHMARK_DEFINE_F(metacall_node_call_bench, call_array_args)(benchmark::State & state)
{
	const int64_t call_count = 100000;
	const int64_t call_size = sizeof(double) * 3; // (double, double) -> double

	for (auto _ : state)
	{
		/* NodeJS */
		#if defined(OPTION_BUILD_LOADERS_NODE)
		{
			void * ret;

			state.PauseTiming();

			void * args[2] =
			{
				metacall_value_create_double(0.0),
				metacall_value_create_double(0.0)
			};

			state.ResumeTiming();

			for (int64_t it = 0; it < call_count; ++it)
			{
				benchmark::DoNotOptimize(ret = metacallv("int_mem_type", args));

				state.PauseTiming();

				if (ret == NULL)
				{
					state.SkipWithError("Null return value from int_mem_type");
				}

				if (metacall_value_to_double(ret) != 0.0)
				{
					state.SkipWithError("Invalid return value from int_mem_type");
				}

				metacall_value_destroy(ret);

				state.ResumeTiming();
			}

			state.PauseTiming();

			for (auto arg : args)
			{
				metacall_value_destroy(arg);
			}

			state.ResumeTiming();
		}
		#endif /* OPTION_BUILD_LOADERS_NODE */
	}

	state.SetLabel("MetaCall NodeJS Call Benchmark - Array Argument Call");
	state.SetBytesProcessed(call_size * call_count);
	state.SetItemsProcessed(call_count);
}

BENCHMARK_REGISTER_F(metacall_node_call_bench, call_array_args)
	->Threads(1)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(3);

BENCHMARK_DEFINE_F(metacall_node_call_bench, call_async)(benchmark::State & state)
{
	const int64_t call_count = 100000;
	const int64_t call_size = sizeof(double) * 3; // (double, double) -> double

	for (auto _ : state)
	{
		/* NodeJS */
		#if defined(OPTION_BUILD_LOADERS_NODE)
		{
			void * ret;

			state.PauseTiming();

			void * args[2] =
			{
				metacall_value_create_double(0.0),
				metacall_value_create_double(0.0)
			};

			state.ResumeTiming();

			for (int64_t it = 0; it < call_count; ++it)
			{
				benchmark::DoNotOptimize(ret = metacall_await("int_mem_async_type", args, [](void * result, void * data) -> void * {
					benchmark::State * state = static_cast<benchmark::State *>(data);

					if (metacall_value_to_double(result) != 0.0)
					{
						state->SkipWithError("Invalid return value from int_mem_async_type");
					}

					state->PauseTiming();

					return NULL;
				}, NULL, static_cast<void *>(&state)));

				if (ret == NULL)
				{
					state.SkipWithError("Null return value from int_mem_async_type");
				}

				if (metacall_value_id(ret) != METACALL_FUTURE)
				{
					state.SkipWithError("Invalid return type from int_mem_async_type");
				}

				metacall_value_destroy(ret);

				state.ResumeTiming();
			}

			state.PauseTiming();

			for (auto arg : args)
			{
				metacall_value_destroy(arg);
			}

			state.ResumeTiming();
		}
		#endif /* OPTION_BUILD_LOADERS_NODE */
	}

	state.SetLabel("MetaCall NodeJS Call Benchmark - Async Call");
	state.SetBytesProcessed(call_size * call_count);
	state.SetItemsProcessed(call_count);
}

BENCHMARK_REGISTER_F(metacall_node_call_bench, call_async)
	->Threads(1)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(3);

/* TODO: NodeJS re-initialization */
/* BENCHMARK_MAIN(); */

int main(int argc, char ** argv)
{
	::benchmark::Initialize(&argc, argv);

	if (::benchmark::ReportUnrecognizedArguments(argc, argv))
	{
		return 1;
	}

	/* TODO: MetaCall NodeJS Loader does not work with re-initalization */
	/* Maybe the bug cannot be solved, but if it is eventually solved, */
	/* use SetUp and TearDown in the Fixture instead of this */

	metacall_print_info();

	metacall_log_null();

	if (metacall_initialize() != 0)
	{
		return 1;
	}

	/* NodeJS */
	#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		static const char tag[] = "node";

		static const char int_mem_type[] =
			"#!/usr/bin/env node\n"
			"module.exports = {\n"
			"	int_mem_type: (left, right) => 0,\n"
			"	int_mem_async_type: async (left, right) => new Promise(resolve => 0),\n"
			"};\n";

		if (metacall_load_from_memory(tag, int_mem_type, sizeof(int_mem_type), NULL) != 0)
		{
			metacall_destroy();
			return 1;
		}
	}
	#endif /* OPTION_BUILD_LOADERS_NODE */

	::benchmark::RunSpecifiedBenchmarks();

	return metacall_destroy();
}
