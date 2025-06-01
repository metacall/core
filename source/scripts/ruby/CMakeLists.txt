# Check if this script is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_RB OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_RB)
	return()
endif()

# Append cmake path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Ruby project utility
include(RubyProject)

#
# Sub-projects
#

add_subdirectory(hello)
add_subdirectory(second)
add_subdirectory(blog)
add_subdirectory(cache)
add_subdirectory(ducktype)
add_subdirectory(invalid)
add_subdirectory(klass)
add_subdirectory(failempty)
add_subdirectory(simplest)
