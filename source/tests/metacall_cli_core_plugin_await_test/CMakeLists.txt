# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_EXT OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_NODE OR NOT TARGET cli_core_plugin)
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-cli-core-plugin-await-test)
message(STATUS "Test ${target}")

#
# Compiler warnings
#

include(Warnings)

#
# Compiler security
#

include(SecurityFlags)

#
# Sources
#

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target}")
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(sources
	${source_path}/main.cpp
	${source_path}/metacall_cli_core_plugin_await_test.cpp
)

# Group source files
set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$"
	${header_group} ${headers})
source_group_by_path(${source_path}  "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$"
	${source_group} ${sources})

#
# Create executable
#

# Build executable
add_executable(${target}
	${sources}
)

# Create namespaced alias
add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})

#
# Project options
#

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

#
# Include directories
#

target_include_directories(${target}
	PRIVATE
	${DEFAULT_INCLUDE_DIRECTORIES}
	${PROJECT_BINARY_DIR}/source/include
)

#
# Libraries
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LIBRARIES}

	GTest

	${META_PROJECT_NAME}::metacall
)

#
# Compile definitions
#

set(METACALL_PLUGIN_PATH "${PROJECT_OUTPUT_DIR}/metacall_cli_core_plugin_await_test")

target_compile_definitions(${target}
	PRIVATE
	${DEFAULT_COMPILE_DEFINITIONS}

	# Plugin path
	METACALL_PLUGIN_PATH="${METACALL_PLUGIN_PATH}"
)

# Isolate the plugin into a different directory in order to make it fully reproducible
add_custom_command(TARGET ${target} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${METACALL_PLUGIN_PATH}"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${METACALL_PLUGIN_PATH}/cli_core_plugin"
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_OUTPUT_DIR}/plugins/cli/repl/cli_core_plugin" "${METACALL_PLUGIN_PATH}/cli_core_plugin"
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/plugins/await_test" "${METACALL_PLUGIN_PATH}/await_test"
)

#
# Compile options
#

target_compile_options(${target}
	PRIVATE
	${DEFAULT_COMPILE_OPTIONS}
)

#
# Compile features
#

target_compile_features(${target}
	PRIVATE
	cxx_std_17
)

#
# Linker options
#

target_link_options(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

#
# Define test
#

add_test(NAME ${target}
	COMMAND $<TARGET_FILE:${target}>
)

#
# Define dependencies
#

add_dependencies(${target}
	ext_loader
	node_loader
	plugin_extension
	node_port
	cli_core_plugin
)

#
# Define test properties
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}

	# NodeJS Port path
	"METACALL_NODE_PORT_PATH=${CMAKE_SOURCE_DIR}/source/ports/node_port/index.js"
)
