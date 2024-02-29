#Check if extension loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_EXT OR NOT OPTION_BUILD_EXTENSIONS)
	return()
endif()

# Extension sub-projects
add_subdirectory(cli_repl_plugin)
add_subdirectory(cli_cmd_plugin)
add_subdirectory(cli_core_plugin)
add_subdirectory(cli_sandbox_plugin)

# Generate output directories for CLI plugins
execute_process(
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_OUTPUT_DIR}/plugins/cli"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_OUTPUT_DIR}/plugins/cli/internal"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_OUTPUT_DIR}/plugins/cli/repl"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_OUTPUT_DIR}/plugins/cli/cmd"
)
