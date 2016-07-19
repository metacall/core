/*
 *	File System Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing file system, paths and files.
 *
 */

/* -- Headers -- */

#include <filesystem/file_descriptor.h>
#include <filesystem/directory_descriptor.h>

#include <stdlib.h>

/* -- Type Definitions -- */

typedef char file_descriptor_name_str[FILE_DESCRIPTOR_NAME_SIZE];
typedef char file_descriptor_extension_str[FILE_DESCRIPTOR_EXTENSION_SIZE];

/* -- Member data -- */

typedef struct file_descriptor_type
{
	directory			d;
	file_descriptor_name_str	name;
	file_descriptor_extension_str	extension;

} * file_descriptor;

/* -- Methods -- */

file_descriptor file_descriptor_invalid()
{
	static struct file_descriptor_type f =
	{
		NULL,
		"invalid_name",
		"invalid_ext"
	};

	return &f;
}

file_descriptor file_descriptor_create(const char * path)
{
	file_descriptor f = malloc(sizeof(struct file_descriptor_type));

	if (f != NULL)
	{
		(void)path;

		return f;
	}

	return NULL;
}

const char * file_descriptor_extension(file_descriptor f)
{
	if (f != NULL)
	{
		return f->extension;
	}

	return file_descriptor_invalid()->extension;
}

const char * file_descriptor_name(file_descriptor f)
{
	if (f != NULL)
	{
		return f->name;
	}

	return file_descriptor_invalid()->name;
}

const char * file_descriptor_path(file_descriptor f)
{
	if (f != NULL)
	{
		return directory_descriptor_path(f->d);
	}

	return directory_descriptor_path(file_descriptor_invalid()->d);
}

const char * file_descriptor_path_absolute(file_descriptor f)
{
	if (f != NULL)
	{
		return directory_descriptor_path_absolute(f->d);
	}

	return directory_descriptor_path_absolute(file_descriptor_invalid()->d);
}

void file_descriptor_destroy(file_descriptor f)
{
	if (f != NULL)
	{
		free(f);
	}
}
