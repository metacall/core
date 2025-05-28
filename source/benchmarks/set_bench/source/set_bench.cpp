/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <adt/adt_set.h>

#include <string>
#include <vector>

#define SET_SIZE   1000
#define ITERATIONS 1000

class set_bench : public benchmark::Fixture
{
public:
	void SetUp(benchmark::State &)
	{
		s = set_create(&hash_callback_ptr, &comparable_callback_ptr);

		keys.reserve(SET_SIZE);
		values.reserve(SET_SIZE);

		for (int i = 0; i < SET_SIZE; ++i)
		{
			keys.push_back(std::to_string(i));
			values.push_back(i);
			set_insert(s, (set_key)keys[i].c_str(), &values[i]);
		}
	}

	void TearDown(benchmark::State &)
	{
		set_destroy(s);
	}

	set s;
	std::vector<std::string> keys;
	std::vector<int> values;
};

int set_cb_iterate_sum(set s, set_key key, set_value value, set_cb_iterate_args args)
{
	int *i = (int *)value;
	uint64_t *sum = (uint64_t *)args;

	(void)s;
	(void)key;

	*sum = ((*sum) + (uint64_t)(*i));

	return 0;
}

BENCHMARK_DEFINE_F(set_bench, set_iterate)
(benchmark::State &state)
{
	uint64_t sum = 0;

	for (auto _ : state)
	{
		set_iterate(s, &set_cb_iterate_sum, &sum);
	}

	state.SetLabel("Set Benchmark - Iterate Callback");
	state.SetItemsProcessed(SET_SIZE);
}

BENCHMARK_REGISTER_F(set_bench, set_iterate)
	->Unit(benchmark::kMillisecond)
	->Iterations(ITERATIONS)
	->Repetitions(3);

/*
BENCHMARK_DEFINE_F(set_bench, set_iterators)
(benchmark::State &state)
{
	uint64_t sum = 0;

	for (auto _ : state)
	{
		for (set_iterator it = set_iterator_begin(s); set_iterator_end(&it) > 0; set_iterator_next(it))
		{
			int *i = (int *)set_iterator_value(it);

			sum += ((uint64_t)(*i));
		}
	}

	(void)sum;

	state.SetLabel("Set Benchmark - Iterators");
	state.SetItemsProcessed(SET_SIZE);
}

BENCHMARK_REGISTER_F(set_bench, set_iterators)
	->Unit(benchmark::kMillisecond)
	->Iterations(ITERATIONS)
	->Repetitions(3);
*/

BENCHMARK_DEFINE_F(set_bench, set_iterators_2)
(benchmark::State &state)
{
	uint64_t sum = 0;

	for (auto _ : state)
	{
		set_iterator_type it;

		for (set_iterator_begin(&it, s); set_iterator_end(&it) > 0; set_iterator_next(&it))
		{
			int *i = (int *)set_iterator_value(&it);

			sum += ((uint64_t)(*i));
		}
	}

	(void)sum;

	state.SetLabel("Set Benchmark - Iterators 2");
	state.SetItemsProcessed(SET_SIZE);
}

BENCHMARK_REGISTER_F(set_bench, set_iterators_2)
	->Unit(benchmark::kMillisecond)
	->Iterations(ITERATIONS)
	->Repetitions(3);

BENCHMARK_MAIN();
