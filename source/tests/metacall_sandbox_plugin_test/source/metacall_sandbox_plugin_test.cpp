/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <metacall/metacall.h>

#include <stdio.h>
#include <sys/utsname.h>
#include <sys/socket.h>

void invalid_syscall(void)
{
	struct utsname data;
	uname(&data);
	printf("%s\n", data.sysname);
}

void invalid_io_syscall(void) {
	FILE* fptr = fopen("main.cpp", "r");
	(void)fptr;
}

void invalid_sockets_syscall() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  close(fd);
}

class metacall_sandbox_plugin_test : public testing::Test
{
protected:
};

TEST_F(metacall_sandbox_plugin_test, DefaultConstructor)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	void *sandbox_ctx, *handle = metacall_plugin_extension();

	/* Initialize sandboxing */
	{
		void *args[1] = { metacall_value_create_bool(1L) /* Allow */ };

		sandbox_ctx = metacallhv_s(handle, "sandbox_initialize", args, 1);

		EXPECT_NE((void *)NULL, (void *)sandbox_ctx);
		EXPECT_NE((void *)metacall_value_to_ptr(sandbox_ctx), (void *)NULL);

		metacall_value_destroy(args[0]);
	}

	/* Disable uname syscall */
	{
		void *args[2] = { sandbox_ctx, metacall_value_create_bool(0L) /* Kill */ };

		void *ret = metacallhv_s(handle, "sandbox_uname", args, 2);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[1]);
	}

	/* Generate a syscall exception when trying to execute uname */
	ASSERT_EXIT({ invalid_syscall(); }, testing::KilledBySignal(SIGSYS), "");

	/* Destroy sandboxing */
	{
		void *args[1] = { sandbox_ctx };

		void *ret = metacallhv_s(handle, "sandbox_destroy", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

		metacall_value_destroy(args[0]);
	}

	/* Testing invalid number of parameters */
	{
		void *ret = metacallhv_s(handle, "sandbox_destroy", metacall_null_args, 0);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
	}

	/* Testing correct number of parameters but invalid type */
	{
		void *args[1] = { metacall_value_create_long(2343) };

		void *ret = metacallhv_s(handle, "sandbox_destroy", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((enum metacall_value_id)metacall_value_id(ret), (enum metacall_value_id)METACALL_THROWABLE);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[0]);
	}

	EXPECT_EQ((int)0, (int)metacall_destroy());
}

/* [Note] This test blocks all the gtest context, so you should comment it to allow testing for other test cases */
// TEST_F(metacall_sandbox_plugin_test, SANDBOX_IO_DISABLE_TEST)
// {
// 	ASSERT_EQ((int)0, (int)metacall_initialize());

// 	void *sandbox_ctx, *handle = metacall_plugin_extension();

// 	/* Initialize sandboxing */
// 	{
// 		void *args[1] = { metacall_value_create_bool(1L) /* Allow */ };

// 		sandbox_ctx = metacallhv_s(handle, "sandbox_initialize", args, 1);

// 		EXPECT_NE((void *)NULL, (void *)sandbox_ctx);
// 		EXPECT_NE((void *)metacall_value_to_ptr(sandbox_ctx), (void *)NULL);

// 		metacall_value_destroy(args[0]);
// 	}

// 	/* Disable io syscall */
// 	{
// 		void *args[2] = { sandbox_ctx, metacall_value_create_bool(0L) /* Kill */ };

// 		void *ret = metacallhv_s(handle, "sandbox_io", args, 2);

// 		EXPECT_NE((void *)NULL, (void *)ret);
// 		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

// 		metacall_value_destroy(ret);
// 		metacall_value_destroy(args[1]);
// 	}

// 	/* Generate a syscall exception when trying to execute fopen */
// 	ASSERT_EXIT({ invalid_io_syscall(); }, testing::KilledBySignal(SIGSYS), "");

// 	/* Destroy sandboxing */
// 	{
// 		void *args[1] = { sandbox_ctx };

// 		void *ret = metacallhv_s(handle, "sandbox_destroy", args, 1);

// 		EXPECT_NE((void *)NULL, (void *)ret);
// 		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

// 		metacall_value_destroy(args[0]);
// 	}

// 	EXPECT_EQ((int)0, (int)metacall_destroy());
// }

TEST_F(metacall_sandbox_plugin_test, SANDBOX_SOCKETS_DISABLE_TEST)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	void *sandbox_ctx, *handle = metacall_plugin_extension();

	/* Initialize sandboxing */
	{
		void *args[1] = { metacall_value_create_bool(1L) /* Allow */ };

		sandbox_ctx = metacallhv_s(handle, "sandbox_initialize", args, 1);

		EXPECT_NE((void *)NULL, (void *)sandbox_ctx);
		EXPECT_NE((void *)metacall_value_to_ptr(sandbox_ctx), (void *)NULL);

		metacall_value_destroy(args[0]);
	}

	/* Disable sockets syscall */
	{
		void *args[2] = { sandbox_ctx, metacall_value_create_bool(0L) /* Kill */ };

		void *ret = metacallhv_s(handle, "sandbox_sockets", args, 2);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[1]);
	}

	/* Generate a syscall exception when trying to execute socket */
	ASSERT_EXIT({ invalid_sockets_syscall(); }, testing::KilledBySignal(SIGSYS), "");

	/* Destroy sandboxing */
	{
		void *args[1] = { sandbox_ctx };

		void *ret = metacallhv_s(handle, "sandbox_destroy", args, 1);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

		metacall_value_destroy(args[0]);
	}

	EXPECT_EQ((int)0, (int)metacall_destroy());
}