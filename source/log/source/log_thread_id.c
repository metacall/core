/*
 *	Logger Library by Parra Studios
 *	A generic logger library providing application execution reports.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <log/log_thread_id.h>

#if (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
#	include <AvailabilityMacros.h>
#endif

#if defined(_WIN32)
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif

#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif

#	include <windows.h>

#	if defined(__MINGW32__) || defined(__MINGW64__)
#		include <share.h>
#	endif
#elif defined(__linux__) || \
	((defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__) && (!defined(MAC_OS_X_VERSION_10_12) || MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12))
#	define _GNU_SOURCE
#	include <unistd.h>
#	include <sys/syscall.h>
#	include <sys/types.h>
#elif ((defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)) && (defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12)
#	include <pthread.h>
#elif defined(__FreeBSD__)
#	include <sys/thr.h>
#endif

/* -- Methods -- */

size_t log_thread_id()
{
	#if defined(_WIN32)
		return GetCurrentThreadId();
	#elif defined(__linux__)
	#	if defined(__ANDROID__) && defined(__ANDROID_API__) && (__ANDROID_API__ < 21)
			return syscall(__NR_gettid);
	#	else
			return syscall(SYS_gettid);
	#	endif
	#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
		#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
			uint64_t thread_id;

			pthread_threadid_np(NULL, &thread_id);

			return (size_t)thread_id;
		#else
			return syscall(SYS_thread_selfid);
		#endif
	#elif defined(__FreeBSD__)
		long thread_id = 0;

		thr_self(&thread_id);

		return (thread_id < 0) ? 0 : (size_t)thread_id;
	#else
		return 0;
	#endif
}
