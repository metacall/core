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
#include <metacall/metacall_loaders.h>

#define METACALL_CALL_TEST_SIZE 10000

class metacall_node_call_test : public testing::Test
{
public:
};

TEST_F(metacall_node_call_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* NodeJS */
	#if defined(OPTION_BUILD_LOADERS_NODE)
	{
		const char * node_scripts[] =
		{
			"nod.js"
		};

		const enum metacall_value_id hello_boy_double_ids[] =
		{
			METACALL_DOUBLE, METACALL_DOUBLE
		};

		const size_t call_size = METACALL_CALL_TEST_SIZE;

		EXPECT_EQ((int) 0, (int) metacall_load_from_file("node", node_scripts, sizeof(node_scripts) / sizeof(node_scripts[0]), NULL));

		for (size_t iterator = 0; iterator < call_size; ++iterator)
		{
			void * ret =  metacallt("call_test", hello_boy_double_ids, 3.0, 4.0);

			EXPECT_NE((void *) NULL, (void *) ret);

			EXPECT_EQ((double) metacall_value_to_double(ret), (double) 12.0);

			metacall_value_destroy(ret);
		}
	}
	#endif /* OPTION_BUILD_LOADERS_NODE */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
