# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_NODE OR NOT OPTION_BUILD_CLI)
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target node_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

#
# Create target
#

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")

set(sources
	${source_path}/scripts/preinstall.js
	${source_path}/test/index.js
	${source_path}/index.js
	${source_path}/test.js
)

# Group source files
set(source_group "Source Files")
source_group_by_path(${source_path}  "\\\\.js$"
	${source_group} ${sources})

if(NOT OPTION_BUILD_GUIX)
	add_custom_target(${target} ALL
		COMMAND npm install --ignore-scripts
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		SOURCES ${sources}
	)

	set_target_properties(${target}
		PROPERTIES
		${DEFAULT_PROJECT_OPTIONS}
		FOLDER "${IDE_FOLDER}"
	)
endif()

#
# Install command
#

# Files
install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/index.js
	${CMAKE_CURRENT_SOURCE_DIR}/package.json
	${CMAKE_CURRENT_SOURCE_DIR}/package-lock.json
	${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
	DESTINATION ${INSTALL_LIB}/node_modules/metacall
	COMPONENT runtime
)

#
# Configure test
#

# Check if loaders are enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_RB OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_SCRIPTS_RB)
	return()
endif()

set(node_port_test "${target}_test")

#
# Define test
#

if(WIN32)
	set(TEST_COMMAND cmd /c)
	set(GREP_COMMAND findstr)
else()
	set(TEST_COMMAND sh -c)
	set(GREP_COMMAND grep)
endif()

add_test(NAME ${target}
	COMMAND ${TEST_COMMAND} "echo 'load node ${CMAKE_CURRENT_SOURCE_DIR}/test.js\nawait main()\nexit' | $<TARGET_FILE:metacallcli>"
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${node_port_test}
)

include(TestEnvironmentVariables)

# Enable cobol test if it is built
if(OPTION_BUILD_LOADERS_COB)
	set(TESTS_ENVIRONMENT_VARIABLES_COB "OPTION_BUILD_LOADERS_COB=1")
endif()

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
	${TESTS_ENVIRONMENT_VARIABLES_COB}
)
