/*
 *	File System Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing file system, paths and files.
 *
 */

#ifndef FILESYSTEM_DIRECTORY_DESCRIPTOR_H
#define FILESYSTEM_DIRECTORY_DESCRIPTOR_H 1

/* -- Headers -- */

#include <filesystem/filesystem_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define DIRECTORY_DESCRIPTOR_PATH_SIZE		0x0200

/* -- Forward Declarations -- */

struct vector_type;

struct directory_descriptor_type;

/* -- Type Definitions -- */

typedef struct vector_type * vector;

typedef struct directory_descriptor_type * directory_descriptor;

/* -- Methods -- */

/**
*  @brief
*    Create a directory descriptor from specified path
*
*  @param[in] parent
*    Parent directory if any
*
*  @param[in] path
*    The path of the directory
*
*  @return
*    A pointer to the directory descriptor if success, null pointer otherwhise
*/
FILESYSTEM_API directory_descriptor directory_descriptor_create(directory_descriptor parent, const char * path);

/**
*  @brief
*    Get the parent directory
*
*  @param[in] d
*    Directory descriptor pointer
*
*  @return
*    A pointer to the parent directory descriptor
*/
FILESYSTEM_API directory_descriptor directory_descriptor_parent(directory_descriptor d);

/**
*  @brief
*    Get the path where directory points
*
*  @param[in] d
*    Directory descriptor pointer
*
*  @return
*    A constant string pointer to the path of @d
*/
FILESYSTEM_API const char * directory_descriptor_path(directory_descriptor d);

/**
*  @brief
*    Get the files contained in a directory
*
*  @param[in] d
*    Directory descriptor pointer
*
*  @return
*    A vector of files contained in directory
*/
FILESYSTEM_API vector directory_descriptor_files(directory_descriptor d);

/**
*  @brief
*    Destroy a directory from memory
*
*  @param[in] d
*    Directory descriptor pointer
*/
FILESYSTEM_API void directory_descriptor_destroy(directory_descriptor d);

#ifdef __cplusplus
}
#endif

#endif /* FILESYSTEM_DIRECTORY_DESCRIPTOR_H */
