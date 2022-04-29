# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE)
	return()
endif()

# Target name
set(target cli_plugins)

# Exit here if required dependencies are not met
message(STATUS "CLI ${target}")

#
# Create target
#

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")
set(base_plugin_dir ${CMAKE_CURRENT_SOURCE_DIR}/base_plugins)
set(cli_plugins_output_dir ${PROJECT_OUTPUT_DIR}/cli_plugins)
set(base_plugins_output_dir ${cli_plugins_output_dir}/base_plugins)

set(sources
	${base_plugin_dir}/config_parser.js
)

# Group source files
set(source_group "Source Files")
source_group_by_path(${source_path}  "\\\\.js$\\\\.py$|\\\\.c$"
	${source_group} ${sources})

add_custom_target(${target}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${cli_plugins_output_dir}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${cli_plugins_output_dir}/base_plugins
	COMMAND ${CMAKE_COMMAND} -E copy ${base_plugin_dir}/config_parser.js ${base_plugins_output_dir}
	SOURCES ${sources}
	DEPENDS ${sources}
)

install(DIRECTORY
		${cli_plugins_output_dir}
		DESTINATION ${INSTALL_LIB}
		COMPONENT runtime
)
