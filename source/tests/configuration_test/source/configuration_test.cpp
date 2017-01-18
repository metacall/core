/*
*	Reflect Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A library for provide reflection and metadata representation.
*
*/

#include <gmock/gmock.h>

#include <configuration/configuration.h>

#include <log/log.h>

class configuration_test : public testing::Test
{
  public:
};

TEST_F(configuration_test, DefaultConstructor)
{
	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	const char path[] = "./global.json";
	const char name[] = "global";
	const char key[] = "value";

	EXPECT_EQ((int) 0, (int) configuration_initialize(path));

	configuration global = configuration_scope(name);

	EXPECT_NE((configuration) NULL, (configuration) global);

	value v = configuration_value(global, key, TYPE_INT);

	EXPECT_NE((value) NULL, (value) v);

	EXPECT_EQ((int) 12345, (int) value_to_int(v));

	configuration_destroy();
}
