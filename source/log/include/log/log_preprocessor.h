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

#ifdef __cplusplus
extern "C" {
#endif

/* -- Macros -- */
	/*
#define log_preprocessor_write(...) \
	log_write_impl(log_write_required_args(__VA_ARGS__) log_write_variable_args(__VA_ARGS__))

#define log_write_required_args(...) \
	log_write_required_args_impl(__VA_ARGS__, ~)

#define log_write_required_args_impl(name, line, func, file, level, format, ...) \
	name, line, func, file, level, format




#define log_write_variable_args(...) \
	log_write_variable_args_impl()



	*/



#ifdef __cplusplus
}
#endif

#endif /* LOG_PREPROCESSOR_H */
