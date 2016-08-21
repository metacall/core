#
# External dependencies
#

find_package(SWIG REQUIRED)

if(NOT SWIG_FOUND)
    message(STATUS "SWIG not found")
    return()
endif()

include(${SWIG_USE_FILE})
