/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_RECORD_H
#define LOG_RECORD_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_level.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Macros -- */

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#	define log_record_function() __FUNCTION__
#elif defined(__GNUC__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)
#	if (defined(__cplusplus) && (__cplusplus >= 201103L)) || \
		(defined(__STDC__) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))

#		define log_record_function() __func__
#	else
#		define log_record_function() __PRETTY_FUNCTION__
#	endif
#elif defined(__BORLANDC__)
#	define log_record_function() __FUNC__
#elif defined(__STDC__) && defined(__STDC_VERSION__) && (__STDC_VERION__ >= 199901L)
#	define log_record_function() __func__
#else
#	define LOG_RECORD_FUNCTION_UNKNOWN_IMPL 1
#	log_record_function() __log_record_unknown_function__()
#endif

/* -- Forward Declarations -- */

struct log_record_ctor_type;

struct log_record_type;

/* -- Type Definitions -- */

typedef struct log_record_ctor_type * log_record_ctor;

typedef struct log_record_type * log_record;

/* -- Member Data -- */

struct log_record_ctor_type
{
	size_t line;
	const char * func;
	const char * file;
	enum log_level_id level;
	const char * message;
	void * data;
};

/* -- Private Methods -- */

#if defined(LOG_RECORD_FUNCTION_UNKNOWN_IMPL)
	LOG_API const char * __log_record_unknown_function__(void);
#endif

/* -- Protected Methods -- */

LOG_NO_EXPORT size_t log_record_size(void);

/* -- Methods -- */

LOG_API log_record log_record_create(const log_record_ctor record_ctor);

LOG_API log_record log_record_initialize(log_record record, const log_record_ctor record_ctor);

LOG_API const time_t * log_record_time(log_record record);

LOG_API size_t log_record_thread_id(log_record record);

LOG_API size_t log_record_line(log_record record);

LOG_API const char * log_record_func(log_record record);

LOG_API const char * log_record_file(log_record record);

LOG_API enum log_level_id log_record_level(log_record record);

LOG_API const char * log_record_message(log_record record);

LOG_API void * log_record_data(log_record record);

LOG_API int log_record_destroy(log_record record);

#ifdef __cplusplus
}
#endif

#endif /* LOG_RECORD_H */
