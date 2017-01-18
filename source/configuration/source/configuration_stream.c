/*
*	Configuration Library by Parra Studios
*	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
*
*	A cross-platform library for managing multiple configuration formats.
*
*/

/* -- Headers -- */

#include <configuration/configuration_stream.h>

#include <log/log.h>

#include <stdlib.h>
#include <stdio.h>

/* -- Methods -- */

char * configuration_stream_create(const char * path)
{
	FILE * file = fopen(path, "rb");

	size_t size, size_read;

	char * stream;

	if (file == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration file path (%s)", path);

		return NULL;
	}

	fseek(file, 0, SEEK_END);

	size = ftell(file);

	fseek(file, 0, SEEK_SET);

	stream = malloc((size + 1) * sizeof(char));

	if (stream == NULL)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration file stream allocation");

		fclose(file);

		return NULL;
	}

	size_read = fread(stream, sizeof(char), size, file);

	fclose(file);

	if (size_read != size)
	{
		log_write("metacall", LOG_LEVEL_ERROR, "Invalid configuration file size read (%u != %u)", size_read, size);

		free(stream);

		return NULL;
	}

	stream[size_read] = '\0';

	return stream;
}

void configuration_stream_destroy(char * stream)
{
	if (stream != NULL)
	{
		free(stream);
	}
}
