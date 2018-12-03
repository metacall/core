#
# Ruby project generator by Parra Studios
# Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# Generates a ruby project embedded into CMake
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
