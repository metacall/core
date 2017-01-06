/*
 *	File System Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing file system, paths and files.
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
	directory_descriptor		owner;				/**< Directory descriptor which file belongs to */
	file_descriptor_name_str	name;				/**< File name string */
	file_descriptor_extension_str	extension;		/**< File extension string */

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

	return "invalid_name";
}

const char * file_descriptor_extension(file_descriptor f)
{
	if (f != NULL)
	{
		return f->extension;
	}

	return "invalid_extension";
}

void file_descriptor_destroy(file_descriptor f)
{
	if (f != NULL)
	{
		free(f);
	}
}
