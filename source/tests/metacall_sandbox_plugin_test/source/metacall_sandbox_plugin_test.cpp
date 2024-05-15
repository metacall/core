/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

void invalid_syscall(void)
{
	struct utsname data;
	uname(&data);
	printf("%s\n", data.sysname);
}

void invalid_io_syscall(void *sandbox_ctx, void *handle)
{
	/* Disable io syscall */
	{
		void *args[2] = { sandbox_ctx, metacall_value_create_bool(0L) /* Kill */ };

		void *ret = metacallhv_s(handle, "sandbox_io", args, 2);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[1]);
	}

	printf("Invalid IO syscall\n");
}

#include <sys/socket.h>

void invalid_sockets_syscall()
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	close(fd);
}

#include <sys/ipc.h>
#include <sys/shm.h>

void invalid_ipc_syscall()
{
	// Create a shared memory segment
	int shm_id = shmget(1234, 1024, IPC_CREAT | 0666);
	if (shm_id == -1)
	{
		perror("shmget");
		exit(EXIT_FAILURE);
	}

	// Attach the shared memory segment to the process's address space
	void *shm_addr = shmat(shm_id, NULL, 0);
	if (shm_addr == (void *)-1)
	{
		perror("shmat");
		exit(EXIT_FAILURE);
	}

	// Write data to shared memory
	const char *message = "Hello, Shared Memory!";
	strncpy((char *)shm_addr, message, 1024);

	printf("Data written to shared memory: %s\n", (char *)shm_addr);

	// Detach the shared memory segment
	if (shmdt(shm_addr) == -1)
	{
		perror("shmdt");
		exit(EXIT_FAILURE);
	}

	// Remove the shared memory segment
	if (shmctl(shm_id, IPC_RMID, NULL) == -1)
	{
		perror("shmctl");
		exit(EXIT_FAILURE);
	}
}

#include <sys/types.h>
#include <sys/wait.h>

void invalid_process_syscall(void *sandbox_ctx, void *handle)
{
	/* Disable process syscall */
	{
		void *args[2] = { sandbox_ctx, metacall_value_create_bool(0L) /* Kill */ };

		void *ret = metacallhv_s(handle, "sandbox_process", args, 2);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[1]);
	}

	pid_t pid = fork();

	if (pid < 0)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (pid == 0)
	{
		// Child process
		exit(EXIT_SUCCESS); // Exit immediately
	}
	else
	{
		// Parent process
		int status;
		if (waitpid(pid, &status, 0) == -1)
		{
			perror("waitpid");
			exit(EXIT_FAILURE);
		}
	}
}

#include <fcntl.h>
#include <unistd.h>

void invalid_filesystems_syscall(void *sandbox_ctx, void *handle)
{
	/* Disable filesystems syscall */
	{
		void *args[2] = { sandbox_ctx, metacall_value_create_bool(0L) /* Kill */ };

		void *ret = metacallhv_s(handle, "sandbox_filesystems", args, 2);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[1]);
	}

	int fd = open("/tmp/testfile", O_RDONLY);
	if (fd == -1)
	{
		perror("open");
		exit(EXIT_FAILURE);
	}

	close(fd);
}

#include <time.h>

void invalid_time_syscall()
{
	sleep(1);
}

#include <sys/mman.h>

void invalid_memory_syscall(void *sandbox_ctx, void *handle)
{
	/* Disable memory syscall */
	{
		void *args[2] = { sandbox_ctx, metacall_value_create_bool(0L) /* Kill */ };

		void *ret = metacallhv_s(handle, "sandbox_memory", args, 2);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[1]);
	}

	void *addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap");
		exit(EXIT_FAILURE);
	}
	munmap(addr, 4096);
}

#include <signal.h>

void invalid_signals_syscall()
{
	if (signal(SIGINT, SIG_IGN) == SIG_ERR)
	{
		perror("signal");
		exit(EXIT_FAILURE);
	}
}
class metacall_sandbox_plugin_test : public testing::Test
{
protected:
};

TEST_F(metacall_sandbox_plugin_test, DefaultConstructor)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	void *sandbox_ctx, *handle = metacall_plugin_core();

	ASSERT_NE((void *)NULL, (void *)handle);

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
TEST_F(metacall_sandbox_plugin_test, SANDBOX_IO_DISABLE_TEST)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	void *sandbox_ctx, *handle = metacall_plugin_core();

	ASSERT_NE((void *)NULL, (void *)handle);

	/* Initialize sandboxing */
	{
		void *args[1] = { metacall_value_create_bool(1L) /* Allow */ };

		sandbox_ctx = metacallhv_s(handle, "sandbox_initialize", args, 1);

		EXPECT_NE((void *)NULL, (void *)sandbox_ctx);
		EXPECT_NE((void *)metacall_value_to_ptr(sandbox_ctx), (void *)NULL);

		metacall_value_destroy(args[0]);
	}

	/* Generate a syscall exception when trying to execute fopen */
	ASSERT_EXIT({ invalid_io_syscall(sandbox_ctx, handle); }, testing::KilledBySignal(SIGSYS), "");

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

TEST_F(metacall_sandbox_plugin_test, SANDBOX_SOCKETS_DISABLE_TEST)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	void *sandbox_ctx, *handle = metacall_plugin_core();

	ASSERT_NE((void *)NULL, (void *)handle);

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

TEST_F(metacall_sandbox_plugin_test, SANDBOX_IPC_DISABLE_TEST)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	void *sandbox_ctx, *handle = metacall_plugin_core();

	ASSERT_NE((void *)NULL, (void *)handle);

	/* Initialize sandboxing */
	{
		void *args[1] = { metacall_value_create_bool(1L) /* Allow */ };

		sandbox_ctx = metacallhv_s(handle, "sandbox_initialize", args, 1);

		EXPECT_NE((void *)NULL, (void *)sandbox_ctx);
		EXPECT_NE((void *)metacall_value_to_ptr(sandbox_ctx), (void *)NULL);

		metacall_value_destroy(args[0]);
	}

	/* Disable IPC syscall */
	{
		void *args[2] = { sandbox_ctx, metacall_value_create_bool(0L) /* Kill */ };

		void *ret = metacallhv_s(handle, "sandbox_ipc", args, 2);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[1]);
	}

	/* Generate a syscall exception when trying to execute IPC operation */
	ASSERT_EXIT({ invalid_ipc_syscall(); }, testing::KilledBySignal(SIGSYS), "");

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

TEST_F(metacall_sandbox_plugin_test, SANDBOX_PROCESS_DISABLE_TEST)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	void *sandbox_ctx, *handle = metacall_plugin_core();

	ASSERT_NE((void *)NULL, (void *)handle);

	/* Initialize sandboxing */
	{
		void *args[1] = { metacall_value_create_bool(1L) /* Allow */ };

		sandbox_ctx = metacallhv_s(handle, "sandbox_initialize", args, 1);

		EXPECT_NE((void *)NULL, (void *)sandbox_ctx);
		EXPECT_NE((void *)metacall_value_to_ptr(sandbox_ctx), (void *)NULL);

		metacall_value_destroy(args[0]);
	}

	/* Generate a syscall exception when trying to execute process operation */
	ASSERT_EXIT({ invalid_process_syscall(sandbox_ctx, handle); }, testing::KilledBySignal(SIGSYS), "");

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

TEST_F(metacall_sandbox_plugin_test, SANDBOX_FILESYSTEMS_DISABLE_TEST)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	void *sandbox_ctx, *handle = metacall_plugin_core();

	ASSERT_NE((void *)NULL, (void *)handle);

	/* Initialize sandboxing */
	{
		void *args[1] = { metacall_value_create_bool(1L) /* Allow */ };

		sandbox_ctx = metacallhv_s(handle, "sandbox_initialize", args, 1);

		EXPECT_NE((void *)NULL, (void *)sandbox_ctx);
		EXPECT_NE((void *)metacall_value_to_ptr(sandbox_ctx), (void *)NULL);

		metacall_value_destroy(args[0]);
	}

	/* Generate a syscall exception when trying to execute file systems operation */
	ASSERT_EXIT({ invalid_filesystems_syscall(sandbox_ctx, handle); }, testing::KilledBySignal(SIGSYS), "");

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

TEST_F(metacall_sandbox_plugin_test, SANDBOX_TIME_DISABLE_TEST)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	void *sandbox_ctx, *handle = metacall_plugin_core();

	ASSERT_NE((void *)NULL, (void *)handle);

	/* Initialize sandboxing */
	{
		void *args[1] = { metacall_value_create_bool(1L) /* Allow */ };

		sandbox_ctx = metacallhv_s(handle, "sandbox_initialize", args, 1);

		EXPECT_NE((void *)NULL, (void *)sandbox_ctx);
		EXPECT_NE((void *)metacall_value_to_ptr(sandbox_ctx), (void *)NULL);

		metacall_value_destroy(args[0]);
	}

	/* Disable Time syscall */
	{
		void *args[2] = { sandbox_ctx, metacall_value_create_bool(0L) /* Kill */ };

		void *ret = metacallhv_s(handle, "sandbox_time", args, 2);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[1]);
	}

	/* Generate a syscall exception when trying to execute time operation */
	ASSERT_EXIT({ invalid_time_syscall(); }, testing::KilledBySignal(SIGSYS), "");

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

TEST_F(metacall_sandbox_plugin_test, SANDBOX_MEMORY_DISABLE_TEST)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	void *sandbox_ctx, *handle = metacall_plugin_core();

	ASSERT_NE((void *)NULL, (void *)handle);

	/* Initialize sandboxing */
	{
		void *args[1] = { metacall_value_create_bool(1L) /* Allow */ };

		sandbox_ctx = metacallhv_s(handle, "sandbox_initialize", args, 1);

		EXPECT_NE((void *)NULL, (void *)sandbox_ctx);
		EXPECT_NE((void *)metacall_value_to_ptr(sandbox_ctx), (void *)NULL);

		metacall_value_destroy(args[0]);
	}

	/* Generate a syscall exception when trying to execute memory operation */
	ASSERT_EXIT({ invalid_memory_syscall(sandbox_ctx, handle); }, testing::KilledBySignal(SIGSYS), "");

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

TEST_F(metacall_sandbox_plugin_test, SANDBOX_SIGNALS_DISABLE_TEST)
{
	ASSERT_EQ((int)0, (int)metacall_initialize());

	void *sandbox_ctx, *handle = metacall_plugin_core();

	ASSERT_NE((void *)NULL, (void *)handle);

	/* Initialize sandboxing */
	{
		void *args[1] = { metacall_value_create_bool(1L) /* Allow */ };

		sandbox_ctx = metacallhv_s(handle, "sandbox_initialize", args, 1);

		EXPECT_NE((void *)NULL, (void *)sandbox_ctx);
		EXPECT_NE((void *)metacall_value_to_ptr(sandbox_ctx), (void *)NULL);

		metacall_value_destroy(args[0]);
	}

	/* Disable IPC syscall */
	{
		void *args[2] = { sandbox_ctx, metacall_value_create_bool(0L) /* Kill */ };

		void *ret = metacallhv_s(handle, "sandbox_signals", args, 2);

		EXPECT_NE((void *)NULL, (void *)ret);
		EXPECT_EQ((int)metacall_value_to_int(ret), (int)0);

		metacall_value_destroy(ret);
		metacall_value_destroy(args[1]);
	}

	/* Generate a syscall exception when trying to execute signals operation */
	ASSERT_EXIT({ invalid_signals_syscall(); }, testing::KilledBySignal(SIGSYS), "");

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
