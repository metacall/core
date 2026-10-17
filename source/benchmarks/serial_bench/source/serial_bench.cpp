/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#include <benchmark/benchmark.h>

#include <serial/serial.h>

#include <log/log.h>

#include <string>

#define NUMBERS_SIZE  10000
#define STRINGS_SIZE  1000
#define NESTED_DEPTH  4
#define NESTED_LENGTH 8

/* Reference serial used for generating the values to be serialized */
#define SERIAL_BENCH_REFERENCE "rapid_json"

static memory_allocator allocator = NULL;

static std::string payload_small()
{
	return "{\"id\":1234,\"name\":\"metacall\",\"active\":true,\"score\":3.14,\"tags\":[\"a\",\"b\",\"c\"],\"owner\":null}";
}

static std::string payload_numbers()
{
	std::string json = "[";

	for (int i = 0; i < NUMBERS_SIZE; ++i)
	{
		if (i > 0)
		{
			json += ",";
		}

		if (i % 2 == 0)
		{
			json += std::to_string(i * 7919);
		}
		else
		{
			json += std::to_string(i) + "." + std::to_string(i % 1000);
		}
	}

	return json + "]";
}

static std::string payload_strings()
{
	std::string json = "[";

	for (int i = 0; i < STRINGS_SIZE; ++i)
	{
		if (i > 0)
		{
			json += ",";
		}

		json += "\"The quick brown fox \\\"jumps\\\" over the lazy dog\\n" + std::to_string(i) + " \\\\ end\"";
	}

	return json + "]";
}

static std::string payload_nested_level(int depth)
{
	std::string json = "{";

	for (int i = 0; i < NESTED_LENGTH; ++i)
	{
		if (i > 0)
		{
			json += ",";
		}

		json += "\"key_" + std::to_string(i) + "\":";

		if (depth == 0)
		{
			json += std::to_string(i);
		}
		else
		{
			json += payload_nested_level(depth - 1);
		}
	}

	return json + "}";
}

static std::string payload_nested()
{
	return payload_nested_level(NESTED_DEPTH);
}

static void serial_bench_serialize(benchmark::State &state, const char *name, std::string (*payload)())
{
	const std::string json = payload();

	value v = serial_deserialize(serial_create(SERIAL_BENCH_REFERENCE), json.c_str(), json.size() + 1, allocator);

	if (v == NULL)
	{
		state.SkipWithError("Failed to initialize the serialization benchmark value");
		return;
	}

	serial s = serial_create(name);

	if (s == NULL)
	{
		state.SkipWithError("Failed to initialize the serialization benchmark serial");
		value_type_destroy(v);
		return;
	}

	size_t total = 0;

	for (auto _ : state)
	{
		size_t size = 0;

		/* Do not wrap the result in benchmark::DoNotOptimize, it is already consumed by the deallocation
		* and GCC with AddressSanitizer miscompiles the "+m,r" constraint, leaving the local uninitialized */
		char *buffer = serial_serialize(s, v, &size, allocator);

		if (buffer == NULL)
		{
			state.SkipWithError("Failed to serialize the value");
			break;
		}

		total += size;

		memory_allocator_deallocate(allocator, buffer);
	}

	state.SetBytesProcessed((int64_t)total);

	value_type_destroy(v);
}

static void serial_bench_deserialize(benchmark::State &state, const char *name, std::string (*payload)())
{
	const std::string json = payload();

	serial s = serial_create(name);

	if (s == NULL)
	{
		state.SkipWithError("Failed to initialize the deserialization benchmark");
		return;
	}

	for (auto _ : state)
	{
		value v = serial_deserialize(s, json.c_str(), json.size() + 1, allocator);

		if (v == NULL)
		{
			state.SkipWithError("Failed to deserialize the buffer");
			break;
		}

		value_type_destroy(v);
	}

	state.SetBytesProcessed((int64_t)state.iterations() * (int64_t)json.size());
}

#define SERIAL_BENCH_REGISTER(name) \
	BENCHMARK_CAPTURE(serial_bench_serialize, name##_small, #name, &payload_small)->Unit(benchmark::kMicrosecond); \
	BENCHMARK_CAPTURE(serial_bench_serialize, name##_numbers, #name, &payload_numbers)->Unit(benchmark::kMicrosecond); \
	BENCHMARK_CAPTURE(serial_bench_serialize, name##_strings, #name, &payload_strings)->Unit(benchmark::kMicrosecond); \
	BENCHMARK_CAPTURE(serial_bench_serialize, name##_nested, #name, &payload_nested)->Unit(benchmark::kMicrosecond); \
	BENCHMARK_CAPTURE(serial_bench_deserialize, name##_small, #name, &payload_small)->Unit(benchmark::kMicrosecond); \
	BENCHMARK_CAPTURE(serial_bench_deserialize, name##_numbers, #name, &payload_numbers)->Unit(benchmark::kMicrosecond); \
	BENCHMARK_CAPTURE(serial_bench_deserialize, name##_strings, #name, &payload_strings)->Unit(benchmark::kMicrosecond); \
	BENCHMARK_CAPTURE(serial_bench_deserialize, name##_nested, #name, &payload_nested)->Unit(benchmark::kMicrosecond)

SERIAL_BENCH_REGISTER(rapid_json);

#if defined(SERIAL_BENCH_SIMD_JSON)
SERIAL_BENCH_REGISTER(simd_json);
#endif

int main(int argc, char **argv)
{
	if (log_configure("metacall",
			log_policy_format_text(),
			log_policy_schedule_sync(),
			log_policy_storage_sequential(),
			log_policy_stream_stdio(stdout)) != 0)
	{
		return 1;
	}

	allocator = memory_allocator_std(&malloc, &realloc, &free);

	if (allocator == NULL)
	{
		return 2;
	}

	if (serial_initialize() != 0)
	{
		return 3;
	}

	::benchmark::Initialize(&argc, argv);

	if (::benchmark::ReportUnrecognizedArguments(argc, argv))
	{
		return 4;
	}

	::benchmark::RunSpecifiedBenchmarks();
	::benchmark::Shutdown();

	serial_destroy();

	memory_allocator_destroy(allocator);

	return 0;
}
