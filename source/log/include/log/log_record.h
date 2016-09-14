/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_RECORD_H
#define LOG_RECORD_H 1

/* -- Headers -- */

#include <log/log_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

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
};

/* -- Macros -- */
/*
#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#	define log_record_function() __FUNCTION__
#elif defined(__GNUC__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)
#	define log_record_function() __PRETTY_FUNCTION__
#elif defined(__BORLANDC__)
#	define log_record_function() __FUNC__
#elif defined(__STDC__) && defined(__STDC_VERSION__) && (__STDC_VERION__ >= 199901L)
#	define log_record_function() __func__
#else
	static const char __unknown_func__[] = "unknown_function";

#	define log_record_function() __unknown_func__
#endif

#define log_record_create(record, level, message) \
	do \
	{ \
		const struct log_record_ctor_type record_ctor = \
		{ \
			__LINE__, log_record_function(), __FILE__, level, message \
		}; \
		\
		record = log_record_create_impl(&record_ctor); \
	} while (0)


#define log_record_initialize(record, level, message) \
	do \
	{ \
		const struct log_record_ctor_type record_ctor = \
		{ \
			__LINE__, log_record_function(), __FILE__, level, message \
		}; \
		\
		record = log_record_initialize_impl(record, &record_ctor); \
	} while (0)
*/

/* -- Protected Methods -- */

LOG_NO_EXPORT const size_t log_record_size(void);

/* -- Methods -- */

LOG_API log_record log_record_create(const log_record_ctor record_ctor);

LOG_API log_record log_record_initialize(log_record record, const log_record_ctor record_ctor);

LOG_API const time_t * const log_record_time(log_record record);

LOG_API const size_t log_record_thread_id(log_record record);

LOG_API const size_t log_record_line(log_record record);

LOG_API const char * const log_record_func(log_record record);

LOG_API const char * const log_record_file(log_record record);

LOG_API const enum log_level_id log_record_level(log_record record);

LOG_API const char * log_record_message(log_record record);

LOG_API int log_record_destroy(log_record record);

#ifdef __cplusplus
}
#endif

#endif /* LOG_RECORD_H */
