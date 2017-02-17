/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall_value.h>
#include <metacall/metacall-plugins.h>

#include <log/log.h>

class rb_loader_parser_integration_test : public testing::Test
{
public:
};

TEST_F(rb_loader_parser_integration_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Ruby */
	#if defined(OPTION_BUILD_PLUGINS_RB)
	{
		const char * rb_scripts[] =
		{
			"cache.rb"
		};

		void * ret = NULL;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("rb", rb_scripts, sizeof(rb_scripts) / sizeof(rb_scripts[0])));

		EXPECT_EQ((void *) NULL, (void *) metacall("cache_initialize"));

		EXPECT_EQ((void *) NULL, (void *) metacall("cache_set", "meta", "call"));

		ret = metacall("cache_has_key");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_NE((int) 0, (int) metacall_value_to_bool(ret));

		metacall_value_destroy(ret);

		ret = metacall("cache_get", "meta");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 0, (int) strcmp(metacall_value_to_string(ret), "call"));

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_PLUGINS_RB */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
