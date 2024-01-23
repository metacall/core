/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_POLICY_FORMAT_CUSTOM_H
#define LOG_POLICY_FORMAT_CUSTOM_H 1

/* -- Headers -- */

#include <log/log_api.h>

#include <log/log_policy.h>

#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Forward Declarations -- */

struct log_policy_format_custom_ctor_type;
struct log_policy_format_custom_va_list_type;

/* -- Type Definitions -- */

typedef struct log_policy_format_custom_ctor_type *log_policy_format_custom_ctor;
typedef struct log_policy_format_custom_va_list_type *log_policy_format_custom_va_list;

typedef size_t (*log_policy_format_custom_size_ptr)(void *, const char *, uint64_t, size_t, const char *, const char *, const char *, const char *, log_policy_format_custom_va_list);
typedef size_t (*log_policy_format_custom_serialize_ptr)(void *, void *, const size_t, const char *, uint64_t, size_t, const char *, const char *, const char *, const char *, log_policy_format_custom_va_list);
typedef size_t (*log_policy_format_custom_deserialize_ptr)(void *, const void *, const size_t, const char *, uint64_t, size_t, const char *, const char *, const char *, const char *, log_policy_format_custom_va_list);

/* -- Member Data -- */

struct log_policy_format_custom_va_list_type
{
	va_list va;
};

struct log_policy_format_custom_ctor_type
{
	void *context;
	log_policy_format_custom_size_ptr format_size;
	log_policy_format_custom_serialize_ptr format_serialize;
	log_policy_format_custom_deserialize_ptr format_deserialize;
};

/* -- Methods -- */

LOG_API log_policy_interface log_policy_format_custom_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* LOG_POLICY_FORMAT_CUSTOM_H */
