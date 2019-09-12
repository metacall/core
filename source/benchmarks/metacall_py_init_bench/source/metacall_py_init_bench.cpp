/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

class metacall_py_init_bench : public benchmark::Fixture
{
public:
	void SetUp(benchmark::State &)
	{
		metacall_print_info();

		metacall_log_null();
	}

	void TearDown(benchmark::State & state)
	{
		if (metacall_destroy() != 0)
		{
			state.SkipWithError("Error destroying MetaCall");
		}
	}
};

BENCHMARK_DEFINE_F(metacall_py_init_bench, init)(benchmark::State & state)
{
	for (auto _ : state)
	{
		/* Python */
		#if defined(OPTION_BUILD_LOADERS_PY)
		{
			if (metacall_initialize() != 0)
			{
				state.SkipWithError("Error initializing MetaCall");
			}
		}
		#endif /* OPTION_BUILD_LOADERS_PY */
	}

	state.SetLabel("MetaCall Python Init Benchmark - Init");
}

BENCHMARK_REGISTER_F(metacall_py_init_bench, init)
	->Threads(1)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(1);

BENCHMARK_DEFINE_F(metacall_py_init_bench, load)(benchmark::State & state)
{
	for (auto _ : state)
	{
		/* Python */
		#if defined(OPTION_BUILD_LOADERS_PY)
		{
			static const char tag[] = "py";

			static const char int_mem_type[] =
				"#!/usr/bin/python3.5\n"
				"def int_mem_type(left: int, right: int) -> int:\n"
				"\treturn 0;";

			state.PauseTiming();

			if (metacall_initialize() != 0)
			{
				state.SkipWithError("Error initializing MetaCall");
			}

			state.ResumeTiming();

			if (metacall_load_from_memory(tag, int_mem_type, sizeof(int_mem_type), NULL) != 0)
			{
				state.SkipWithError("Error loading int_mem_type function");
			}
		}
		#endif /* OPTION_BUILD_LOADERS_PY */
	}

	state.SetLabel("MetaCall Python Init Benchmark - Load Runtime");
}

BENCHMARK_REGISTER_F(metacall_py_init_bench, load)
	->Threads(1)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(1);

BENCHMARK_DEFINE_F(metacall_py_init_bench, load_warm)(benchmark::State & state)
{
	for (auto _ : state)
	{
		/* Python */
		#if defined(OPTION_BUILD_LOADERS_PY)
		{
			static const char tag[] = "py";

			static const char int_a_type[] =
				"#!/usr/bin/python3.5\n"
				"def int_a_type(left: int, right: int) -> int:\n"
				"\treturn 0;";
			static const char int_b_type[] =
				"#!/usr/bin/python3.5\n"
				"def int_b_type(left: int, right: int) -> int:\n"
				"\treturn 0;";

			state.PauseTiming();

			if (metacall_initialize() != 0)
			{
				state.SkipWithError("Error initializing MetaCall");
			}

			if (metacall_load_from_memory(tag, int_a_type, sizeof(int_a_type), NULL) != 0)
			{
				state.SkipWithError("Error loading int_a_type function");
			}

			state.ResumeTiming();

			if (metacall_load_from_memory(tag, int_b_type, sizeof(int_b_type), NULL) != 0)
			{
				state.SkipWithError("Error loading int_b_type function");
			}
		}
		#endif /* OPTION_BUILD_LOADERS_PY */
	}

	state.SetLabel("MetaCall Python Init Benchmark - Load Warm");
}

BENCHMARK_REGISTER_F(metacall_py_init_bench, load_warm)
	->Threads(1)
	->Unit(benchmark::kMicrosecond)
	->Iterations(1)
	->Repetitions(1);

BENCHMARK_MAIN();
