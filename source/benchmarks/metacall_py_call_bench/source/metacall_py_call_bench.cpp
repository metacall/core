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

class metacall_py_call_bench : public benchmark::Fixture
{
public:
	void SetUp(benchmark::State & state)
	{
		metacall_log_stdio_type log_stdio = { stdout };

		metacall_print_info();

		if (metacall_log(METACALL_LOG_STDIO, static_cast<void *>(&log_stdio)) != 0)
		{
			state.SkipWithError("Error initializing MetaCall Log");
		}

		if (metacall_initialize() != 0)
		{
			state.SkipWithError("Error initializing MetaCall");
		}

		/* Python */
		#if defined(OPTION_BUILD_LOADERS_PY)
		{
			static const char tag[] = "py";

			static const char int_mem_type[] =
				"#!/usr/bin/python3.5\n"
				"def int_mem_type(left: int, right: int) -> int:\n"
				"\treturn 0;";

			if (metacall_load_from_memory(tag, int_mem_type, sizeof(int_mem_type), NULL) != 0)
			{
				state.SkipWithError("Error loading int_mem_type function");
			}
		}
		#endif /* OPTION_BUILD_LOADERS_PY */
	}

	void TearDown(benchmark::State & state)
	{
		if (metacall_destroy() != 0)
		{
			state.SkipWithError("Error destroying MetaCall");
		}
	}
};

BENCHMARK_DEFINE_F(metacall_py_call_bench, va_args_call)(benchmark::State & state)
{
	/*
	const int64_t call_count = 10000;
	const int64_t call_size = sizeof(long) * 3; // (long, long) -> long
	*/

	for (auto _ : state)
	{
		/* Python */
		#if defined(OPTION_BUILD_LOADERS_PY)
		{
			void * ret;

			/*
			benchmark::DoNotOptimize(ret = metacall("int_mem_type", 0L, 0L));
			*/

			ret = metacall("int_mem_type", 0L, 0L);

			/*
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
			*/
		}
		#endif /* OPTION_BUILD_LOADERS_PY */
	}

	/*
	state.SetLabel("MetaCall Python Call Benchmark - Variadic Argument Call");
	state.SetBytesProcessed(call_size * call_count);
	state.SetItemsProcessed(call_count);
	*/
}

BENCHMARK_REGISTER_F(metacall_py_call_bench, va_args_call)
	->Threads(1)
	->Unit(benchmark::kNanosecond)
	/*->Iterations(10000)*/
	->Iterations(1)
	/*->Repetitions(10)*/;
	->Repetitions(1);

BENCHMARK_MAIN();
