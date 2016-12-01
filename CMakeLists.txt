#
#	MetaCall Ports by Parra Studios
#	Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#
#	Language porting infrastructure for MetaCall library.
#

#
# External dependencies
#

# SWIG
find_package(SWIG REQUIRED)

if(NOT SWIG_FOUND)
    message(STATUS "Swig not found")
    return()
endif()

include(${SWIG_USE_FILE})

# MetaCall Library
find_package(MetaCall REQUIRED)

if(NOT METACALL_FOUND)
    message(STATUS "MetaCall libraries not found")
    return()
endif()

include_directories(${METACALL_INCLUDE_DIRS})

#
# Source
#

add_subdirectory(source)
