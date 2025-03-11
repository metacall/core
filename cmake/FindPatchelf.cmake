#
#	CMake Find Patchelf by Parra Studios
#	CMake script to find Patchelf executable.
#
#	Copyright (C) 2016 - 2025 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.
#

# Prevent vervosity if already included
if(Patchelf_FOUND)
	set(Patchelf_FIND_QUIETLY TRUE)
endif()

# Find patchelf
find_program(Patchelf_EXECUTABLE NAMES patchelf)

if(Patchelf_EXECUTABLE)
	set(Patchelf_FOUND TRUE)
else()
	set(Patchelf_FOUND FALSE)
endif()
