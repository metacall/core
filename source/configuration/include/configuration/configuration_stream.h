/*
 *	Configuration Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing multiple configuration formats.
 *
 */

#ifndef CONFIGURATION_STREAM_H
#define CONFIGURATION_STREAM_H 1

/* -- Headers -- */

#include <configuration/configuration_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

/**
*  @brief
*    Load configuration from @path into memory
*
*  @param[in] path
*    Path where is located the configuration file
*
*  @return
*    Returns pointer to stream on correct initialization, null otherwise
*
*/
CONFIGURATION_API char * configuration_stream_create(const char * path);

/**
*  @brief
*    Destroy configuration @stream from memory
*
*  @param[in] stream
*    Memory block describing a configuration
*
*  @return
*    Returns pointer to stream on correct initialization, null otherwise
*
*/
CONFIGURATION_API void configuration_stream_destroy(char * stream);

#ifdef __cplusplus
}
#endif

#endif /* CONFIGURATION_STREAM_H */
