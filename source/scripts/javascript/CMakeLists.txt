# Check if this script is enabled
if(NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_JS)
	return()
endif()

# Append cmake path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# JavaScript project utility
include(JavaScriptProject)

#
# Sub-projects
#

add_subdirectory(divide)
add_subdirectory(third)
add_subdirectory(spider)
add_subdirectory(ducktype)
