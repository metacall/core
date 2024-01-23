/*
 *	File System Library by Parra Studios
 *	A cross-platform library for managing file system, paths and files.
 *
 *	Copyright (C) 2016 - 2024 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

/* -- Headers -- */

#include <filesystem/filesystem_directory_descriptor.h>
#include <filesystem/filesystem_file_descriptor.h>

#include <adt/adt_vector.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

/* -- Type Definitions -- */

typedef char directory_descriptor_path_str[DIRECTORY_DESCRIPTOR_PATH_SIZE];

/* -- Member Data -- */

struct directory_descriptor_type
{
	directory_descriptor parent;		/**< Pointer to parent directory */
	directory_descriptor_path_str path; /**< Path of current descriptor */
	vector files;						/**< String vector of files contained in current directory */
};

/* -- Methods -- */

directory_descriptor directory_descriptor_create(directory_descriptor parent, const char *path)
{
	directory_descriptor d = malloc(sizeof(struct directory_descriptor_type));

	if (d != NULL)
	{
		d->files = vector_create(sizeof(file_descriptor));

		if (d->files == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Directory descriptor invalid file allocation");

			free(d);

			return NULL;
		}

		d->parent = parent;

		strncpy(d->path, path, DIRECTORY_DESCRIPTOR_PATH_SIZE - 1);

		return d;
	}

	return NULL;
}

directory_descriptor directory_descriptor_parent(directory_descriptor d)
{
	if (d != NULL)
	{
		return d->parent;
	}

	return NULL;
}

const char *directory_descriptor_path(directory_descriptor d)
{
	if (d != NULL)
	{
		return d->path;
	}

	return NULL;
}

vector directory_descriptor_files(directory_descriptor d)
{
	if (d != NULL)
	{
		return d->files;
	}

	return NULL;
}

void directory_descriptor_destroy(directory_descriptor d)
{
	if (d != NULL)
	{
		if (d->files != NULL)
		{
			vector_destroy(d->files);
		}

		free(d);
	}
}
