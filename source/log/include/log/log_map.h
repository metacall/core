/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

#ifndef LOG_MAP_H
#define LOG_MAP_H 1

/* -- Headers -- */

#include <log/log_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Headers -- */

#include <stdlib.h>

/* -- Definitions -- */

#define LOG_MAP_MIN_SIZE ((size_t)0x00000100)
#define LOG_MAP_MAX_SIZE ((size_t)0x00010000)

/* -- Forward Declarations -- */

struct log_map_type;

struct log_map_iterator_type;

/* -- Type Definitions -- */

typedef struct log_map_type *log_map;

typedef struct log_map_iterator_type *log_map_iterator;

/* -- Methods -- */

LOG_API log_map log_map_create(size_t size);

LOG_API size_t log_map_size(log_map map);

LOG_API size_t log_map_collisions(log_map map);

LOG_API int log_map_insert(log_map map, const char *key, const void *value);

LOG_API const void *log_map_get(log_map map, const char *key);

LOG_API const void *log_map_remove(log_map map, const char *key);

LOG_API int log_map_clear(log_map map);

LOG_API int log_map_destroy(log_map map);

LOG_API log_map_iterator log_map_iterator_begin(log_map map);

LOG_API const char *log_map_iterator_key(log_map_iterator iterator);

LOG_API const void *log_map_iterator_value(log_map_iterator iterator);

LOG_API int log_map_iterator_next(log_map_iterator iterator);

LOG_API int log_map_iterator_end(log_map_iterator iterator);

#ifdef __cplusplus
}
#endif

#endif /* LOG_MAP_H */
