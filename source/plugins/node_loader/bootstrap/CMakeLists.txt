# Check if this plugin is enabled
if(NOT OPTION_BUILD_PLUGINS OR NOT OPTION_BUILD_PLUGINS_NODE)
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target node_loader_bootstrap)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target} bootstrap")

#
# Create target
#

add_custom_target(${target} ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/bootstrap.js ${CMAKE_BINARY_DIR}
)

#
# Project options
#

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

#
# Deployment
#

# Library (TODO)
#install(FILES
#	${CMAKE_BINARY_DIR}/bootstrap.js
#	DESTINATION ${INSTALL_LIB}
#)
