/*
 *	Portability Library by Parra Studios
 *	A generic cross-platform portability utility.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <portability/portability_executable_path.h>

#include <string.h>

#if (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
	#include <mach-o/dyld.h>
#elif defined(__FreeBSD__)
	#include <sys/sysctl.h>
#endif

int portability_executable_path(portability_executable_path_str path, portability_executable_path_length *length)
{
	const size_t path_max_length = PORTABILITY_PATH_SIZE;

	/* Reset the path */
	memset(path, 0, path_max_length);

#if defined(WIN32) || defined(_WIN32) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) || \
	defined(__MINGW32__) || defined(__MINGW64__)
	*length = GetModuleFileName(NULL, path, (DWORD)path_max_length);
#elif defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux)
	*length = readlink("/proc/self/exe", path, path_max_length);
#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)
	uint32_t size = (uint32_t)PORTABILITY_PATH_SIZE;
	char executable_path[PORTABILITY_PATH_SIZE] = { 0 };

	if (_NSGetExecutablePath(executable_path, &size) != 0)
	{
		return 1;
	}

	if (realpath(executable_path, path) == NULL)
	{
		return 1;
	}

	*length = strnlen(path, PORTABILITY_PATH_SIZE);
#elif defined(__FreeBSD__)
	int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
	*length = sizeof(char) * path_max_length;
	if (sysctl(name, sizeof(name) / sizeof(name[0]), path, length, NULL, 0) < 0)
	{
		return 1;
	}
#elif defined(__NetBSD__)
	*length = readlink("/proc/curproc/exe", path, path_max_length);
#elif defined(__DragonFly__)
	*length = readlink("/proc/curproc/file", path, path_max_length);
#elif defined(sun) || defined(__sun)
	const char *path_ptr = getexecname();
	*length = strnlen(path_ptr, path_max_length);
	strncpy(path, path_ptr, *length);
#endif

	if (*length <= 0 || *length == path_max_length)
	{
		return 1;
	}

	return 0;
}
