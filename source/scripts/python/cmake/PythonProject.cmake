#
# Python project generator by Parra Studios
# Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# Generates a python project embedded into CMake
#

if(PYTHONPROJECT_FOUND)
	return()
endif()

set(PYTHONPROJECT_FOUND YES)

#
# Generic script project generator
#

include(ScriptProject)

# Define current python project configuration path
get_filename_component(PY_PROJECT_CONFIG_PATH ${CMAKE_CURRENT_LIST_FILE} PATH)

#
# Python sub-project util function
#

function(py_project target version)

	# Configuration
	set(PACKAGE_NAME		 ${target})
	set(PACKAGE_VERSION	  ${version})
	set(PACKAGE_SETUP_PY_IN "${PY_PROJECT_CONFIG_PATH}/PythonProject.py.in")
	set(PACKAGE_SETUP_PY	"${CMAKE_CURRENT_BINARY_DIR}/setup.py")
	set(PACKAGE_DEPS		"${CMAKE_CURRENT_SOURCE_DIR}/depends/__init__.py")
	set(PACKAGE_OUTPUT	  "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

	# Create python setup file
	configure_file(${PACKAGE_SETUP_PY_IN} ${PACKAGE_SETUP_PY} @ONLY)

	# Create project file
	script_project(${target} Python ${PY_PROJECT_CONFIG_PATH}/PythonProject.cmake.in)

endfunction()
