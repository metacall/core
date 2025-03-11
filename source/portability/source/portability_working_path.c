/*
 *	Portability Library by Parra Studios
 *	A generic cross-platform portability utility.
 *
 *	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <portability/portability_working_path.h>

#include <string.h>

#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)
	#include <winbase.h>
#else
	#include <unistd.h>
#endif

int portability_working_path(portability_working_path_str path, portability_working_path_length *length)
{
	const portability_working_path_length path_max_length = PORTABILITY_PATH_SIZE;

	/* Reset the path */
	memset(path, 0, path_max_length);

#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)
	*length = GetCurrentDirectory(0, NULL);

	if (*length == 0)
	{
		/* TODO: DWORD dw = GetLastError(); */
		return 1;
	}

	if (*length > path_max_length)
	{
		/* TODO: Handle error */
		return 1;
	}

	if (GetCurrentDirectory(*length, path) == 0)
	{
		/* TODO: DWORD dw = GetLastError(); */
		return 1;
	}
#else
	if (getcwd(path, path_max_length) == NULL)
	{
		*length = 0;
		/* TODO: Handle error */
		return 1;
	}

	*length = strnlen(path, path_max_length);
#endif

	return 0;
}
