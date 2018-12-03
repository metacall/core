/*
 *	File System Library by Parra Studios
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing file system, paths and files.
 *
 */

/* -- Headers -- */

#include <metacall/metacall_version.h>

#include <filesystem/filesystem.h>

#include <adt/adt_trie.h>

#include <log/log.h>

#include <stdlib.h>

/* -- Member Data -- */

struct filesystem_type
{
	filesystem_flags	flags;			/**< File system configuration flags */
	const char *		root;			/**< Root path where file system will be mounted */
	trie				storage;		/**< Trie containing whole file system directories and files */
};

/* -- Methods -- */

filesystem filesystem_create(const char * root, filesystem_flags flags)
{
	if (root != NULL)
	{
		filesystem fs = malloc(sizeof(struct filesystem_type));

		if (fs == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "File system invalid allocation");

			return NULL;
		}

		fs->storage = trie_create(&hash_callback_str, &comparable_callback_str);

		if (fs->storage == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "File system invalid storage allocation");

			free(fs);

			return NULL;
		}

		fs->root = root;

		fs->flags = flags;

		return fs;
	}

	return NULL;
}

int filesystem_add_file(filesystem fs, const char * file_path)
{
	(void)fs;
	(void)file_path;

	return 1;
}

int filesystem_add_directory(filesystem fs, const char * directory_path)
{
	(void)fs;
	(void)directory_path;

	return 1;
}

int filesystem_remove_file(filesystem fs, const char * file_path)
{
	(void)fs;
	(void)file_path;

	return 1;
}

int filesystem_remove_directory(filesystem fs, const char * directory_path)
{
	(void)fs;
	(void)directory_path;

	return 1;
}

int filesystem_list_files(filesystem fs, vector files)
{
	(void)fs;
	(void)files;

	return 1;
}

int filesystem_list_files_by_extension(filesystem fs, set files)
{
	(void)fs;
	(void)files;

	return 1;
}

int filesystem_list_directories(filesystem fs, vector directories)
{
	(void)fs;
	(void)directories;

	return 1;
}

int filesystem_cannonical_path_from_file(filesystem fs, file f, vector path)
{
	(void)fs;
	(void)f;
	(void)path;

	return 1;
}

int filesystem_cannonical_path_from_directory(filesystem fs, directory d, vector path)
{
	(void)fs;
	(void)d;
	(void)path;

	return 1;
}

void filesystem_destroy(filesystem fs)
{
	if (fs != NULL)
	{
		if (fs->storage != NULL)
		{
			trie_destroy(fs->storage);
		}

		free(fs);
	}
}

const char * filesystem_print_info()
{
	static const char filesystem_info[] =
		"File System Library " METACALL_VERSION "\n"
		"Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>\n"

		#ifdef FILESYSTEM_STATIC_DEFINE
			"Compiled as static library type"
		#else
			"Compiled as shared library type"
		#endif

		"\n";

	return filesystem_info;
}
