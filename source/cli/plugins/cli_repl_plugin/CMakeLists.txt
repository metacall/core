# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_EXT OR NOT OPTION_BUILD_EXTENSIONS)
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target cli_repl_plugin)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target}")

#
# Source
#

set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/source")

#
# Destination
#

set(PLUGIN_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIR}/plugins/repl/${target}")

#
# Project Target
#

add_custom_target(${target} ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${source_path}/metacall.json ${PLUGIN_OUTPUT_DIRECTORY}/metacall.json
	COMMAND ${CMAKE_COMMAND} -E copy ${source_path}/cli_repl_plugin.js ${PLUGIN_OUTPUT_DIRECTORY}/cli_repl_plugin.js
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
)
