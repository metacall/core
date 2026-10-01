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

int threading_thread_create(threading_thread *thread, threading_thread_routine routine, void *arg)
{
	return pthread_create(thread, NULL, routine, arg);
}

int threading_thread_join(threading_thread *thread, void **result)
{
	return pthread_join(*thread, result);
}

int threading_thread_detach(threading_thread *thread)
{
	return pthread_detach(*thread);
}

threading_thread_id threading_thread_self(void)
{
	return pthread_self();
}

void threading_thread_destroy(threading_thread *thread)
{
	/* Nothing to do here, pthread_join already reclaims all the resources
	of the thread and pthread_detach must not be followed by a destroy */
	(void)thread;
}
