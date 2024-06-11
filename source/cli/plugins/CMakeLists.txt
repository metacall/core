#
# NodeJS Dependency
#

find_package(NodeJS)

if(NOT NodeJS_FOUND)
	message(SEND_ERROR "NodeJS libraries not found")
	return()
endif()

# Generate output directories for CLI plugins
execute_process(
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_OUTPUT_DIR}/plugins/cli"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_OUTPUT_DIR}/plugins/cli/internal"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_OUTPUT_DIR}/plugins/cli/repl"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_OUTPUT_DIR}/plugins/cli/cmd"
)

#
# REPL Plugins
#

add_subdirectory(cli_repl_plugin)
add_subdirectory(cli_core_plugin)

#
# CMD Plugins
#

# NodeJS added util.parseArgs([config]) in versions v18.3.0, v16.17.0
# Check for compatibility, otherwise use fallback command parser in the CLI
if(NOT (NodeJS_VERSION VERSION_GREATER_EQUAL "18.3.0" OR (NodeJS_VERSION_MAJOR LESS 18 AND NodeJS_VERSION VERSION_GREATER_EQUAL "16.17.0")))
	message(WARNING "NodeJS version ${NodeJS_VERSION} does not support CLI command line plugins, at least v18.3.0 or v16.17.0 are required, using fallback command parser")
	return()
endif()

add_subdirectory(cli_cmd_plugin)
add_subdirectory(cli_sandbox_plugin)
