/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#ifndef METACALL_LOG_H
#define METACALL_LOG_H 1

/* -- Headers -- */

#include <metacall/metacall_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Enumerations -- */

enum metacall_log_id
{
	METACALL_LOG_STDIO,
	METACALL_LOG_FILE,
	METACALL_LOG_SOCKET,
	METACALL_LOG_SYSLOG,
	METACALL_LOG_NGNIX
};

/* -- Type Definitions -- */

typedef struct metacall_log_stdio_type * metacall_log_stdio;

typedef struct metacall_log_file_type * metacall_log_file;

typedef struct metacall_log_socket_type * metacall_log_socket;

typedef struct metacall_log_syslog_type * metacall_log_syslog;

typedef struct metacall_log_nginx_type * metacall_log_nginx;

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
	void (*log_error)();
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
*    Context of the log
*
*  @return
*    Zero if success, different from zero otherwise
*/
METACALL_API int metacall_log(enum metacall_log_id log_id, void * ctx);

#ifdef __cplusplus
}
#endif

#endif  /* METACALL_LOG_H */
