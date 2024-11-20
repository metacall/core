# Check if this script is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_C OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_C)
	return()
endif()

# Append cmake path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# C project utility
include(CProject)

#
# Sub-projects
#

add_subdirectory(compiled)
add_subdirectory(ffi)
add_subdirectory(cbks)
add_subdirectory(loadtest)
