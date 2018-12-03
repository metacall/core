/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#include <gmock/gmock.h>

#include <log/log.h>

#include <detour/detour.h>

#include <string.h>

class detour_test : public testing::Test
{
  public:
};

static detour_handle handle;

int hook_function(int x)
{
	EXPECT_EQ((int) 2, (int) x);

	log_write("metacall", LOG_LEVEL_DEBUG, "Hook function %d", x);

	int (*target_function_ptr)(int) = (int(*)(int))detour_trampoline(handle);

	return target_function_ptr(x + 4) + 2;
}

int target_function(int x)
{
	EXPECT_EQ((int) 6, (int) x);

	log_write("metacall", LOG_LEVEL_DEBUG, "Target function %d", x);

	return 4;
}

TEST_F(detour_test, DefaultConstructor)
{
	static const char name[] = "funchook";

	/* Initialize log */
	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	/* Initialize detour */
	EXPECT_EQ((int) 0, (int) detour_initialize());

	/* Create detour funchook */
	detour d = detour_create(name);

	EXPECT_NE((detour) NULL, (detour) d);

	EXPECT_EQ((int) 0, (int) strcmp(name, detour_name(d)));

	/* Install detour */
	handle = detour_install(d, (void(*)(void))&target_function, (void(*)(void))&hook_function);

	EXPECT_NE((detour_handle) NULL, (detour_handle) handle);

	/* Call detour, it should call hooked function */
	EXPECT_EQ((int) 6, (int) target_function(2));

	/* Uninstall detour */
	EXPECT_EQ((int) 0, (int) detour_uninstall(d, handle));

	/* Clear detour */
	EXPECT_EQ((int) 0, (int) detour_clear(d));

	/* Destroy detour */
	detour_destroy();
}
