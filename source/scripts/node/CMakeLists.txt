# Check if this script is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_NODE)
	return()
endif()

# Append cmake path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# NodeJS project utility
include(NodeJSProject)

#
# Sub-projects
#

add_subdirectory(nod)
add_subdirectory(inline)
add_subdirectory(export)
add_subdirectory(host)
add_subdirectory(server)
