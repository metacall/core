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

#include <detour/detour.h>

#include <string.h>

class detour_test : public testing::Test
{
public:
};

static detour_handle handle = NULL;
static void (*trampoline)(void) = NULL;

int hook_function(int x)
{
	EXPECT_EQ((int)128, (int)x);

	log_write("metacall", LOG_LEVEL_DEBUG, "Hook function %d", x);

	int (*trampoline_ptr)(int) = (int (*)(int))trampoline;

	int result = trampoline_ptr(x + 2) + 2;

	log_write("metacall", LOG_LEVEL_DEBUG, "Hook function result %d", result);

	return result;
}

#ifdef _WIN32
	#define EXPORT_SYMBOL __declspec(dllexport)
#else
	#define EXPORT_SYMBOL __attribute__((visibility("default")))
#endif

extern "C" EXPORT_SYMBOL int target_function(int x)
{
	EXPECT_EQ((int)130, (int)x);

	log_write("metacall", LOG_LEVEL_DEBUG, "Target function %d", x);

	return x;
}

TEST_F(detour_test, DefaultConstructor)
{
	static const char name[] = "plthook";

	/* Initialize log */
	EXPECT_EQ((int)0, (int)log_configure("metacall",
						  log_policy_format_text(),
						  log_policy_schedule_sync(),
						  log_policy_storage_sequential(),
						  log_policy_stream_stdio(stdout)));

	/* Initialize detour */
	EXPECT_EQ((int)0, (int)detour_initialize());

	/* Create detour plthook */
	detour d = detour_create(name);

	ASSERT_NE((detour)NULL, (detour)d);

	EXPECT_EQ((int)0, (int)strcmp(name, detour_name(d)));

	/* Load detour */
	handle = detour_load_file(d, NULL);

	/* Install detour */
	detour_replace(d, handle, "target_function", (void (*)(void))(&hook_function), &trampoline);

	EXPECT_NE((detour_handle)NULL, (detour_handle)handle);

	/* Old funciton must equal to the trampoline returned by replace */
	EXPECT_EQ((int (*)(int))trampoline, (int (*)(int))(&target_function));

	/* Call detour, it should call hooked function */
	EXPECT_EQ((int)132, (int)target_function(128));

	/* Uninstall detour */
	detour_unload(d, handle);

	/* Clear detour */
	EXPECT_EQ((int)0, (int)detour_clear(d));

	/* Destroy detour */
	detour_destroy();
}
