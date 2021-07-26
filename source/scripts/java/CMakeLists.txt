# Check if this script is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_JAVA OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_JAVA)
	return()
endif()

# Append cmake path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Java project utility
include(JavaProject)

#
# Sub-projects
#

add_subdirectory(fibonacci)
add_subdirectory(jartest)
add_subdirectory(test)

