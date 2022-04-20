/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
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

#ifndef METACALL_ERROR_H
#define METACALL_ERROR_H 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Member Data -- */

struct metacall_exception_type
{
	const char *message;
	const char *label;
	int64_t code;
	const char *stacktrace;
};

/* -- Type Definitions -- */

typedef struct metacall_exception_type *metacall_exception;

/* -- Methods -- */

/**
*  @brief
*    Retrieve the exception from a value, it can be either a throwable value with an exception inside or an exception itself
*
*  @param[in] v
*    Value that represents the exception to be retrieved
*
*  @param[out] ex
*    Exception that will be used as out parameter, the lifetime of the struct fields is attached to @v
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_error_from_value(void *v, metacall_exception ex);

/**
*  @brief
*    Retrieve last error that has happened after a call to any API from MetaCall
*
*  @param[out] ex
*    Exception that will be used as out parameter, the lifetime of the struct fields is attached to the internal MetaCall exception
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_error_last(metacall_exception ex);

/**
*  @brief
*    Clear last error that has happened after a call to any API from MetaCall
*/
METACALL_API void metacall_error_clear(void);

#ifdef __cplusplus
}
#endif

#endif /* METACALL_ERROR_H */
