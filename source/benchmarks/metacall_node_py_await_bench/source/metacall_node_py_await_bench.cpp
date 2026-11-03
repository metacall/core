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

#include <metacall/metacall.h>
#include <metacall/metacall_loaders.h>

#include <condition_variable>
#include <mutex>

class metacall_node_py_await_bench : public benchmark::Fixture
{
public:
};

struct metacall_node_py_await_bench_wait
{
	std::mutex mutex;
	std::condition_variable cv;
	bool done = false;
	double result = 0.0;
	bool error = false;
};

static void *metacall_node_py_await_bench_finish(void *result, void *data, bool error)
{
	metacall_node_py_await_bench_wait *wait = static_cast<metacall_node_py_await_bench_wait *>(data);

	std::unique_lock<std::mutex> lock(wait->mutex);

	wait->error = error;

	if (error == false && metacall_value_id(result) == METACALL_DOUBLE)
	{
		wait->result = metacall_value_to_double(result);
	}

	wait->done = true;

	wait->cv.notify_one();

	return NULL;
}

static void *metacall_node_py_await_bench_resolve(void *result, void *data)
{
	return metacall_node_py_await_bench_finish(result, data, false);
}

static void *metacall_node_py_await_bench_reject(void *result, void *data)
{
	return metacall_node_py_await_bench_finish(result, data, true);
}

BENCHMARK_DEFINE_F(metacall_node_py_await_bench, node_py_await_stress)
(benchmark::State &state)
{
	/* Amount of awaits and chains launched at once, each one creates its own async handle */
	const int64_t size = state.range(0);

	for (auto _ : state)
	{
/* NodeJS & Python */
#if defined(OPTION_BUILD_LOADERS_NODE) && defined(OPTION_BUILD_LOADERS_PY)
		{
			metacall_node_py_await_bench_wait wait;

			void *args[] = {
				metacall_value_create_double(static_cast<double>(size))
			};

			void *ret = metacall_await("node_py_await_stress", args, &metacall_node_py_await_bench_resolve, &metacall_node_py_await_bench_reject, static_cast<void *>(&wait));

			if (ret == NULL || metacall_value_id(ret) != METACALL_FUTURE)
			{
				state.SkipWithError("Invalid return value from node_py_await_stress");
			}
			else
			{
				/* Wait until all the awaits and chains have been resolved */
				std::unique_lock<std::mutex> lock(wait.mutex);

				wait.cv.wait(lock, [&wait] { return wait.done; });

				if (wait.error == true || wait.result != static_cast<double>(size * 2))
				{
					state.SkipWithError("Invalid result from node_py_await_stress");
				}
			}

			if (ret != NULL)
			{
				metacall_value_destroy(ret);
			}

			metacall_value_destroy(args[0]);
		}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY */
	}

	state.SetLabel("MetaCall NodeJS Python Await Benchmark - Stress");
	state.SetItemsProcessed(size * 2);
}

BENCHMARK_REGISTER_F(metacall_node_py_await_bench, node_py_await_stress)
	->Unit(benchmark::kMillisecond)
	->Arg(2000)
	->Arg(10000)
	->Iterations(1)
	->Repetitions(3)
	->UseRealTime();

/* TODO: NodeJS re-initialization */
/* BENCHMARK_MAIN(); */

int main(int argc, char **argv)
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

/* NodeJS & Python */
#if defined(OPTION_BUILD_LOADERS_NODE) && defined(OPTION_BUILD_LOADERS_PY)
	{
		static const char py_script[] =
			"async def py_double(n):\n"
			"\treturn n * 2\n";

		if (metacall_load_from_memory("py", py_script, sizeof(py_script), NULL) != 0)
		{
			metacall_destroy();
			return 1;
		}

		static const char node_script[] =
			"const { metacall_await } = require('" METACALL_NODE_PORT_PATH "');\n"
			"function node_sleep(n) { return new Promise(resolve => { setTimeout(() => resolve(n), 0); }); }\n"
			"function node_py_await_stress(size) {\n"
			"	const check = (expected, v) => {\n"
			"		if (v !== expected) {\n"
			"			throw new Error(`Expected ${expected} but got ${v}`);\n"
			"		}\n"
			"	};\n"
			"	const promises = [];\n"
			/* Python awaits launched at once, all of them are resolved from the Python thread */
			"	for (let i = 0; i < size; i++) {\n"
			"		promises.push(metacall_await('py_double', i).then(v => check(i * 2, v)));\n"
			"	}\n"
			/* NodeJS Promise Chains launched at once, mixing resolutions from the NodeJS and the Python threads */
			"	for (let i = 0; i < size; i++) {\n"
			"		promises.push(metacall_await('node_sleep', i).then(v => {\n"
			"			check(i, v);\n"
			"			return metacall_await('py_double', v);\n"
			"		}).then(v => {\n"
			"			check(i * 2, v);\n"
			"			return metacall_await('node_sleep', v + 1);\n"
			"		}).then(v => check(i * 2 + 1, v)));\n"
			"	}\n"
			"	return Promise.all(promises).then(results => results.length);\n"
			"}\n"
			"module.exports = { node_sleep, node_py_await_stress };\n";

		if (metacall_load_from_memory("node", node_script, sizeof(node_script), NULL) != 0)
		{
			metacall_destroy();
			return 1;
		}
	}
#endif /* OPTION_BUILD_LOADERS_NODE && OPTION_BUILD_LOADERS_PY */

	::benchmark::RunSpecifiedBenchmarks();

	metacall_destroy();

	return 0;
}
