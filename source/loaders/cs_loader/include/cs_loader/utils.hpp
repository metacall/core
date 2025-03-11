/*
 *	Loader Library by Parra Studios
 *	A plugin for loading net code at run-time into a process.
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

#include <cstdlib>
#include <cstring>
#include <set>
#include <string>

/* TODO: Remove this limitation */

#if !defined(__GNUC__) || __GNUC__ < 5 || (__GNUC__ == 5 && __GNUC_MINOR__ < 3)
	#error THIS SOFTWARE CURRENTLY BUILDS ONLY ON GCC 5.3 OR NEWER!
#endif

#include <experimental/filesystem>
namespace SCCH_fs = std::experimental::filesystem;

void AddFilesFromDirectoryToTpaList(std::string directory, std::string &tpaList)
{
	for (auto &dirent : SCCH_fs::directory_iterator(directory))
	{
		std::string path = dirent.path();

		if (!path.compare(path.length() - 4, 4, ".dll"))
		{
			tpaList.append(path + ":");
		}
	}
}
