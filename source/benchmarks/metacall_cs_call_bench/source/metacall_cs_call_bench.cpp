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

class metacall_cs_call_bench : public benchmark::Fixture
{
public:
};

BENCHMARK_DEFINE_F(metacall_cs_call_bench, call_va_args)
(benchmark::State &state)
{
	const int64_t call_count = 500000;
	const int64_t call_size = sizeof(int) * 3; // (int, int) -> int

	for (auto _ : state)
	{
/* CSharp */
#if defined(OPTION_BUILD_LOADERS_CS)
		{
			for (int64_t it = 0; it < call_count; ++it)
			{
				void *ret = metacall("sum", 0, 0);

				state.PauseTiming();

				if (ret == NULL)
				{
					state.SkipWithError("Null return value from sum");
				}

				if (metacall_value_to_int(ret) != 0)
				{
					state.SkipWithError("Invalid return value from sum");
				}

				metacall_value_destroy(ret);

				state.ResumeTiming();
			}
		}
#endif /* OPTION_BUILD_LOADERS_CS */
	}

	state.SetLabel("MetaCall CSharp Call Benchmark - Variadic Argument Call");
	state.SetBytesProcessed(call_size * call_count);
	state.SetItemsProcessed(call_count);
}

BENCHMARK_REGISTER_F(metacall_cs_call_bench, call_va_args)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(5);

BENCHMARK_DEFINE_F(metacall_cs_call_bench, call_array_args)
(benchmark::State &state)
{
	const int64_t call_count = 500000;
	const int64_t call_size = sizeof(int) * 3; // (int, int) -> int

	for (auto _ : state)
	{
/* CSharp */
#if defined(OPTION_BUILD_LOADERS_CS)
		{
			state.PauseTiming();

			void *args[2] = {
				metacall_value_create_int(0),
				metacall_value_create_int(0)
			};

			state.ResumeTiming();

			for (int64_t it = 0; it < call_count; ++it)
			{
				void *ret = metacallv("sum", args);

				state.PauseTiming();

				if (ret == NULL)
				{
					state.SkipWithError("Null return value from sum");
				}

				if (metacall_value_to_int(ret) != 0)
				{
					state.SkipWithError("Invalid return value from sum");
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
#endif /* OPTION_BUILD_LOADERS_CS */
	}

	state.SetLabel("MetaCall CSharp Call Benchmark - Array Argument Call");
	state.SetBytesProcessed(call_size * call_count);
	state.SetItemsProcessed(call_count);
}

BENCHMARK_REGISTER_F(metacall_cs_call_bench, call_array_args)
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(5);

/* TODO: NetCore re-initialization */
/* BENCHMARK_MAIN(); */

int main(int argc, char **argv)
{
	::benchmark::Initialize(&argc, argv);

	if (::benchmark::ReportUnrecognizedArguments(argc, argv))
	{
		return 1;
	}

	/* TODO: MetaCall NetCore Loader does not work with re-initalization */
	/* Maybe the bug cannot be solved, but if it is eventually solved, */
	/* use SetUp and TearDown in the Fixture instead of this */

	metacall_print_info();

	metacall_log_null();

	if (metacall_initialize() != 0)
	{
		return 1;
	}

/* CSharp */
#if defined(OPTION_BUILD_LOADERS_CS)
	{
		static const char tag[] = "cs";

		static const char sum[] =
			"using System;\n"
			"namespace Scripts {\n"
			"\tpublic class Program {\n"
			"\t\tpublic static int sum(int a, int b) {\n"
			"\t\t\treturn 0;\n"
			"\t\t}\n"
			"\t}\n"
			"}\n";

		if (metacall_load_from_memory(tag, sum, sizeof(sum), NULL) != 0)
		{
			metacall_destroy();
			return 1;
		}
	}
#endif /* OPTION_BUILD_LOADERS_CS */

	::benchmark::RunSpecifiedBenchmarks();

	metacall_destroy();

	return 0;
}
