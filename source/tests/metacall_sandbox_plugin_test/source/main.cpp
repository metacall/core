/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#include <gtest/gtest.h>

#if defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux)
	#include <portability/portability_path.h>
	#include <unistd.h>

static int is_qemu(void)
{
	pid_t ppid = getppid();

	char link_path[64];
	char exe_path[PORTABILITY_PATH_SIZE];

	snprintf(link_path, sizeof(link_path), "/proc/%d/exe", ppid);

	ssize_t len = readlink(link_path, exe_path, sizeof(exe_path) - 1);

	if (len == -1)
	{
		return 0;
	}

	exe_path[len] = '\0';

	char exe_name[PORTABILITY_PATH_SIZE];
	size_t exe_name_size = portability_path_get_fullname(exe_path, len + 1, exe_name, PORTABILITY_PATH_SIZE);

	if (exe_name_size == 0)
	{
		return 0;
	}

	if (strncmp(exe_name, "qemu", std::min(exe_name_size, sizeof("qemu")) - 1) == 0)
	{
		return 1;
	}

	return 0;
}

#endif

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);

#if defined(unix) || defined(__unix__) || defined(__unix) || \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux)
	{
		/* QEMU fails to run properly this test */
		if (is_qemu())
		{
			return 0;
		}
	}
#endif

	return RUN_ALL_TESTS();
}
