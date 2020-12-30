/*
 *	Abstract Data Type Library by Parra Studios
 *	A abstract data type library providing generic containers.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <threading/threading_thread_id.h>

#include <stdlib.h>

#if (defined(_POSIX_VERSION) || defined(_POSIX2_C_VERSION)) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__) || \
	((defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)) || \
	defined(__unix__)
#	define THREADING_POSIX 1 /* Uses POSIX */
#	include <pthread.h>
#elif defined(WIN32) || defined(_WIN32)
#	define THREADING_WIN32 1 /* Uses WinAPI */
#	include <windows.h>
#else
#	error "Unsupported platform"
#endif

/* -- Member Data -- */

struct thread_id_type
{
#if defined(THREADING_POSIX)
	pthread_t id;
#elif defined(THREADING_WIN32)
	DWORD id;
#endif
};

/* -- Methods -- */

thread_id thread_id_get_current()
{
	thread_id current = malloc(sizeof(struct thread_id_type));

	if (current == NULL)
	{
		return NULL;
	}

	#if defined(THREADING_POSIX)
		current->id = pthread_self();
	#elif defined(THREADING_WIN32)
		current->id = GetCurrentThreadId();
	#endif

	return current;
}

int thread_id_compare(thread_id left, thread_id right)
{
	#if defined(THREADING_POSIX)
		return pthread_equal(left->id, right->id) == 0 ? 1 : 0;
	#elif defined(THREADING_WIN32)
		return left->id == right->id ? 0 : 1;
	#endif
}

void thread_id_destroy(thread_id id)
{
	if (id != NULL)
	{
		free(id);
	}
}
