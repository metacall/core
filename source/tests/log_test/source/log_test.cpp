/*
 *	Logger Library by Parra Studios
 *	A generic logger library providing application execution reports.
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

#include <gtest/gtest.h>

#include <log/log.h>
#include <log/log_handle.h>
#include <log/log_level.h>
#include <log/log_map.h>

class log_test : public testing::Test
{
public:
};

TEST_F(log_test, DefaultConstructor)
{
	struct log_name_list_type
	{
		const char *name;
		unsigned int value;
	} log_name_list[] = {
		{ "test_log_text_sync_seq_stdout", 0 },
		{ "test_log_text_sync_seq_stderr", 1 },
		{ "test_log_text_sync_seq_file", 2 },
		{ "test_log_text_sync_seq_syslog", 3 },
		{ "test_log_text_sync_seq_socket", 4 },
		{ "test_log_bin_async_bat_stdout", 5 },
		{ "test_log_bin_async_bat_stderr", 6 },
		{ "test_log_bin_async_bat_file", 7 },
		{ "test_log_bin_async_bat_syslog", 8 },
		{ "test_log_bin_async_bat_socket", 9 }
	};

	const size_t log_name_list_size = sizeof(log_name_list) / sizeof(log_name_list[0]);

	/* Log map */
	{
		size_t iterator;

		log_map_iterator map_iterator;

		log_map map = log_map_create((size_t)0x00000200);

		EXPECT_NE((log_map)NULL, (log_map)map);

		for (iterator = 0; iterator < log_name_list_size; ++iterator)
		{
			EXPECT_EQ((int)0, (int)log_map_insert(map, log_name_list[iterator].name, (const void *)&log_name_list[iterator].value));
		}

		EXPECT_EQ((size_t)log_name_list_size, (size_t)log_map_size(map));

		for (iterator = 0; iterator < log_name_list_size; ++iterator)
		{
			const void *value_ptr = log_map_get(map, log_name_list[iterator].name);

			unsigned int value = *((unsigned int *)value_ptr);

			EXPECT_EQ((unsigned int)log_name_list[iterator].value, (unsigned int)value);
		}

		for (map_iterator = log_map_iterator_begin(map); log_map_iterator_end(map_iterator) != 0; log_map_iterator_next(map_iterator))
		{
			const char *key = log_map_iterator_key(map_iterator);

			const void *value_ptr = log_map_iterator_value(map_iterator);

			unsigned int value = *((unsigned int *)value_ptr);

			EXPECT_STREQ(log_name_list[value].name, key);
		}

		EXPECT_EQ((int)log_map_destroy(map), (int)0);
	}

	/* Create logs */
	{
		size_t iterator;

		for (iterator = 0; iterator < log_name_list_size; ++iterator)
		{
			EXPECT_EQ((int)0, (int)log_create(log_name_list[iterator].name));
		}
	}

	/* Set policies */
	{
		const size_t storage_batch_size = ((size_t)0x00000010);

		EXPECT_EQ((int)0, (int)log_configure(log_name_list[0].name,
							  log_policy_format_text(),
							  log_policy_schedule_sync(),
							  log_policy_storage_sequential(),
							  log_policy_stream_stdio(stdout)));

		EXPECT_EQ((int)0, (int)log_configure(log_name_list[1].name,
							  log_policy_format_text(),
							  log_policy_schedule_sync(),
							  log_policy_storage_sequential(),
							  log_policy_stream_stdio(stderr)));

		EXPECT_EQ((int)0, (int)log_configure(log_name_list[2].name,
							  log_policy_format_text(),
							  log_policy_schedule_sync(),
							  log_policy_storage_sequential(),
							  log_policy_stream_file(log_name_list[2].name, "a+")));

		EXPECT_EQ((int)0, (int)log_configure(log_name_list[3].name,
							  log_policy_format_text(),
							  log_policy_schedule_sync(),
							  log_policy_storage_sequential(),
							  log_policy_stream_syslog(log_name_list[3].name)));

		EXPECT_EQ((int)0, (int)log_configure(log_name_list[4].name,
							  log_policy_format_text(),
							  log_policy_schedule_sync(),
							  log_policy_storage_sequential(),
							  log_policy_stream_socket("127.0.0.1", UINT16_C(0x0208))));

		EXPECT_EQ((int)0, (int)log_configure(log_name_list[5].name,
							  log_policy_format_binary(),
							  log_policy_schedule_async(),
							  log_policy_storage_batch(storage_batch_size),
							  log_policy_stream_stdio(stdout)));

		EXPECT_EQ((int)0, (int)log_configure(log_name_list[6].name,
							  log_policy_format_binary(),
							  log_policy_schedule_async(),
							  log_policy_storage_batch(storage_batch_size),
							  log_policy_stream_stdio(stderr)));

		EXPECT_EQ((int)0, (int)log_configure(log_name_list[7].name,
							  log_policy_format_binary(),
							  log_policy_schedule_async(),
							  log_policy_storage_batch(storage_batch_size),
							  log_policy_stream_file(log_name_list[7].name, "a+")));

		EXPECT_EQ((int)0, (int)log_configure(log_name_list[8].name,
							  log_policy_format_binary(),
							  log_policy_schedule_async(),
							  log_policy_storage_batch(storage_batch_size),
							  log_policy_stream_syslog(log_name_list[8].name)));

		EXPECT_EQ((int)0, (int)log_configure(log_name_list[9].name,
							  log_policy_format_binary(),
							  log_policy_schedule_async(),
							  log_policy_storage_batch(storage_batch_size),
							  log_policy_stream_socket("127.0.0.1", UINT16_C(0x0209))));
	}

	/* Write simple logs */
	{
		size_t iterator;

		for (iterator = 0; iterator < log_name_list_size; ++iterator)
		{
			EXPECT_EQ((int)0, (int)log_write(log_name_list[iterator].name, LOG_LEVEL_INFO, "hello world"));
			EXPECT_EQ((int)0, (int)log_write(log_name_list[iterator].name, LOG_LEVEL_INFO, "hell yeah"));
		}
	}

	/* Write varidic logs */
	{
		size_t iterator;

		for (iterator = 0; iterator < log_name_list_size; ++iterator)
		{
			EXPECT_EQ((int)0, (int)log_write(log_name_list[iterator].name, LOG_LEVEL_INFO, "hello world from log (id : %" PRIuS ")", iterator));
			EXPECT_EQ((int)0, (int)log_write(log_name_list[iterator].name, LOG_LEVEL_INFO, "%" PRIuS " %f", iterator + 1, 34.02346));
			EXPECT_EQ((int)0, (int)log_write(log_name_list[iterator].name, LOG_LEVEL_INFO, "double log (id : %" PRIuS ")", iterator * 2));
		}
	}

	/* Clear all logs */
	{
		size_t iterator;

		for (iterator = 0; iterator < log_name_list_size; ++iterator)
		{
			EXPECT_EQ((int)0, (int)log_clear(log_name_list[iterator].name));
		}
	}

	/* Policy format text flags */
	{
		EXPECT_EQ((int)0, (int)log_configure("newline",
							  log_policy_format_text_flags(LOG_POLICY_FORMAT_TEXT_NEWLINE),
							  log_policy_schedule_sync(),
							  log_policy_storage_sequential(),
							  log_policy_stream_stdio(stdout)));

		EXPECT_EQ((int)0, (int)log_write("newline", LOG_LEVEL_INFO, "NEW LINE A"));
		EXPECT_EQ((int)0, (int)log_write("newline", LOG_LEVEL_INFO, "NEW LINE B"));
		EXPECT_EQ((int)0, (int)log_write("newline", LOG_LEVEL_INFO, "hello world from log (id : %" PRIuS ")", 1234));
		EXPECT_EQ((int)0, (int)log_write("newline", LOG_LEVEL_INFO, "hello world from log (id : %" PRIuS ")", 5432));

		EXPECT_EQ((int)0, (int)log_configure("nonewline",
							  log_policy_format_text_flags(LOG_POLICY_FORMAT_TEXT_EMPTY),
							  log_policy_schedule_sync(),
							  log_policy_storage_sequential(),
							  log_policy_stream_stdio(stdout)));

		EXPECT_EQ((int)0, (int)log_write("nonewline", LOG_LEVEL_INFO, "NO NEW LINE A"));
		EXPECT_EQ((int)0, (int)log_write("nonewline", LOG_LEVEL_INFO, "NO NEW LINE B"));
		EXPECT_EQ((int)0, (int)log_write("nonewline", LOG_LEVEL_INFO, "hello world from log (id : %" PRIuS ")", 1234));
		EXPECT_EQ((int)0, (int)log_write("nonewline", LOG_LEVEL_INFO, "hello world from log (id : %" PRIuS ")", 5432));

		EXPECT_EQ((int)0, (int)log_clear("newline"));
		EXPECT_EQ((int)0, (int)log_clear("nonewline"));
	}
}
