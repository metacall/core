#
# NodeJS project generator by Parra Studios
# Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# Generates a nodejs project embedded into CMake
#

if(NODEJSPROJECT_FOUND)
	return()
endif()

set(NODEJSPROJECT_FOUND YES)

#
# Generic script project generator
#

include(ScriptProject)

# Define current nodejs project configuration path
get_filename_component(NODEJS_PROJECT_CONFIG_PATH ${CMAKE_CURRENT_LIST_FILE} PATH)

#
# NodeJS sub-project util function
#

function(js_project target version)

	# TODO

	# Configuration
	set(PACKAGE_NAME		 ${target})
	set(PACKAGE_VERSION	  ${version})
	#set(PACKAGE_SETUP_NODEJS_IN "${NODEJS_PROJECT_CONFIG_PATH}/NodeJSProject.json.in")
	#set(PACKAGE_SETUP_NODEJS	"${CMAKE_CURRENT_BINARY_DIR}/setup.js")
	#set(PACKAGE_DEPS		"${CMAKE_CURRENT_SOURCE_DIR}/depends/__init__.js")
	set(PACKAGE_OUTPUT	  "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

	# Create nodejs setup file
	#configure_file(${PACKAGE_SETUP_NODEJS_IN} ${PACKAGE_SETUP_NODEJS} @ONLY)

	# Create project file
	script_project(${target} NodeJS ${JS_PROJECT_CONFIG_PATH}/NodeJSProject.cmake.in)

endfunction()
