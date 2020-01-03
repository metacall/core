/*
 *	Logger Library by Parra Studios
 *	A generic logger library providing application execution reports.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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
