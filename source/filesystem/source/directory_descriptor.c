/*
 *	File System Library by Parra Studios
 *	Copyright (C) 2009 - 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A cross-platform library for managing file system, paths and files.
 *
 */

/* -- Headers -- */

#include <filesystem/directory_descriptor.h>

#include <adt/vector.h>

/* -- Definitions -- */


/* -- Member Data -- */

typedef struct directory_descriptor_type
{
	directory_descriptor	parent;
	/*const char *		path;*/
	vector			childs;

} * directory_descriptor;

/* -- Methods -- */

directory_descriptor directory_descriptor_create(directory_descriptor parent, const char * path)
{

}

directory_descriptor directory_descriptor_parent(directory_descriptor d)
{

}

const char * directory_descriptor_path(directory_descriptor d)
{

}

void directory_descriptor_destroy(directory_descriptor d)
{

}
