#
#	CSharp project generator by Parra Studios
#	Generates a csharp project embedded into CMake.
#
#	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

if(CSHARPPROJECT_FOUND)
	return()
endif()

set(CSHARPPROJECT_FOUND YES)

#
# Generic script project generator
#

include(ScriptProject)

# Define current csharp project configuration path
get_filename_component(CS_PROJECT_CONFIG_PATH ${CMAKE_CURRENT_LIST_FILE} PATH)

#
# CSharp sub-project util function
#

function(cs_project target version)

	# TODO

	# Configuration
	set(PACKAGE_NAME		 ${target})
	set(PACKAGE_VERSION	  ${version})
	#set(PACKAGE_SETUP_CS_IN "${CS_PROJECT_CONFIG_PATH}/CSharpProject.json.in")
	#set(PACKAGE_SETUP_CS	"${CMAKE_CURRENT_BINARY_DIR}/setup.js")
	#set(PACKAGE_DEPS		"${CMAKE_CURRENT_SOURCE_DIR}/depends/__init__.js")
	set(PACKAGE_OUTPUT	  "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

	# Create csharp setup file
	#configure_file(${PACKAGE_SETUP_CS_IN} ${PACKAGE_SETUP_CS} @ONLY)

	# Create project file
	script_project(${target} CSharp ${CS_PROJECT_CONFIG_PATH}/CSharpProject.cmake.in)

endfunction()
