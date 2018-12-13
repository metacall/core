/*
 *	File System Library by Parra Studios
 *	A cross-platform library for managing file system, paths and files.
 *
 *	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <filesystem/filesystem_file_descriptor.h>
#include <filesystem/filesystem_directory_descriptor.h>

#include <stdlib.h>

/* -- Type Definitions -- */

typedef char file_descriptor_name_str[FILE_DESCRIPTOR_NAME_SIZE];
typedef char file_descriptor_extension_str[FILE_DESCRIPTOR_EXTENSION_SIZE];

/* -- Member data -- */

struct file_descriptor_type
{
	directory_descriptor owner;					/**< Directory descriptor which file belongs to */
	file_descriptor_name_str name;				/**< File name string */
	file_descriptor_extension_str extension;	/**< File extension string */
};

/* -- Methods -- */

file_descriptor file_descriptor_create(directory_descriptor owner, const char * name)
{
	file_descriptor f = malloc(sizeof(struct file_descriptor_type));

	if (f != NULL)
	{
		f->owner = owner;

		/* TODO: extract name & extension */

		(void)name;

		return f;
	}

	return NULL;
}

directory_descriptor file_descriptor_owner(file_descriptor f)
{
	if (f != NULL)
	{
		return f->owner;
	}

	return NULL;
}

const char * file_descriptor_name(file_descriptor f)
{
	if (f != NULL)
	{
		return f->name;
	}

	return NULL;
}

const char * file_descriptor_extension(file_descriptor f)
{
	if (f != NULL)
	{
		return f->extension;
	}

	return NULL;
}

void file_descriptor_destroy(file_descriptor f)
{
	if (f != NULL)
	{
		free(f);
	}
}
