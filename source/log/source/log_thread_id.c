/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_thread_id.h>

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
#elif defined(__linux__) || defined(__APPLE__)
#	define _GNU_SOURCE
#	include <unistd.h>
#	include <sys/syscall.h>
#	include <sys/types.h>
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
	#elif defined(__APPLE__)
		return syscall(SYS_thread_selfid);
	#elif defined(__FreeBSD__)
		long thread_id = 0;

		thr_self(&thread_id);

		return (thread_id < 0) ? 0 : (size_t)thread_id;
	#else
		return 0;
	#endif
}
