# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_EXT OR NOT OPTION_BUILD_EXTENSIONS OR NOT OPTION_BUILD_LOADERS_NODE)
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target cli_sandbox_plugin)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target}")

#
# Source
#

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/source")

#
# Destination
#

set(PLUGIN_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIR}/plugins/cli/cmd/${target}")

#
# Project Target
#

add_custom_target(${target} ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${source_path}/cli_sandbox_plugin_cmd.js ${PLUGIN_OUTPUT_DIRECTORY}/cli_sandbox_plugin_cmd.js
)

#
# Target Properties
#

set_target_properties(${target}
	PROPERTIES
	FOLDER "${IDE_FOLDER}"
)

#
# Dependencies
#

add_dependencies(${target}
	plugin_extension
	node_loader
)
