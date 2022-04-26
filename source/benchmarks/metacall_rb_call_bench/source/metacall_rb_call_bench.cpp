/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_rb_call_bench : public benchmark::Fixture
{
public:
	void SetUp(benchmark::State &state)
	{
		metacall_print_info();

		metacall_log_null();

		if (metacall_initialize() != 0)
		{
			state.SkipWithError("Error initializing MetaCall");
		}

/* Ruby */
#if defined(OPTION_BUILD_LOADERS_RB)
		{
			static const char tag[] = "rb";

			static const char int_mem_type[] =
				"#!/usr/bin/env ruby\n"
				"def int_mem_type(left: Fixnum, right: Fixnum)\n"
				"\treturn 0\n"
				"end\n";

			if (metacall_load_from_memory(tag, int_mem_type, sizeof(int_mem_type), NULL) != 0)
			{
				state.SkipWithError("Error loading int_mem_type function");
			}
		}
#endif /* OPTION_BUILD_LOADERS_RB */
	}

	void TearDown(benchmark::State &state)
	{
		if (metacall_destroy() != 0)
		{
			state.SkipWithError("Error destroying MetaCall");
		}
	}
};

BENCHMARK_DEFINE_F(metacall_rb_call_bench, call_va_args)
(benchmark::State &state)
{
	const int64_t call_count = 1000000;
	const int64_t call_size = sizeof(int) * 3; // (int, int) -> int

	for (auto _ : state)
	{
/* Ruby */
#if defined(OPTION_BUILD_LOADERS_RB)
		{
			for (int64_t it = 0; it < call_count; ++it)
			{
				void *ret = metacall("int_mem_type", 0, 0);

				state.PauseTiming();

				if (ret == NULL)
				{
					state.SkipWithError("Null return value from int_mem_type");
				}

				if (metacall_value_to_int(ret) != 0)
				{
					state.SkipWithError("Invalid return value from int_mem_type");
				}

				metacall_value_destroy(ret);

				state.ResumeTiming();
			}
		}
#endif /* OPTION_BUILD_LOADERS_RB */
	}

	state.SetLabel("MetaCall Ruby Call Benchmark - Variadic Argument Call");
	state.SetBytesProcessed(call_size * call_count);
	state.SetItemsProcessed(call_count);
}

BENCHMARK_REGISTER_F(metacall_rb_call_bench, call_va_args)
	->Threads(1)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(5);

BENCHMARK_DEFINE_F(metacall_rb_call_bench, call_array_args)
(benchmark::State &state)
{
	const int64_t call_count = 1000000;
	const int64_t call_size = sizeof(int) * 3; // (int, int) -> int

	for (auto _ : state)
	{
/* Ruby */
#if defined(OPTION_BUILD_LOADERS_RB)
		{
			state.PauseTiming();

			void *args[2] = {
				metacall_value_create_int(0),
				metacall_value_create_int(0)
			};

			state.ResumeTiming();

			for (int64_t it = 0; it < call_count; ++it)
			{
				void *ret = metacallv("int_mem_type", args);

				state.PauseTiming();

				if (ret == NULL)
				{
					state.SkipWithError("Null return value from int_mem_type");
				}

				if (metacall_value_to_int(ret) != 0)
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
#endif /* OPTION_BUILD_LOADERS_RB */
	}

	state.SetLabel("MetaCall Ruby Call Benchmark - Array Argument Call");
	state.SetBytesProcessed(call_size * call_count);
	state.SetItemsProcessed(call_count);
}

BENCHMARK_REGISTER_F(metacall_rb_call_bench, call_array_args)
	->Threads(1)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(5);

BENCHMARK_MAIN();
