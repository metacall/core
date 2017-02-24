/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall-plugins.h>

class rb_rails_integration_test : public testing::Test
{
public:
};

TEST_F(rb_rails_integration_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Ruby */
	#if defined(OPTION_BUILD_PLUGINS_RB)
	{
		const char * rb_scripts[] =
		{
			"blog_test.rb"
		};

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("rb", rb_scripts, sizeof(rb_scripts) / sizeof(rb_scripts[0])));
	}
	#endif /* OPTION_BUILD_PLUGINS_RB */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
