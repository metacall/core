/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>

#include <log/log.h>

class metacall_fork_test : public testing::Test
{
public:
};

int callback_test(metacall_pid pid, void * ctx)
{
	(void)ctx;

	log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall fork callback test %d", (int)pid);

	return 0;
}

TEST_F(metacall_fork_test, DefaultConstructor)
{
	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/*metacall_fork(&callback_test);*/
	metacall_fork(NULL);

	#if defined(WIN32) || defined(_WIN32) || \
		defined(__CYGWIN__) || defined(__CYGWIN32__) || \
		defined(__MINGW32__) || defined(__MINGW64__)

		/* TODO: Not implemented  */

	#elif defined(unix) || defined(__unix__) || defined(__unix) || \
		defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
		defined(__CYGWIN__) || defined(__CYGWIN32__) || \
		(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

		if (fork() == 0)
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall fork child");
		}
		else
		{
			log_write("metacall", LOG_LEVEL_DEBUG, "MetaCall fork parent");
		}

	#else
	#	error "Unknown metacall fork safety platform"
	#endif

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
