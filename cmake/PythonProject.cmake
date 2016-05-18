#
# Python project generator by Parra Studios
# Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
# Generates a python project embedded into CMake
#

if(PYTHONPROJECT_FOUND)
    return()
endif()

set(PYTHONPROJECT_FOUND YES)

#
# External dependencies
#

find_package(PythonInterp)

if(NOT PYTHONINTERP_FOUND)
    message(STATUS "Python interpreter not found")
    return()
endif()

# Define current python project configuration path
get_filename_component(py_project_config_path ${CMAKE_CURRENT_LIST_FILE} PATH)

#
# Python sub-project util function
#

function(py_project name version)

    # Configuration
    set(PACKAGE_NAME         ${name})
    set(PACKAGE_VERSION      ${version})
    set(PACKAGE_SETUP_PY_IN "${py_project_config_path}/PythonProject.py.in")
    set(PACKAGE_SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
    set(PACKAGE_DEPS        "${CMAKE_CURRENT_SOURCE_DIR}/depends/__init__.py")
    set(PACKAGE_OUTPUT      "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

    # Create python setup file
    configure_file(${PACKAGE_SETUP_PY_IN} ${PACKAGE_SETUP_PY})

    # Create project file
    configure_file(${py_project_config_path}/PythonProject.cmake.in python-${name}-config.cmake)

    # Include generated project file
    include(${CMAKE_CURRENT_BINARY_DIR}/python-${name}-config.cmake)

endfunction()
