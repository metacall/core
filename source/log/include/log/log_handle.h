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

#ifndef LOG_HANDLER_H
#define LOG_HANDLER_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_record.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Forward Declarations -- */

struct log_handle_type;

/* -- Type Definitions -- */

typedef struct log_handle_type * log_handle;

/* -- Methods -- */

LOG_API log_handle log_handle_create(void);

LOG_API log_handle log_handle_initialize(log_handle handle);

LOG_API log_record log_handle_get(log_handle handle, const size_t record_id);

LOG_API log_record log_handle_set(log_handle handle, const size_t record_id, const log_record_ctor record_ctor);

LOG_API log_record log_handle_push(log_handle handle, const log_record_ctor record_ctor);

LOG_API log_record log_handle_pop(log_handle handle);

LOG_API size_t log_handle_count(log_handle handle);

LOG_API size_t log_handle_size(log_handle handle);

LOG_API int log_handle_clear(log_handle handle);

LOG_API int log_handle_destroy(log_handle handle);

#ifdef __cplusplus
}
#endif

#endif /* LOG_HANDLER_H */
