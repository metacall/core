/*
 *	Threading Library by Parra Studios
 *	A threading library providing utilities for lock-free data structures and more.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/* -- Headers -- */

#include <threading/threading_thread.h>

#include <process.h>
#include <stdlib.h>

/* -- Private Type Definitions -- */

/* _beginthreadex requires a callback of type (unsigned (__stdcall *)(void *)),
which is not compatible with threading_thread_routine (void *(*)(void *)) as
defined by the pthread based implementation. This trampoline carries the real
routine and its argument through _beginthreadex without relying on an invalid
cast between incompatible function pointer types */
typedef struct threading_thread_win32_trampoline_type
{
	threading_thread_routine routine;
	void *arg;

} * threading_thread_win32_trampoline;

static unsigned __stdcall threading_thread_win32_routine(void *data)
{
	threading_thread_win32_trampoline trampoline = (threading_thread_win32_trampoline)data;
	threading_thread_routine routine = trampoline->routine;
	void *arg = trampoline->arg;

	free(trampoline);

	routine(arg);

	return 0;
}

int threading_thread_create(threading_thread *thread, threading_thread_routine routine, void *arg)
{
	threading_thread_win32_trampoline trampoline = malloc(sizeof(struct threading_thread_win32_trampoline_type));

	if (trampoline == NULL)
	{
		return 1;
	}

	trampoline->routine = routine;
	trampoline->arg = arg;

	/* _beginthreadex is used instead of CreateThread because the thread calls into
	the C runtime and into Python, both of which require proper per-thread CRT
	initialization that only _beginthreadex guarantees */
	*thread = (HANDLE)_beginthreadex(NULL, 0, &threading_thread_win32_routine, trampoline, 0, NULL);

	if (*thread == NULL)
	{
		free(trampoline);
		return 1;
	}

	return 0;
}

int threading_thread_join(threading_thread *thread, void **result)
{
	if (WaitForSingleObject(*thread, INFINITE) != WAIT_OBJECT_0)
	{
		return 1;
	}

	/* The native Win32 thread result only carries a 32 bit exit code, not an
	arbitrary pointer, so there is no value that can be safely handed back here */
	if (result != NULL)
	{
		*result = NULL;
	}

	return 0;
}

int threading_thread_detach(threading_thread *thread)
{
	if (CloseHandle(*thread) == 0)
	{
		return 1;
	}

	return 0;
}

threading_thread_id threading_thread_self(void)
{
	return GetCurrentThreadId();
}

void threading_thread_destroy(threading_thread *thread)
{
	CloseHandle(*thread);
}
