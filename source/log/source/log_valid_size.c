/*
 *	Logger Library by Parra Studios
 *	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A generic logger library providing application execution reports.
 *
 */

/* -- Headers -- */

#include <log/log_valid_size.h>

/* -- Methods -- */

size_t log_valid_size(size_t size)
{
	if (size == 0)
	{
		return 1;
	}

	if (size & (size - 1))
	{
		return size;
	}

	/* Calculate next power of two */
	--size;

	size |= size >> 0x01;
	size |= size >> 0x02;
	size |= size >> 0x04;
	size |= size >> 0x08;
	size |= size >> 0x10;

	++size;

	return size;
}
