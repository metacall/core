# Check if this script is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_CS OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_CS)
	return()
endif()

# Append cmake path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# CSharp project utility
include(CSharpProject)

#
# Sub-projects
#

add_subdirectory(hello)
add_subdirectory(static)
add_subdirectory(function)
