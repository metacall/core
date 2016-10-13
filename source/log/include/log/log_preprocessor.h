/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_PREPROCESSOR_H
#define LOG_PREPROCESSOR_H 1

/* -- Headers -- */

#include <log/log_api.h>
/*
#include <preprocessor/preprocessor_if.h>
#include <preprocessor/preprocessor_va.h>
#include <preprocessor/preprocessor_foreach.h>
*/
#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */

#define log_configure(name, ...) \
	prerprocessor_foreach(__VA_ARGS__)

#define log_write(name, level, message, ...) \
	preprocessor_if(prerpocessor_va_count(__VA_ARGS__)) \
		log_write_impl_va(name, __LINE__, log_record_function(), __FILE__, level, message, __VA_ARGS__) \
	preprocessor_else() \
		log_write_impl(name, __LINE__, log_record_function(), __FILE__, level, message)

#ifdef __cplusplus
}
#endif

#endif /* LOG_PREPROCESSOR_H */
