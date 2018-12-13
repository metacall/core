#
#	Ruby project generator by Parra Studios
#	Generates a ruby project embedded into CMake.
#
#	Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
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

if(RUBYPROJECT_FOUND)
	return()
endif()

set(RUBYPROJECT_FOUND YES)

#
# Generic script project generator
#

include(ScriptProject)

# Define current ruby project configuration path
get_filename_component(RB_PROJECT_CONFIG_PATH ${CMAKE_CURRENT_LIST_FILE} PATH)

#
# Ruby sub-project util function
#

function(rb_project target version)

	# TODO

	# Configuration
	set(PACKAGE_NAME		 ${target})
	set(PACKAGE_VERSION	  ${version})
	#set(PACKAGE_SETUP_RB_IN "${RB_PROJECT_CONFIG_PATH}/RubyProject.rb.in")
	#set(PACKAGE_SETUP_RB	"${CMAKE_CURRENT_BINARY_DIR}/setup.rb")
	#set(PACKAGE_DEPS		"${CMAKE_CURRENT_SOURCE_DIR}/depends/__init__.rb")
	set(PACKAGE_OUTPUT	  "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

	# Create ruby setup file
	#configure_file(${PACKAGE_SETUP_RB_IN} ${PACKAGE_SETUP_RB} @ONLY)

	# Create project file
	script_project(${target} Ruby ${RB_PROJECT_CONFIG_PATH}/RubyProject.cmake.in)

endfunction()
