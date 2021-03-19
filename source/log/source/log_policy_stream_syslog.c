/*
*	Logger Library by Parra Studios
*	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A generic logger library providing application execution reports.
*
*/

/* -- Headers -- */

#include <log/log_policy_stream.h>
#include <log/log_policy_stream_syslog.h>

#if defined(_WIN32)
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif

	#include <windows.h>

	#if defined(__MINGW32__) || defined(__MINGW64__)
		#include <share.h>
	#endif
#elif defined(__linux__) || defined(__FreeBSD__)
	#include <syslog.h>
#endif

/* -- Definitions -- */

#if defined(_WIN32)
	#define LOG_POLICY_STREAM_SYSLOG_WIN_CATEGORY ((WORD)0x01L)
	#define LOG_POLICY_STREAM_SYSLOG_WIN_MSG	  ((DWORD)0x07D0L)
#endif

/* -- Forward Declarations -- */

struct log_policy_stream_syslog_data_type;

/* -- Type Definitions -- */

typedef struct log_policy_stream_syslog_data_type *log_policy_stream_syslog_data;

/* -- Member Data -- */

struct log_policy_stream_syslog_data_type
{
	const char *name;

#if defined(_WIN32)
	HANDLE handle;
#elif defined(__linux__) || defined(__FreeBSD__)
	/* ... */
#endif
};

/* -- Private Methods -- */

static int log_policy_stream_syslog_create(log_policy policy, const log_policy_ctor ctor);

static int log_policy_stream_syslog_write(log_policy policy, const void *buffer, const size_t size);

static int log_policy_stream_syslog_flush(log_policy policy);

static int log_policy_stream_syslog_destroy(log_policy policy);

/* -- Methods -- */

log_policy_interface log_policy_stream_syslog_interface()
{
	static struct log_policy_stream_impl_type log_policy_stream_syslog_impl_obj = {
		&log_policy_stream_syslog_write,
		&log_policy_stream_syslog_flush
	};

	static struct log_policy_interface_type policy_interface_stream = {
		&log_policy_stream_syslog_create,
		&log_policy_stream_syslog_impl_obj,
		&log_policy_stream_syslog_destroy
	};

	return &policy_interface_stream;
}

static int log_policy_stream_syslog_create(log_policy policy, const log_policy_ctor ctor)
{
	log_policy_stream_syslog_data syslog_data = malloc(sizeof(struct log_policy_stream_syslog_data_type));

	const log_policy_stream_syslog_ctor syslog_ctor = ctor;

	if (syslog_data == NULL)
	{
		return 1;
	}

	syslog_data->name = syslog_ctor->name;

#if defined(_WIN32)
	syslog_data->handle = RegisterEventSource(NULL, syslog_data->name);
#elif defined(__linux__) || defined(__FreeBSD__)
	openlog(syslog_data->name, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
#endif

	log_policy_instantiate(policy, syslog_data, LOG_POLICY_STREAM_SYSLOG);

	return 0;
}

static int log_policy_stream_syslog_write(log_policy policy, const void *buffer, const size_t size)
{
	log_policy_stream_syslog_data syslog_data = log_policy_instance(policy);

	/* Unused size, unsafe system log? */
	(void)size;

#if defined(_WIN32)
	LPTSTR lpt_str[1];

	lpt_str[0] = (LPTSTR)buffer;

	ReportEvent(syslog_data->handle,
		EVENTLOG_INFORMATION_TYPE,
		LOG_POLICY_STREAM_SYSLOG_WIN_CATEGORY,
		LOG_POLICY_STREAM_SYSLOG_WIN_MSG,
		NULL, 1, 0, (LPTSTR *)lpt_str, NULL);
#elif defined(__linux__) || defined(__FreeBSD__)
	(void)syslog_data;

	syslog(LOG_INFO, "%s", (const char *)buffer);
#endif

	return 0;
}

static int log_policy_stream_syslog_flush(log_policy policy)
{
	(void)policy;

	return 0;
}

static int log_policy_stream_syslog_destroy(log_policy policy)
{
	log_policy_stream_syslog_data syslog_data = log_policy_instance(policy);

	if (syslog_data != NULL)
	{
#if defined(_WIN32)
		if (syslog_data->handle != NULL)
		{
			DeregisterEventSource(syslog_data->handle);
		}
#elif defined(__linux__) || defined(__FreeBSD__)
		closelog();
#endif

		free(syslog_data);
	}

	return 0;
}
