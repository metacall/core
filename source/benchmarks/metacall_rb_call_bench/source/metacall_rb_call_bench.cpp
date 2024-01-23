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

class metacall_rb_call_bench : public benchmark::Fixture
{
public:
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
	->Unit(benchmark::kMillisecond)
	->Iterations(1)
	->Repetitions(5);

/* Use main for initializing MetaCall once. There's a bug in Ruby 3.2 on MacOS which prevents reinitialization */
/*
	Stack trace (most recent call last):
	#18   Object "metacall-rb-call-bench", at 0x100363520, in main + 48
	#17   Object "metacall-rb-call-bench", at 0x1003647f8, in benchmark::RunSpecifiedBenchmarks() + 40
	#16   Object "metacall-rb-call-bench", at 0x10036561f, in benchmark::RunSpecifiedBenchmarks(benchmark::BenchmarkReporter*, benchmark::BenchmarkReporter*, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >) + 3567
	#15   Object "metacall-rb-call-bench", at 0x10037e408, in benchmark::internal::BenchmarkRunner::DoOneRepetition() + 136
	#14   Object "metacall-rb-call-bench", at 0x10037d97a, in benchmark::internal::BenchmarkRunner::DoNIterations() + 890
	#13   Object "metacall-rb-call-bench", at 0x10037dea7, in benchmark::internal::(anonymous namespace)::RunInThread(benchmark::internal::BenchmarkInstance const*, unsigned long long, int, benchmark::internal::ThreadManager*, benchmark::internal::PerfCountersMeasurement*) + 87
	#12   Object "metacall-rb-call-bench", at 0x10036af5f, in benchmark::internal::BenchmarkInstance::Run(unsigned long long, int, benchmark::internal::ThreadTimer*, benchmark::internal::ThreadManager*, benchmark::internal::PerfCountersMeasurement*) const + 79
	#11   Object "metacall-rb-call-bench", at 0x100363573, in benchmark::Fixture::Run(benchmark::State&) + 19
	#10   Object "metacall-rb-call-bench", at 0x100363605, in metacall_rb_call_bench::SetUp(benchmark::State&) + 69
	#9    Object "libmetacall.dylib", at 0x10050a077, in loader_impl_load_from_memory + 135
	#8    Object "libmetacall.dylib", at 0x10050955f, in loader_impl_initialize + 287
	#7    Object "librb_loader.so", at 0x10058b978, in rb_loader_impl_initialize + 56
	#6    Object "libruby.3.2.dylib", at 0x100c683d7, in ruby_init + 13
	#5    Object "libruby.3.2.dylib", at 0x100c6837b, in ruby_setup + 304
	#4    Object "libruby.3.2.dylib", at 0x100c90ff1, in rb_call_inits + 19
	#3    Object "libruby.3.2.dylib", at 0x100da121e, in Init_TransientHeap + 79
	#2    Object "libsystem_platform.dylib", at 0x7ff8088fbdfc, in _sigtramp + 28
	#1    Object "libbacktrace_plugin.so", at 0x100571b5d, in backward::SignalHandling::sig_handler(int, __siginfo*, void*) + 13
	#0    Object "libbacktrace_plugin.so", at 0x100571bc6, in backward::SignalHandling::handleSignal(int, __siginfo*, void*) + 70
*/
int main(int argc, char *argv[])
{
	metacall_print_info();

	metacall_log_null();

	if (metacall_initialize() != 0)
	{
		return 1;
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
			return 2;
		}
	}
#endif /* OPTION_BUILD_LOADERS_RB */

	::benchmark::Initialize(&argc, argv);

	if (::benchmark::ReportUnrecognizedArguments(argc, argv))
	{
		return 3;
	}

	::benchmark::RunSpecifiedBenchmarks();
	::benchmark::Shutdown();

	if (metacall_destroy() != 0)
	{
		return 4;
	}

	return 0;
}
