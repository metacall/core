/*
 *	Abstract Data Type Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A abstract data type library providing generic containers.
 *
 */

#include <gmock/gmock.h>

#include <log/log_level.h>
#include <log/log_handle.h>

class log_test : public testing::Test
{
  public:
};

TEST_F(log_test, DefaultConstructor)
{
	const char * log_name_list[] =
	{
		"test_log_sync_stdout",
		"test_log_sync_stderr",
		"test_log_sync_file",
		"test_log_sync_syslog",
		"test_log_sync_socket",
		"test_log_async_stdout",
		"test_log_async_stderr",
		"test_log_async_file",
		"test_log_async_syslog",
		"test_log_async_socket"
	};

	const size_t log_name_list_size = sizeof(log_name_list) / sizeof(log_name_list[0]);

	size_t iterator;

	/* Create logs */
	{
		/*
		API :

		LOG_API log_impl log_impl_create_stdio(FILE * stream);

		LOG_API log_impl log_impl_create_file(const char * path);

		LOG_API log_impl log_impl_create_syslog(void);

		#include <stdint.h>

		LOG_API log_impl log_impl_create_socket(const char * ip, uint16_t);
		*/


		/*
		EXPECT_EQ((int) 0, (int) log_create(log_name_list[0], log_policy_create_sync(), log_impl_create_stdio(stdout)));

		EXPECT_EQ((int) 0, (int) log_create(log_name_list[1], log_policy_create_sync(), log_impl_create_stdio(stderr)));

		EXPECT_EQ((int) 0, (int) log_create(log_name_list[2], log_policy_create_sync(), log_impl_create_file("./test_log_sync_file.log", "a+")));

		EXPECT_EQ((int) 0, (int) log_create(log_name_list[3], log_policy_create_sync(), log_impl_create_syslog()));

		EXPECT_EQ((int) 0, (int) log_create(log_name_list[4], log_policy_create_sync(), log_impl_create_socket("127.0.0.1", UINT16_C(0x0208))));

		EXPECT_EQ((int) 0, (int) log_create(log_name_list[5], log_policy_create_async(), log_impl_create_stdio(stdout)));

		EXPECT_EQ((int) 0, (int) log_create(log_name_list[6], log_policy_create_async(), log_impl_create_stdio(stderr)));

		EXPECT_EQ((int) 0, (int) log_create(log_name_list[7], log_policy_create_async(), log_impl_create_file("./test_log_async_file.log", "a+")));

		EXPECT_EQ((int) 0, (int) log_create(log_name_list[8], log_policy_create_async(), log_impl_create_syslog()));

		EXPECT_EQ((int) 0, (int) log_create(log_name_list[9], log_policy_create_async(), log_impl_create_socket("127.0.0.1", UINT16_C(0x0209))));
		*/
	}

	/* Write simple logs */
	/*
	for (iterator = 0; iterator < log_name_list_size; ++iterator)
	{
		EXPECT_EQ((int) 0, (int) log_write(log_name_list[iterator], "test_simple_message_tag", LOG_LEVEL_INFO, "hello world"));
	}
	*/

	/* Write varidic logs */
	/*
	for (iterator = 0; iterator < log_name_list_size; ++iterator)
	{
		EXPECT_EQ((int) 0, (int)log_write_va(log_name_list[iterator], "test_varidic_tag", LOG_LEVEL_INFO, "hello world from log (id : %ul)", iterator));
	}
	*/

	/* Clear all logs */
	/*
	for (iterator = 0; iterator < log_name_list_size; ++iterator)
	{
		EXPECT_EQ((int) 0, (int) log_clear(log_name_list[iterator]));
	}
	*/
}
