# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_PY)
	return()
endif()

#
# External dependencies
#

#
# Port name and options
#

# Target name
set(target py_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

if(NOT OPTION_BUILD_GUIX)
	find_package(Python3 COMPONENTS Interpreter REQUIRED)

	if(Python3_VERSION_MAJOR EQUAL 3 AND Python3_VERSION_MINOR GREATER_EQUAL 11)
		set(PIP_BREAK_SYSTEM_PACKAGES "--break-system-packages")
	endif()

	install(CODE "execute_process(COMMAND pip3 install ${PIP_BREAK_SYSTEM_PACKAGES} ${CMAKE_CURRENT_SOURCE_DIR})")
endif()

#
# Configure test
#

# Check if loaders are enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_RB OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_SCRIPTS_RB OR NOT OPTION_BUILD_SCRIPTS_NODE)
	return()
endif()

set(py_port_test "${target}_test")

#
# Define test
#

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/test/commands/py_port.txt.in
	${CMAKE_CURRENT_BINARY_DIR}/py_port.txt
)

# Add test (must be run with MetaCall CLI)
add_test(NAME ${target}
	COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:metacallcli>" -D "INPUT=${CMAKE_CURRENT_BINARY_DIR}/py_port.txt" -P "${CMAKE_SOURCE_DIR}/source/cli/metacallcli/test/commands/command_runner.cmake"
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${py_port_test}
)

set_tests_properties(${target} PROPERTIES
	PASS_REGULAR_EXPRESSION "Tests passed without errors"
)

include(TestEnvironmentVariables)

# Enable rust test if it is built
if(OPTION_BUILD_LOADERS_RS)
	set(TESTS_ENVIRONMENT_VARIABLES_RS "OPTION_BUILD_LOADERS_RS=1")
endif()

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
	${TESTS_ENVIRONMENT_VARIABLES_RS}
)
