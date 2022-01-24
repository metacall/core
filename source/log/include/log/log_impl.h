/*
 *	Logger Library by Parra Studios
 *	A generic logger library providing application execution reports.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef LOG_IMPL_H
#define LOG_IMPL_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_aspect.h>
#include <log/log_handle.h>
#include <log/log_record.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

LOG_API log_aspect_interface log_impl_aspect_interface(enum log_aspect_id aspect_id);

LOG_API log_impl log_impl_create(const char *name);

LOG_API void log_impl_define(log_impl impl, log_aspect aspect, enum log_aspect_id aspect_id);

LOG_API const char *log_impl_name(log_impl impl);

LOG_API log_handle log_impl_handle(log_impl impl);

LOG_API log_aspect log_impl_aspect(log_impl impl, enum log_aspect_id aspect_id);

LOG_API enum log_level_id log_impl_level(log_impl impl);

LOG_API void log_impl_verbosity(log_impl impl, enum log_level_id level);

LOG_API int log_impl_write(log_impl impl, const log_record_ctor record_ctor);

LOG_API int log_impl_clear(log_impl impl);

LOG_API int log_impl_destroy(log_impl impl);

#ifdef __cplusplus
}
#endif

#endif /* LOG_IMPL_H */
