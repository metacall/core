/*
 *	File System Library by Parra Studios
 *	A cross-platform library for managing file system, paths and files.
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#ifndef FILESYSTEM_FILE_DESCRIPTOR_H
#define FILESYSTEM_FILE_DESCRIPTOR_H 1

/* -- Headers -- */

#include <filesystem/filesystem_api.h>

#include <filesystem/filesystem_directory_descriptor.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define FILE_DESCRIPTOR_NAME_SIZE			0x0100
#define FILE_DESCRIPTOR_EXTENSION_SIZE		0x0012

/* -- Forward Declarations -- */

struct file_descriptor_type;

/* -- Type Definitions -- */

typedef struct file_descriptor_type * file_descriptor;

/* -- Methods -- */

/**
*  @brief
*    Create a file descriptor from specified directory and name
*
*  @param[in] owner
*    Directory which file belongs to
*
*  @param[in] name
*    The name of the file
*
*  @return
*    A pointer to the file descriptor if success, null pointer otherwhise
*/
FILESYSTEM_API file_descriptor file_descriptor_create(directory_descriptor owner, const char * name);

/**
*  @brief
*    Get the owner directory where is the file
*
*  @param[in] f
*    File descriptor pointer
*
*  @return
*    A pointer to directory descriptor which owns the file
*/
FILESYSTEM_API directory_descriptor file_descriptor_owner(file_descriptor f);

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
