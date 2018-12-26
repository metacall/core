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

#ifndef LOG_PREPROCESSOR_H
#define LOG_PREPROCESSOR_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <preprocessor/preprocessor_arguments.h>
#include <preprocessor/preprocessor_if.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define LOG_PREPROCESSOR_LINE ((size_t) __LINE__)

/* -- Macros -- */

#define log_configure(name, ...) \
	log_configure_impl(name, PREPROCESSOR_ARGS_COUNT(__VA_ARGS__), __VA_ARGS__)

#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || \
	(defined(__STDC__) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
#	define log_write(name, level, ...) \
		PREPROCESSOR_IF(PREPROCESSOR_ARGS_EMPTY(__VA_ARGS__), \
			log_write_impl(name, LOG_PREPROCESSOR_LINE, log_record_function(), __FILE__, level, __VA_ARGS__), \
			log_write_impl_va(name, LOG_PREPROCESSOR_LINE, log_record_function(), __FILE__, level, __VA_ARGS__) \
		)
#else
#	define log_write(name, level, message, ...) \
		PREPROCESSOR_IF(PREPROCESSOR_ARGS_EMPTY(__VA_ARGS__), \
			log_write_impl(name, LOG_PREPROCESSOR_LINE, log_record_function(), __FILE__, level, message), \
			log_write_impl_va(name, LOG_PREPROCESSOR_LINE, log_record_function(), __FILE__, level, message, __VA_ARGS__) \
		)
#endif

#ifdef __cplusplus
}
#endif

#endif /* LOG_PREPROCESSOR_H */
