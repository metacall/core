#
# JavaScript project generator by Parra Studios
# Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# Generates a javascript project embedded into CMake
#

if(JAVASCRIPTPROJECT_FOUND)
	return()
endif()

set(JAVASCRIPTPROJECT_FOUND YES)

#
# Generic script project generator
#

include(ScriptProject)

# Define current javascript project configuration path
get_filename_component(JS_PROJECT_CONFIG_PATH ${CMAKE_CURRENT_LIST_FILE} PATH)

#
# JavaScript sub-project util function
#

function(js_project target version)

	# TODO

	# Configuration
	set(PACKAGE_NAME		 ${target})
	set(PACKAGE_VERSION	  ${version})
	#set(PACKAGE_SETUP_JS_IN "${JS_PROJECT_CONFIG_PATH}/JavaScriptProject.json.in")
	#set(PACKAGE_SETUP_JS	"${CMAKE_CURRENT_BINARY_DIR}/setup.js")
	#set(PACKAGE_DEPS		"${CMAKE_CURRENT_SOURCE_DIR}/depends/__init__.js")
	set(PACKAGE_OUTPUT	  "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

	# Create javascript setup file
	#configure_file(${PACKAGE_SETUP_JS_IN} ${PACKAGE_SETUP_JS} @ONLY)

	# Create project file
	script_project(${target} JavaScript ${JS_PROJECT_CONFIG_PATH}/JavaScriptProject.cmake.in)

endfunction()
