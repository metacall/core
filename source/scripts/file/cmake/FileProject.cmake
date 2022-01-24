#
#	File project generator by Parra Studios
#	Generates a file project embedded into CMake.
#
#	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

#
# Generic script project generator
#

include(ScriptProject)

# Define current file project configuration path
get_filename_component(FILE_PROJECT_CONFIG_PATH ${CMAKE_CURRENT_LIST_FILE} PATH)

#
# File sub-project util function
#

function(file_project target version)

	# Configuration
	set(PACKAGE_NAME		${target})
	set(PACKAGE_VERSION		${version})

	# Create project file
	script_project(${target} File ${FILE_PROJECT_CONFIG_PATH}/FileProject.cmake.in)

endfunction()
