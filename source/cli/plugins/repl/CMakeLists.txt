# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_LOADERS_EXT OR NOT OPTION_BUILD_EXTENSIONS)
	return()
endif()

#
# Plugin name and options
#

if(NOT OPTION_BUILD_GUIX)
	find_package(NPM)

	if(NOT NPM_FOUND)
		message(STATUS "NPM not found")
		return()
	endif()
endif()

# Target name
set(target repl)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target} ")

#
# Create target
#

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")

set(sources
	${source_path}/repl.js
	${source_path}/metacall.json
)

# Group source files
set(source_group "Source Files")
source_group_by_path(${source_path}  "\\\\.js$|\\\\.json"
	${source_group} ${sources})

set(PLUGIN_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIR}/plugins/cli/${target}")

#
# Define dependencies
#

# Copy lib
add_custom_target(${target} ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${PLUGIN_OUTPUT_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy ${source_path}/metacall.json ${PLUGIN_OUTPUT_DIRECTORY}/metacall.json
	COMMAND ${CMAKE_COMMAND} -E copy ${source_path}/repl.js ${PLUGIN_OUTPUT_DIRECTORY}/repl.js
	SOURCES ${sources}
	DEPENDS ${sources}
)

set_target_properties(${target}
	PROPERTIES
	FOLDER "${IDE_FOLDER}"
)
