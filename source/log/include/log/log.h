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

#ifndef LOG_H
#define LOG_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_level.h>
#include <log/log_record.h>
#include <log/log_policy.h>
#include <log/log_policy_format.h>
#include <log/log_policy_schedule.h>
#include <log/log_policy_storage.h>
#include <log/log_policy_stream.h>
#include <log/log_preprocessor.h>

#include <format/format_specifier.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdarg.h>

/* -- Methods -- */

LOG_API void * log_instance(void);

LOG_API void log_copy(void * instance);

LOG_API int log_create(const char * name);

LOG_API int log_define(const char * name, log_policy policy);

LOG_API int log_configure_impl(const char * name, size_t size, ...);

LOG_API int log_level(const char * name, const char * level, size_t length);

LOG_API int log_write_impl(const char * name, const size_t line, const char * func, const char * file, const enum log_level_id level, const char * message);

LOG_API int log_write_impl_va(const char * name, const size_t line, const char * func, const char * file, const enum log_level_id level, const char * message, ...);

LOG_API int log_clear(const char * name);

LOG_API int log_delete(const char * name);

LOG_API const char * log_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_H */
