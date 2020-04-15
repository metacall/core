/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef METACALL_LOG_H
#define METACALL_LOG_H 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

/* -- Enumerations -- */

enum metacall_log_id
{
	METACALL_LOG_STDIO,
	METACALL_LOG_FILE,
	METACALL_LOG_SOCKET,
	METACALL_LOG_SYSLOG,
	METACALL_LOG_NGINX,
	METACALL_LOG_CUSTOM
};

/* -- Forward Declarations -- */

struct metacall_log_stdio_type;

struct metacall_log_file_type;

struct metacall_log_socket_type;

struct metacall_log_syslog_type;

struct metacall_log_nginx_type;

struct metacall_log_custom_va_list_type;

struct metacall_log_custom_type;

/* -- Type Definitions -- */

typedef struct metacall_log_stdio_type * metacall_log_stdio;

typedef struct metacall_log_file_type * metacall_log_file;

typedef struct metacall_log_socket_type * metacall_log_socket;

typedef struct metacall_log_syslog_type * metacall_log_syslog;

typedef struct metacall_log_nginx_type * metacall_log_nginx;

typedef struct metacall_log_custom_va_list_type * metacall_log_custom_va_list;

typedef struct metacall_log_custom_type * metacall_log_custom;

/* -- Member Data -- */

struct metacall_log_stdio_type
{
	FILE * stream;
};

struct metacall_log_file_type
{
	const char * file_name;
	const char * mode;
};

struct metacall_log_socket_type
{
	const char * ip;
	uint16_t port;
};

struct metacall_log_syslog_type
{
	const char * name;
};

struct metacall_log_nginx_type
{
	void * log;
	void (*log_error)(void);
	uint16_t log_level;
};

struct metacall_log_custom_va_list_type
{
	va_list va;
};

struct metacall_log_custom_type
{
	void * context;
	size_t (*format_size)(void *, const char *, size_t, size_t, const char *, const char *, const char *, const char *, metacall_log_custom_va_list);
	size_t (*format_serialize)(void *, void *, const size_t, const char *, size_t, size_t, const char *, const char *, const char *, const char *, metacall_log_custom_va_list);
	size_t (*format_deserialize)(void *, const void *, const size_t, const char *, size_t, size_t, const char *, const char *, const char *, const char *, metacall_log_custom_va_list);
	int (*stream_write)(void *, const char *, const size_t);
	int (*stream_flush)(void *);
};

/* -- Methods -- */

/**
*  @brief
*    Create a log instance
*
*  @param[in] log_id
*    Type of log to be created
*
*  @param[in] ctx
*    Context of the log (a pointer to metacall_log_{stdio, file, socket, syslog, nginx, custom}_type)
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_log(enum metacall_log_id log_id, void * ctx);

#ifdef __cplusplus
}
#endif

#endif  /* METACALL_LOG_H */
