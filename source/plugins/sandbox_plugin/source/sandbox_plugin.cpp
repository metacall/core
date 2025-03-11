/*
 *	Sandbox Plugin by Parra Studios
 *	A plugin implementing sandboxing functionality for MetaCall Core.
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

#include <sandbox_plugin/sandbox_plugin.h>

#include <plugin/plugin_interface.hpp>

#include <seccomp.h>

#define SANDBOX_ACTION(value) \
	metacall_value_to_bool(value) == 0L ? SCMP_ACT_KILL_PROCESS : SCMP_ACT_ALLOW

/* Error messages */
#define SANDBOX_INITIALIZE_ERROR  "Sandbox plugin failed to initialize a context"
#define SANDBOX_UNAME_ERROR		  "Sandbox plugin failed to set uname syscall permissions"
#define SANDBOX_IO_ERROR		  "Sandbox plugin failed to set io syscalls permissions"
#define SANDBOX_SOCKETS_ERROR	  "Sandbox plugin failed to set sockets syscalls permissions"
#define SANDBOX_IPC_ERROR		  "Sandbox plugin failed to set IPC syscalls permissions"
#define SANDBOX_PROCESS_ERROR	  "Sandbox plugin failed to set process syscalls permissions"
#define SANDBOX_FILESYSTEMS_ERROR "Sandbox plugin failed to set filesystems syscalls permissions"
#define SANDBOX_TIME_ERROR		  "Sandbox plugin failed to set time syscalls permissions"
#define SANDBOX_MEMORY_ERROR	  "Sandbox plugin failed to set memory syscalls permissions"
#define SANDBOX_SIGNALS_ERROR	  "Sandbox plugin failed to set signals syscalls permissions"
#define SANDBOX_DESTROY_ERROR	  "Sandbox plugin failed to destroy a context"

void add_syscalls_to_seccomp(scmp_filter_ctx ctx, const int *syscalls, const int action, size_t size)
{
	for (size_t iterator = 0; iterator < size; ++iterator)
	{
		seccomp_rule_add(ctx, action, syscalls[iterator], 0);
	}
}

void *sandbox_initialize(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_INITIALIZE_ERROR, METACALL_BOOL);

	/* Initialize the seccomp context */
	ctx = seccomp_init(SANDBOX_ACTION(args[0]));

	if (ctx == NULL)
	{
		/* Throw exception if seccomp initialization failed */
		EXTENSION_FUNCTION_THROW(SANDBOX_INITIALIZE_ERROR);
	}

	return metacall_value_create_ptr(ctx);
}

void *sandbox_uname(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_UNAME_ERROR, METACALL_PTR, METACALL_BOOL);

	ctx = metacall_value_to_ptr(args[0]);

	seccomp_rule_add(ctx, SANDBOX_ACTION(args[1]), SCMP_SYS(uname), 0);
	seccomp_load(ctx);

	return metacall_value_create_int(0);
}

/* [Note] This function shouldn't be called to disable io, because metacall's basic operation is loading which is io operation */
void *sandbox_io(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_IO_ERROR, METACALL_PTR, METACALL_BOOL);

	ctx = metacall_value_to_ptr(args[0]);

	const int syscalls[] = {
		SCMP_SYS(brk),
		SCMP_SYS(read),
		SCMP_SYS(write),
		SCMP_SYS(open),
		SCMP_SYS(close),
		SCMP_SYS(lseek),
		SCMP_SYS(dup),
		SCMP_SYS(dup2),
		SCMP_SYS(dup3),
		SCMP_SYS(pipe),
		SCMP_SYS(select),
		SCMP_SYS(poll),
		SCMP_SYS(fcntl),
		SCMP_SYS(ioctl),
		SCMP_SYS(readv),
		SCMP_SYS(writev),
		SCMP_SYS(send),
		SCMP_SYS(recv),
		SCMP_SYS(sendto),
		SCMP_SYS(recvfrom),
		SCMP_SYS(sendmsg),
		SCMP_SYS(recvmsg),
		SCMP_SYS(fsync),
		SCMP_SYS(fdatasync)
	};

	add_syscalls_to_seccomp(ctx, syscalls, SANDBOX_ACTION(args[1]), sizeof(syscalls) / sizeof(syscalls[0]));

	seccomp_load(ctx);

	return metacall_value_create_int(0);
}

void *sandbox_sockets(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_SOCKETS_ERROR, METACALL_PTR, METACALL_BOOL);

	ctx = metacall_value_to_ptr(args[0]);

	const int syscalls[] = {
		SCMP_SYS(socket),	   // It is primarily associated to networking
		SCMP_SYS(bind),		   // TODO: Check if this is needed, because it is also used for unix sockets (IPC)
		SCMP_SYS(listen),	   // TODO: Check if this is needed, because it is also used for unix sockets (IPC)
		SCMP_SYS(accept),	   // TODO: Check if this is needed, because it is also used for unix sockets (IPC)
		SCMP_SYS(connect),	   // TODO: Check if this is needed, because it is also used for unix sockets (IPC)
		SCMP_SYS(send),		   // TODO: Check if this is needed, because it is also used for unix sockets (IPC)
		SCMP_SYS(recv),		   // TODO: Check if this is needed, because it is also used for unix sockets (IPC)
		SCMP_SYS(sendto),	   // TODO: Check if this is needed, because it is also used for unix sockets (IPC)
		SCMP_SYS(recvfrom),	   // TODO: Check if this is needed, because it is also used for unix sockets (IPC)
		SCMP_SYS(shutdown),	   // It is primarily associated to networking
		SCMP_SYS(getpeername), // It is primarily associated to networking
		SCMP_SYS(socketpair),  // It is primarily associated to networking
		SCMP_SYS(setsockopt)   // It is primarily associated to networking
							   // SCMP_SYS(select),			// Shouldn't be needed because it is used for file descriptors too
							   // SCMP_SYS(poll),				// Shouldn't be needed because it is used for file descriptors too
							   // SCMP_SYS(fcntl),				// Shouldn't be needed because it is used for file descriptors too
							   // SCMP_SYS(ioctl)				// Shouldn't be needed because it is used for file descriptors too
	};

	add_syscalls_to_seccomp(ctx, syscalls, SANDBOX_ACTION(args[1]), sizeof(syscalls) / sizeof(syscalls[0]));

	seccomp_load(ctx);

	return metacall_value_create_int(0);
}

void *sandbox_ipc(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_IPC_ERROR, METACALL_PTR, METACALL_BOOL);

	ctx = metacall_value_to_ptr(args[0]);

	const int syscalls[] = {
		SCMP_SYS(shmget),
		SCMP_SYS(shmat),
		SCMP_SYS(shmdt),
		SCMP_SYS(shmctl),
		SCMP_SYS(msgget),
		SCMP_SYS(msgsnd),
		SCMP_SYS(msgrcv),
		SCMP_SYS(msgctl),
		SCMP_SYS(semget),
		SCMP_SYS(semop),
		SCMP_SYS(semctl)
	};

	add_syscalls_to_seccomp(ctx, syscalls, SANDBOX_ACTION(args[1]), sizeof(syscalls) / sizeof(syscalls[0]));

	seccomp_load(ctx);

	return metacall_value_create_int(0);
}

void *sandbox_process(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_PROCESS_ERROR, METACALL_PTR, METACALL_BOOL);

	ctx = metacall_value_to_ptr(args[0]);

	const int syscalls[] = {
		SCMP_SYS(fork),
		SCMP_SYS(vfork),
		SCMP_SYS(clone),
		SCMP_SYS(execve),
		SCMP_SYS(wait4),
		SCMP_SYS(waitpid),
		SCMP_SYS(waitid),
		SCMP_SYS(exit),
		SCMP_SYS(exit_group),
		SCMP_SYS(kill),
		SCMP_SYS(getpid),
		SCMP_SYS(getppid),
		SCMP_SYS(setsid),
		SCMP_SYS(setpgid),
		SCMP_SYS(nice),
		SCMP_SYS(sched_yield),
		SCMP_SYS(setpriority),
		SCMP_SYS(getpriority),
		SCMP_SYS(getpgid),
		SCMP_SYS(setsid)
	};

	add_syscalls_to_seccomp(ctx, syscalls, SANDBOX_ACTION(args[1]), sizeof(syscalls) / sizeof(syscalls[0]));

	seccomp_load(ctx);

	return metacall_value_create_int(0);
}

void *sandbox_filesystems(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_FILESYSTEMS_ERROR, METACALL_PTR, METACALL_BOOL);

	ctx = metacall_value_to_ptr(args[0]);

	const int syscalls[] = {
		SCMP_SYS(access),
		SCMP_SYS(faccessat),
		SCMP_SYS(chdir),
		SCMP_SYS(fchdir),
		SCMP_SYS(chroot),
		SCMP_SYS(fchmod),
		SCMP_SYS(fchmodat),
		SCMP_SYS(chown),
		SCMP_SYS(fchown),
		SCMP_SYS(fchownat),
		SCMP_SYS(lchown),
		SCMP_SYS(fcntl),
		SCMP_SYS(ioctl),
		SCMP_SYS(link),
		SCMP_SYS(linkat),
		SCMP_SYS(unlink),
		SCMP_SYS(unlinkat),
		SCMP_SYS(mkdir),
		SCMP_SYS(mkdirat),
		SCMP_SYS(rmdir),
		SCMP_SYS(rename),
		SCMP_SYS(renameat),
		SCMP_SYS(symlink),
		SCMP_SYS(symlinkat),
		SCMP_SYS(readlink),
		SCMP_SYS(readlinkat),
		SCMP_SYS(truncate),
		SCMP_SYS(ftruncate),
		SCMP_SYS(stat),
		SCMP_SYS(lstat),
		SCMP_SYS(fstat),
		SCMP_SYS(statfs),
		SCMP_SYS(statfs64),
		SCMP_SYS(fstatfs),
		SCMP_SYS(fstatfs64),
		SCMP_SYS(umount),
		SCMP_SYS(umount2),
		SCMP_SYS(mount),
		SCMP_SYS(mount),
		SCMP_SYS(mount)
	};

	add_syscalls_to_seccomp(ctx, syscalls, SANDBOX_ACTION(args[1]), sizeof(syscalls) / sizeof(syscalls[0]));

	seccomp_load(ctx);

	return metacall_value_create_int(0);
}

void *sandbox_time(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_TIME_ERROR, METACALL_PTR, METACALL_BOOL);

	ctx = metacall_value_to_ptr(args[0]);

	const int syscalls[] = {
		SCMP_SYS(time),
		SCMP_SYS(gettimeofday),
		SCMP_SYS(settimeofday),
		SCMP_SYS(clock_gettime),
		SCMP_SYS(clock_settime),
		SCMP_SYS(clock_getres),
		SCMP_SYS(clock_nanosleep),
		SCMP_SYS(nanosleep),
		SCMP_SYS(stime),
		SCMP_SYS(adjtimex),
		SCMP_SYS(timer_create),
		SCMP_SYS(timer_settime),
		SCMP_SYS(timer_gettime),
		SCMP_SYS(timer_getoverrun),
		SCMP_SYS(timer_delete),
		SCMP_SYS(timerfd_create),
		SCMP_SYS(timerfd_settime),
		SCMP_SYS(timerfd_gettime)
	};

	add_syscalls_to_seccomp(ctx, syscalls, SANDBOX_ACTION(args[1]), sizeof(syscalls) / sizeof(syscalls[0]));

	seccomp_load(ctx);

	return metacall_value_create_int(0);
}

void *sandbox_memory(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_MEMORY_ERROR, METACALL_PTR, METACALL_BOOL);

	ctx = metacall_value_to_ptr(args[0]);

	const int syscalls[] = {
		SCMP_SYS(mmap),
		SCMP_SYS(munmap),
		SCMP_SYS(mprotect),
		SCMP_SYS(brk),
		SCMP_SYS(mincore),
		SCMP_SYS(madvise),
		SCMP_SYS(mlock),
		SCMP_SYS(munlock),
		SCMP_SYS(mlockall),
		SCMP_SYS(munlockall),
		SCMP_SYS(getrlimit),
		SCMP_SYS(setrlimit),
		SCMP_SYS(getrusage)
	};

	add_syscalls_to_seccomp(ctx, syscalls, SANDBOX_ACTION(args[1]), sizeof(syscalls) / sizeof(syscalls[0]));

	seccomp_load(ctx);

	return metacall_value_create_int(0);
}

void *sandbox_signals(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_SIGNALS_ERROR, METACALL_PTR, METACALL_BOOL);

	ctx = metacall_value_to_ptr(args[0]);

	const int syscalls[] = {
		SCMP_SYS(kill),
		SCMP_SYS(tgkill),
		SCMP_SYS(tkill),
		SCMP_SYS(sigaction),
		SCMP_SYS(sigprocmask),
		SCMP_SYS(sigpending),
		SCMP_SYS(sigsuspend),
		SCMP_SYS(sigreturn),
		SCMP_SYS(rt_sigaction),
		SCMP_SYS(rt_sigprocmask),
		SCMP_SYS(rt_sigpending),
		SCMP_SYS(rt_sigsuspend),
		SCMP_SYS(rt_sigreturn),
		SCMP_SYS(rt_tgsigqueueinfo),
		SCMP_SYS(rt_sigtimedwait),
		SCMP_SYS(rt_sigqueueinfo)
	};

	add_syscalls_to_seccomp(ctx, syscalls, SANDBOX_ACTION(args[1]), sizeof(syscalls) / sizeof(syscalls[0]));

	seccomp_load(ctx);

	return metacall_value_create_int(0);
}

void *sandbox_destroy(size_t argc, void *args[], void *data)
{
	scmp_filter_ctx ctx;

	/* Validate function parameters */
	EXTENSION_FUNCTION_CHECK(SANDBOX_DESTROY_ERROR, METACALL_PTR);

	ctx = metacall_value_to_ptr(args[0]);

	if (ctx == NULL)
	{
		EXTENSION_FUNCTION_THROW(SANDBOX_DESTROY_ERROR ", the first parameter requires a non null pointer");
	}

	seccomp_release(ctx);

	return metacall_value_create_int(0);
}

#if 0 /* TODO: Fork safety */
	#ifdef METACALL_FORK_SAFE
static int sandbox_plugin_post_fork_callback(metacall_pid id, void *data)
{
	(void)id;
	(void)data;

	/* Reset libseccomp library status: https://man7.org/linux/man-pages/man3/seccomp_init.3.html */
	return seccomp_reset(NULL, SANDBOX_DEFAULT_ACTION);
}
	#endif /* METACALL_FORK_SAFE */
#endif

int sandbox_plugin(void *loader, void *handle)
{
	EXTENSION_FUNCTION(METACALL_PTR, sandbox_initialize, METACALL_BOOL);
	EXTENSION_FUNCTION(METACALL_INT, sandbox_uname, METACALL_PTR, METACALL_BOOL);
	EXTENSION_FUNCTION(METACALL_INT, sandbox_io, METACALL_PTR, METACALL_BOOL);
	EXTENSION_FUNCTION(METACALL_INT, sandbox_sockets, METACALL_PTR, METACALL_BOOL);
	EXTENSION_FUNCTION(METACALL_INT, sandbox_ipc, METACALL_PTR, METACALL_BOOL);
	EXTENSION_FUNCTION(METACALL_INT, sandbox_process, METACALL_PTR, METACALL_BOOL);
	EXTENSION_FUNCTION(METACALL_INT, sandbox_filesystems, METACALL_PTR, METACALL_BOOL);
	EXTENSION_FUNCTION(METACALL_INT, sandbox_time, METACALL_PTR, METACALL_BOOL);
	EXTENSION_FUNCTION(METACALL_INT, sandbox_memory, METACALL_PTR, METACALL_BOOL);
	EXTENSION_FUNCTION(METACALL_INT, sandbox_signals, METACALL_PTR, METACALL_BOOL);
	EXTENSION_FUNCTION(METACALL_INT, sandbox_destroy, METACALL_PTR);

#if 0 /* TODO: Fork safety */
	#ifdef METACALL_FORK_SAFE
	/* TODO: This requires to implement multiple callback handling in metacall_fork which is not implemented yet,
	and maybe also passing a context to the pre/post fork callbacks which is not implemented neither */
	metacall_fork(NULL, &sandbox_plugin_post_fork_callback);
	#endif /* METACALL_FORK_SAFE */
#endif

	return 0;
}
