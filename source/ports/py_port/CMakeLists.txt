# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_PY)
	return()
endif()

#
# External dependencies
#

find_package(PythonLibs 3 REQUIRED)

if(NOT PYTHONLIBS_FOUND)
	message(STATUS "Python libraries not found")
	return()
endif()

#
# Port name and options
#

# Target name
set(target py_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

#
# Configure test
#

# Check if loaders are enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_RB OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_SCRIPTS_RB)
	return()
endif()

set(py_port_test "${target}_test")

#
# Define test
#

if(WIN32)
	set(TEST_COMMAND cmd /c)
else()
	set(TEST_COMMAND sh -c)
endif()

# Add test (must be run with MetaCall CLI)
add_test(NAME ${target}
	COMMAND ${TEST_COMMAND} "echo 'load py ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py\nexit' | $<TARGET_FILE:metacallcli>"
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${py_port_test}
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)
