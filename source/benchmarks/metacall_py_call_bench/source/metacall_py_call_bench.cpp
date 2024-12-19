/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#include <benchmark/benchmark.h>

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

class metacall_py_call_bench : public benchmark::Fixture
{
public:
};

BENCHMARK_DEFINE_F(metacall_py_call_bench, call_va_args)
(benchmark::State &state)
{
	const int64_t call_count = 1000000;
	const int64_t call_size = sizeof(long) * 3; // (long, long) -> long

	for (auto _ : state)
	{
/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
		{
			for (int64_t it = 0; it < call_count; ++it)
			{
				void *ret = metacall("int_mem_type", 0L, 0L);

				state.PauseTiming();

				if (ret == NULL)
				{
					state.SkipWithError("Null return value from int_mem_type");
				}

				if (metacall_value_to_long(ret) != 0L)
				{
					state.SkipWithError("Invalid return value from int_mem_type");
				}

				metacall_value_destroy(ret);

				state.ResumeTiming();
			}
		}
#endif /* OPTION_BUILD_LOADERS_PY */
	}

	state.SetLabel("MetaCall Python Call Benchmark - Variadic Argument Call");
	state.SetBytesProcessed(call_size * call_count);
	state.SetItemsProcessed(call_count);
}

BENCHMARK_REGISTER_F(metacall_py_call_bench, call_va_args)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(5);

BENCHMARK_DEFINE_F(metacall_py_call_bench, call_array_args)
(benchmark::State &state)
{
	const int64_t call_count = 1000000;
	const int64_t call_size = sizeof(long) * 3; // (long, long) -> long

	for (auto _ : state)
	{
/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
		{
			state.PauseTiming();

			void *args[2] = {
				metacall_value_create_long(0L),
				metacall_value_create_long(0L)
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

				if (metacall_value_to_long(ret) != 0L)
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
#endif /* OPTION_BUILD_LOADERS_PY */
	}

	state.SetLabel("MetaCall Python Call Benchmark - Array Argument Call");
	state.SetBytesProcessed(call_size * call_count);
	state.SetItemsProcessed(call_count);
}

BENCHMARK_REGISTER_F(metacall_py_call_bench, call_array_args)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(5);

/* Use main for initializing MetaCall once. There's a bug in Python async which prevents reinitialization */
/* https://github.com/python/cpython/issues/89425 */
/* https://bugs.python.org/issue45262 */
/* metacall-py-call-benchd: ./Modules/_asynciomodule.c:261: get_running_loop: Assertion `Py_IS_TYPE(rl, &PyRunningLoopHolder_Type)' failed. */
int main(int argc, char *argv[])
{
	metacall_print_info();

	metacall_log_null();

	if (metacall_initialize() != 0)
	{
		return 1;
	}

/* Python */
#if defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char tag[] = "py";

		static const char int_mem_type[] =
			"#!/usr/bin/env python3\n"
			"def int_mem_type(left: int, right: int) -> int:\n"
			"\treturn 0;";

		if (metacall_load_from_memory(tag, int_mem_type, sizeof(int_mem_type), NULL) != 0)
		{
			return 2;
		}
	}
#endif /* OPTION_BUILD_LOADERS_PY */

	::benchmark::Initialize(&argc, argv);

	if (::benchmark::ReportUnrecognizedArguments(argc, argv))
	{
		return 3;
	}

	::benchmark::RunSpecifiedBenchmarks();
	::benchmark::Shutdown();

	metacall_destroy();

	return 0;
}
