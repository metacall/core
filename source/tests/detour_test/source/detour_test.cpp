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

#include <dynlink/dynlink.h>

#include <string.h>

class detour_test : public testing::Test
{
public:
};

static detour_handle handle = NULL;
static const char *(*trampoline)(void) = NULL;

int check_detour_hook(const char *(*fp)(void))
{
	static const char str_without_hook[] = "Detour Library";

	const char *str = fp();

	log_write("metacall", LOG_LEVEL_DEBUG, "Check: %s", str);

	return strncmp(str, str_without_hook, sizeof(str_without_hook) - 1);
}

const char *hook_function(void)
{
	static const char str_with_hook[] = "Yeet";

	log_write("metacall", LOG_LEVEL_DEBUG, "HOOK WORKING PROPERLY");
	log_write("metacall", LOG_LEVEL_DEBUG, "Original function: %s", trampoline());

	/* Here we check that we got the correct trampoline implementation (aka the original function)
	and we can call it from inside of the body of the hook function */
	EXPECT_EQ((int)0, (int)check_detour_hook(trampoline));

	return str_with_hook;
}

/* TODO:
*    This test is not going to work because detour_enumeration does not walk in
*    the following sections:
*        T	Global text symbol
*        t	Local text symbol
*    This funtion we are searching for is stored in:
*        0000000000073630 T test_exported_symbols_from_executable
*        00000000000736e0 t _Z13hook_functionv
*        0000000000072e34 t _Z13hook_functionv.cold
*        0000000000073680 t _Z17check_detour_hookPFPKcvE
*    We can find all the sections here: https://en.wikipedia.org/wiki/Nm_(Unix)
*    For listing properly all the symbols we should replicate something like
*    GNU libc does under the hood for dlsym, which is implemented through do_lookup:
*    https://sourceware.org/git/?p=glibc.git;a=blob_plain;f=elf/dl-lookup.c;hb=HEAD
*    We will leave this for future versions, including support for GNU hashed symbols.
*/
#define TODO_TEST_EXPORTED_SYMBOLS_FROM_EXECUTABLE 1

#ifdef _WIN32
	#define EXPORT_SYMBOL __declspec(dllexport)
#else
	#define EXPORT_SYMBOL __attribute__((visibility("default")))
#endif

extern "C" EXPORT_SYMBOL int test_exported_symbols_from_executable(int x)
{
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

	EXPECT_STREQ(name, detour_name(d));

	/* Load detour of detour library */
	handle = detour_load_file(d, NULL);

	ASSERT_NE((detour_handle)NULL, (detour_handle)handle);

	/* Check if it can list exported symbols from executable */
#ifndef TODO_TEST_EXPORTED_SYMBOLS_FROM_EXECUTABLE
	test_exported_symbols_from_executable(3);

	unsigned int position = 0;
	const char *fn_name = NULL;
	void (**addr)(void) = NULL;
	bool found = false;
	while (detour_enumerate(d, handle, &position, &fn_name, &addr) == 0)
	{
		log_write("metacall", LOG_LEVEL_DEBUG, "[%d] %p %s", position, *addr, fn_name);

		if (strcmp("test_exported_symbols_from_executable", fn_name) == 0)
		{
			found = true;
			EXPECT_EQ((void *)(*addr), (void *)(&test_exported_symbols_from_executable));
			break;
		}
	}

	EXPECT_EQ((bool)true, (bool)found);
#endif

	/* Install detour */
	union
	{
		const char *(**trampoline)(void);
		void (**ptr)(void);
	} cast = { &trampoline };

	ASSERT_EQ((int)0, detour_replace(d, handle, "detour_print_info", (void (*)(void))(&hook_function), cast.ptr));

	/* This must return "Yeet", so when checking the test it should return distinct from 0, then the funtion is properly hooked */
	EXPECT_NE((int)0, (int)check_detour_hook(&detour_print_info));

	/* Uninstall detour */
	detour_unload(d, handle);

	/* Clear detour */
	EXPECT_EQ((int)0, (int)detour_clear(d));

	/* Destroy detour */
	detour_destroy();
}
