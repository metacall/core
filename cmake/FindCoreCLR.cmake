#
# CMake Find CoreCLR NET Engine by Parra Studios
# Copyright (C) 2016 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Find CoreCLR library and include paths
#
# CORECLR_FOUND - True if CoreCLR was found
# CORECLR_INCLUDE_DIR - CoreCLR headers path
# CORECLR_LIBRARIES - List of CoreCLR libraries
# CORECLR_CGINFO - List of CoreCLR libraries

# Prevent vervosity if already included
if(CORECLR_FOUND)
    set(CORECLR_FIND_QUIETLY TRUE)
endif()

# Include package manager
include(FindPackageHandleStandardArgs)


set(WITH_CORECLR "" CACHE PATH "Path to the directory where CoreCLR was built or installed.")
set(CORECLR_SEARCH_PATHS "")
set(CORECLR_INCLUDE_DIR "")
set(CORECLR_GCINFO "")

if(NOT WITH_CORECLR STREQUAL "")
    get_filename_component(WITH_CORECLR_ABS "${WITH_CORECLR}" ABSOLUTE)
    list(APPEND CORECLR_SEARCH_PATHS "${WITH_CORECLR_ABS}")
    set(CORECLR_INCLUDE_DIR "${WITH_CORECLR_ABS}/inc")
    set(CORECLR_GCINFO "${WITH_CORECLR_ABS}/gcinfo")
endif()

# Check for CoreCLR headers
find_path(CORINFO_H "corinfo.h" HINTS "${CORECLR_INCLUDE_DIR}" NO_CMAKE_FIND_ROOT_PATH)

if(CORINFO_H STREQUAL CORINFO_H-NOTFOUND)
    message(FATAL_ERROR "Cannot find corinfo.h in ${CORECLR_INCLUDE_DIR}. Please set WITH_CORECLR to a directory where CoreCLR was built or installed.")
    unset(WITH_CORECLR CACHE)
endif()

get_filename_component(CORECLR_INCLUDE_DIR "${CORECLR_INCLUDE_DIR}" DIRECTORY CACHE)

find_path(CORJIT_H "corjit.h" PATHS "${CORECLR_INCLUDE_DIR}" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
if(CORJIT_H STREQUAL CORJIT_H-NOTFOUND)
    message(FATAL_ERROR "Cannot find corjit.h. Please set WITH_CORECLR to a directory where CoreCLR was built or installed.")
endif()

find_path(CORERROR_H "corerror.h" PATHS "${CORECLR_INCLUDE_DIR}" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
if(CORERROR_H STREQUAL CORERROR_H-NOTFOUND)
    message(FATAL_ERROR "Cannot find corerror.h. Please set WITH_CORECLR to a directory where CoreCLR was built or installed.")
endif()

find_path(CORHDR_H "corhdr.h" PATHS "${CORECLR_INCLUDE_DIR}" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
if(CORHDR_H STREQUAL CORHDR_H-NOTFOUND)
    message(FATAL_ERROR "Cannot find corhdr.h. Please set WITH_CORECLR to a directory where CoreCLR was built or installed.")
endif()

find_path(COR_H "cor.h" PATHS "${CORECLR_INCLUDE_DIR}" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
if(COR_H STREQUAL COR_H-NOTFOUND)
    message(FATAL_ERROR "Cannot find cor.h. Please set WITH_CORECLR to a directory where CoreCLR was built or installed.")
endif()

find_path(OPCODE_DEF "opcode.def" PATHS "${CORECLR_INCLUDE_DIR}" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
if(OPCODE_DEF STREQUAL OPCODE_DEF-NOTFOUND)
    message(FATAL_ERROR "Cannot find opcode.def. Please set WITH_CORECLR to a directory where CoreCLR was built or installed.")
endif()

find_path(OPENUM_H "openum.h" PATHS "${CORECLR_INCLUDE_DIR}" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
if(OPENUM_H STREQUAL OPENUM_H-NOTFOUND)
    message(FATAL_ERROR "Cannot find openum.h. Please set WITH_CORECLR to a directory where CoreCLR was built or installed.")
endif()

find_path(GCINFOTYPES_H "gcinfotypes.h" PATHS "${CORECLR_INCLUDE_DIR}" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
if(GCINFOTYPES_H STREQUAL GCINFOENCODER_H-NOTFOUND)
    message(FATAL_ERROR "Cannot find gcinfotypes.h. Please set WITH_CORECLR to a directory where CoreCLR was built or installed.")
endif()

find_path(GCINFOENCODER_H "gcinfoencoder.h" PATHS "${CORECLR_INCLUDE_DIR}" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
if(GCINFOENCODER_H STREQUAL GCINFOENCODER_H-NOTFOUND)
    message(FATAL_ERROR "Cannot find gcinfoencoder.h. Please set WITH_CORECLR to a directory where CoreCLR was built or installed.")
endif()

find_path(GCINFOENCODER_CPP "gcinfoencoder.cpp" PATHS "${CORECLR_GCINFO}" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
if(GCINFOENCODER_CPP STREQUAL GCINFOENCODER_CPP-NOTFOUND)
    message(FATAL_ERROR "Cannot find gcinfoencoder.cpp. Please set WITH_CORECLR to a directory where CoreCLR was built or installed.")
endif()

if(UNIX)
    find_library(CORECLR_LIBRARY "coreclr"
        HINTS ${CORECLR_SEARCH_PATHS}
        DOC "Path to libcoreclr"
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH)
    find_library(CORECLR_LIBRARY "coreclr"
        DOC "Path to libcoreclr")

    if(NOT ${CORECLR_LIBRARY} STREQUAL CORECLR_LIBRARY-NOTFOUND AND EXISTS "${CORECLR_LIBRARY}" )
        get_filename_component(WITH_CORECLR_ABS "${CORECLR_SO_PATH}" DIRECTORY CACHE)
        link_directories("${WITH_CORECLR_ABS}")
    else()
        message(FATAL_ERROR "Could not find libcoreclr. Please set WITH_CORECLR to a directory where CoreCLR was built or installed.")
    endif()
endif()

find_package_handle_standard_args(CORECLR DEFAULT_MSG CORECLR_LIBRARY CORECLR_INCLUDE_DIR)

mark_as_advanced(CORECLR_LIBRARY CORECLR_INCLUDE_DIR)

if(CORECLR_FOUND)
	set(CORECLR_INCLUDE_DIRS ${CORECLR_INCLUDE_DIR})
endif()

message(STATUS "CoreCLR Libraries: ${CORECLR_LIBRARIES}")
