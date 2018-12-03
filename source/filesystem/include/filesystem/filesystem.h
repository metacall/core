/*
 *	File System Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing file system, paths and files.
 *
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H 1

/* -- Headers -- */

#include <filesystem/filesystem_api.h>

/*
#include <filesystem/filesystem_file.h>
#include <filesystem/filesystem_directory.h>
#include <filesystem/filesystem_watcher.h>
#include <filesystem/filesystem_interface.h>
*/

#include <adt/adt_vector.h>
#include <adt/adt_set.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

typedef enum filesystem_flags_enumeration
{
	FILESYSTEM_FLAGS_NONE			= 0x00,
	FILESYSTEM_FLAGS_LOAD_LAZY		= 0x01 << 0x00,
	FILESYSTEM_FLAGS_WATCH_SYNC		= 0x01 << 0x01,
	FILESYSTEM_FLAGS_READABLE		= 0x01 << 0x02,
	FILESYSTEM_FLAGS_WRITEABLE		= 0x01 << 0x03

} filesystem_flags;

/* -- Forward Declarations -- */

struct file_type;

struct directory_type;

struct filesystem_type;

/* -- Type Definitions -- */

typedef struct file_type * file;

typedef struct directory_type * directory;

typedef struct filesystem_type * filesystem;

/* -- Methods -- */

/**
*  @brief
*    Creates a new file system based on a @root path
*
*  @param[in] root
*    String path where file system will be virtually mounted
*
*  @return
*    A handle to file system if success, null otherwhise
*/
FILESYSTEM_API filesystem filesystem_create(const char * root, filesystem_flags flags);

/**
*  @brief
*    Add a file into the file system
*
*  @param[in] fs
*    A handle to the file system
*
*  @param[in] file_path
*    Absolute path to the file
*
*  @return
*    Returns zero on success, different from zero otherwhise
*/
FILESYSTEM_API int filesystem_add_file(filesystem fs, const char * file_path);

/**
*  @brief
*    Add a directory into the file system
*
*  @param[in] fs
*    A handle to the file system
*
*  @param[in] directory_path
*    Absolute path to the directory
*
*  @return
*    Returns zero on success, different from zero otherwhise
*/
FILESYSTEM_API int filesystem_add_directory(filesystem fs, const char * directory_path);

/**
*  @brief
*    Add a file into the file system
*
*  @param[in] fs
*    A handle to the file system
*
*  @param[in] file_path
*    Absolute path to the file
*
*  @return
*    Returns zero on success, different from zero otherwhise
*/
FILESYSTEM_API int filesystem_remove_file(filesystem fs, const char * file_path);

/**
*  @brief
*    Remove a directory from the file system
*
*  @param[in] fs
*    A handle to the file system
*
*  @param[in] directory_path
*    Absolute path to the directory
*
*  @return
*    Returns zero on success, different from zero otherwhise
*/
FILESYSTEM_API int filesystem_remove_directory(filesystem fs, const char * directory_path);

/**
*  @brief
*    List all files contained in a file system
*
*  @param[in] fs
*    A handle to the file system
*
*  @param[out] files
*    Vector containing all files inside the file system
*
*  @return
*    Returns zero on success, different from zero otherwhise
*/
FILESYSTEM_API int filesystem_list_files(filesystem fs, vector files);

/**
*  @brief
*    List all files contained in a file system sorted by extension
*
*  @param[in] fs
*    A handle to the file system
*
*  @param[out] files
*    Set containing vectors of files sorted by extension
*
*  @return
*    Returns zero on success, different from zero otherwhise
*/
FILESYSTEM_API int filesystem_list_files_by_extension(filesystem fs, set files);

/**
*  @brief
*    List all directories contained in a file system
*
*  @param[in] fs
*    A handle to the file system
*
*  @param[out] directories
*    Vector containing all directories inside the file system
*
*  @return
*    Returns zero on success, different from zero otherwhise
*/
FILESYSTEM_API int filesystem_list_directories(filesystem fs, vector directories);

/**
*  @brief
*    Generates the cannonical path of a file
*
*  @param[in] fs
*    A handle to the file system
*
*  @param[in] file
*    Pointer reference to the file to be cannonicalized
*
*  @param[out] path
*    Vector of strings containing the cannonical path
*
*  @return
*    Returns zero on success, different from zero otherwhise
*/
FILESYSTEM_API int filesystem_cannonical_path_from_file(filesystem fs, file f, vector path);

/**
*  @brief
*    Generates the cannonical path of a directory
*
*  @param[in] fs
*    A handle to the file system
*
*  @param[in] directory
*    Pointer reference to the directory to be cannonicalized
*
*  @param[out] path
*    Vector of strings containing the cannonical path
*
*  @return
*    Returns zero on success, different from zero otherwhise
*/
FILESYSTEM_API int filesystem_cannonical_path_from_directory(filesystem fs, directory d, vector path);





/* TODO: filesystem_cannonical_path_to_string */





/**
*  @brief
*    Unmounts the virtual file system
*
*  @param[in] fs
*    Handle referencing the file system
*/
FILESYSTEM_API void filesystem_destroy(filesystem fs);

/**
*  @brief
*    Provide the module information
*
*  @return
*    Static string containing module information
*
*/
FILESYSTEM_API const char * filesystem_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* FILESYSTEM_H */
