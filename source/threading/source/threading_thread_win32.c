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

/* -- Private Methods -- */

static unsigned __stdcall threading_thread_win32_routine(void *data)
{
	threading_thread *thread = (threading_thread *)data;

	thread->result = thread->routine(thread->arg);

	return 0;
}

/* -- Methods -- */

int threading_thread_create(threading_thread *thread, threading_thread_routine routine, void *arg)
{
	thread->routine = routine;
	thread->arg = arg;
	thread->result = NULL;

	/* _beginthreadex is used instead of CreateThread because the thread calls into
	the C runtime and into Python, both of which require proper per-thread CRT
	initialization that only _beginthreadex guarantees */
	thread->handle = (HANDLE)_beginthreadex(NULL, 0, &threading_thread_win32_routine, thread, 0, NULL);

	if (thread->handle == NULL)
	{
		return 1;
	}

	return 0;
}

int threading_thread_join(threading_thread *thread, void **result)
{
	if (WaitForSingleObject(thread->handle, INFINITE) != WAIT_OBJECT_0)
	{
		return 1;
	}

	if (result != NULL)
	{
		*result = thread->result;
	}

	return 0;
}

int threading_thread_detach(threading_thread *thread)
{
	HANDLE handle = thread->handle;

	if (handle == NULL)
	{
		return 1;
	}

	thread->handle = NULL;

	if (CloseHandle(handle) == FALSE)
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
	if (thread->handle == NULL)
	{
		return;
	}

	CloseHandle(thread->handle);
	thread->handle = NULL;
}
