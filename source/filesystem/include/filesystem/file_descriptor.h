/*
 *	File System Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing file system, paths and files.
 *
 */

#ifndef FILESYSTEM_FILE_DESCRIPTOR_H
#define FILESYSTEM_FILE_DESCRIPTOR_H 1

/* -- Headers -- */

#include <filesystem/filesystem_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define FILE_DESCRIPTOR_NAME_SIZE		0x0100
#define FILE_DESCRIPTOR_EXTENSION_SIZE		0x0010

/* -- Forward Declarations -- */

struct file_descriptor_type;

/* -- Type Definitions -- */

typedef struct file_descriptor_type * file_descriptor;

/* -- Methods -- */

/**
*  @brief
*    Create a file descriptor from specified path
*
*  @param[in] path
*    The path of the file
*
*  @return
*    A pointer to the file descriptor if success, null pointer otherwhise
*/
FILESYSTEM_API file_descriptor file_descriptor_create(const char * path);

/**
*  @brief
*    Get the file extension
*
*  @param[in] f
*    File descriptor pointer
*
*  @return
*    A constant string pointer to the extension of @f
*/
FILESYSTEM_API const char * file_descriptor_extension(file_descriptor f);

/**
*  @brief
*    Get the file name
*
*  @param[in] f
*    File descriptor pointer
*
*  @return
*    A constant string pointer to the name of @f
*/
FILESYSTEM_API const char * file_descriptor_name(file_descriptor f);

/**
*  @brief
*    Get the path where is the file
*
*  @param[in] f
*    File descriptor pointer
*
*  @return
*    A constant string pointer to the path of @f
*/
FILESYSTEM_API const char * file_descriptor_path(file_descriptor f);

/**
*  @brief
*    Get the full path with file name and extension
*
*  @param[in] f
*    File descriptor pointer
*
*  @return
*    A constant string pointer to the full path of @f
*/
FILESYSTEM_API const char * file_descriptor_path_absolute(file_descriptor f);

/**
*  @brief
*    Destroy a file from memory
*
*  @param[in] f
*    File descriptor pointer
*/
FILESYSTEM_API void file_descriptor_destroy(file_descriptor f);

#ifdef __cplusplus
}
#endif

#endif /* FILESYSTEM_FILE_DESCRIPTOR_H */
