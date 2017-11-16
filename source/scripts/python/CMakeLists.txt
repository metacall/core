# Check if this script is enabled
if(NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY)
	return()
endif()

# Append cmake path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Python project utility
include(PythonProject)

#
# Sub-projects
#

add_subdirectory(example)
add_subdirectory(sample)
add_subdirectory(frontend)
add_subdirectory(initfini)
add_subdirectory(callback)
add_subdirectory(ducktype)
add_subdirectory(rsasample)
