/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
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

#include <gtest/gtest.h>

#include <adt/adt_atomic_set.h>

#include <log/log.h>

#include <thread>

class adt_atomic_set_test : public testing::Test
{
public:
};

TEST_F(adt_atomic_set_test, DefaultConstructor)
{
	EXPECT_EQ((int)0, (int)log_configure("metacall",
						  log_policy_format_text(),
						  log_policy_schedule_sync(),
						  log_policy_storage_sequential(),
						  log_policy_stream_stdio(stdout)));

	// Single thread
	{
		const size_t capacity = 16;
		atomic_set s = atomic_set_create(capacity);

		ASSERT_NE((atomic_set)s, (atomic_set)NULL);

		atomic_set_insert(s, "hello", (void *)0x10);
		atomic_set_insert(s, "bye", (void *)0x20);
		atomic_set_insert(s, "yeet", (void *)0x30);

		EXPECT_EQ((void *)atomic_set_get(s, "hello"), (void *)0x10);
		EXPECT_EQ((void *)atomic_set_get(s, "bye"), (void *)0x20);
		EXPECT_EQ((void *)atomic_set_get(s, "yeet"), (void *)0x30);

		EXPECT_EQ((void *)atomic_set_remove(s, "bye"), (void *)0x20);
		EXPECT_EQ((void *)atomic_set_get(s, "bye"), (void *)NULL);

		EXPECT_EQ((size_t)atomic_set_size(s), (size_t)2);

		EXPECT_EQ((int)atomic_set_clear(s), (int)0);

		EXPECT_EQ((size_t)atomic_set_size(s), (size_t)0);
		EXPECT_EQ((size_t)atomic_set_capacity(s), (size_t)capacity);

		atomic_set_destroy(s);
	}

	// TODO:
	// Multi thread
	/*
	{
		const size_t capacity = 4096;
		const size_t remover_threads = 8;

		atomic_set s = atomic_set_create(capacity);

		ASSERT_NE((atomic_set)NULL, s);

		for (size_t i = 0; i < capacity; ++i)
		{
			char key[8];
			snprintf(key, sizeof(key), "k%zu", i);

			ASSERT_EQ(
				(int)atomic_set_insert(
					s,
					key,
					(void *)(uintptr_t)(i + 1)
				),
				0
			);
		}

		std::atomic<bool> running(true);
		std::atomic<size_t> removed(0);

		std::thread reader([&]()
		{
			while (running.load(std::memory_order_relaxed))
			{
				for (size_t i = 0; i < capacity; ++i)
				{
					char key[8];
					snprintf(key, sizeof(key), "k%zu", i);

					void *value = (void *)atomic_set_get(s, key);

					if (value != NULL)
					{
						EXPECT_EQ(
							(uintptr_t)value,
							(uintptr_t)(i + 1)
						);
					}
				}
			}
		});

		std::vector<std::thread> workers;

		for (size_t t = 0; t < remover_threads; ++t)
		{
			workers.emplace_back([&, t]()
			{
				for (size_t i = t; i < capacity; i += remover_threads)
				{
					char key[8];
					snprintf(key, sizeof(key), "k%zu", i);

					void *value = (void *)atomic_set_remove(s, key);

					if (value != NULL)
					{
						EXPECT_EQ(
							(uintptr_t)value,
							(uintptr_t)(i + 1)
						);

						removed.fetch_add(1, std::memory_order_relaxed);
					}
				}
			});
		}

		for (auto &t : workers)
		{
			t.join();
		}

		running.store(false);
		reader.join();

		EXPECT_EQ(removed.load(), capacity);
		EXPECT_EQ((size_t)atomic_set_size(s), (size_t)0);

		atomic_set_destroy(s);
	}
	*/
}
