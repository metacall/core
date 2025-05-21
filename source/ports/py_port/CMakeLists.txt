# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_PY OR NOT OPTION_BUILD_LOADERS_PY)
	return()
endif()

#
# External dependencies
#

find_package(Python3 COMPONENTS Interpreter REQUIRED)

#
# Port name and options
#

# Target name
set(target py_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

if(NOT OPTION_BUILD_GUIX)
	if(Python3_VERSION_MAJOR EQUAL 3 AND Python3_VERSION_MINOR GREATER_EQUAL 11)
		set(PIP_BREAK_SYSTEM_PACKAGES "--break-system-packages")
	else()
		set(PIP_BACKWARD_COMPATIBILITY "${CMAKE_COMMAND} -E env SETUPTOOLS_USE_DISTUTILS=stdlib")
	endif()

	install(CODE "execute_process(COMMAND ${PIP_BACKWARD_COMPATIBILITY} pip3 install ${PIP_BREAK_SYSTEM_PACKAGES} ${CMAKE_CURRENT_SOURCE_DIR})")
endif()

# Check if loaders are enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_RB OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_SCRIPTS_RB OR NOT OPTION_BUILD_SCRIPTS_NODE)
	return()
endif()

# Enable Rust test if the rs_loader is built
if(OPTION_BUILD_LOADERS_RS)
	set(TESTS_ENVIRONMENT_VARIABLES_RS "OPTION_BUILD_LOADERS_RS=1")
endif()

#
# Define test (CLI)
#

set(py_port_test "${target}_test")

if(NOT OPTION_BUILD_CLI)
	# Add test (CLI)
	add_test(NAME ${target}
		COMMAND $<TARGET_FILE:metacallcli> ${CMAKE_CURRENT_SOURCE_DIR}/test.py
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
		${TESTS_ENVIRONMENT_VARIABLES_RS}
	)
endif()

#
# Define test (Python)
#

set(py_port_test_exec "${py_port_test}_executable")

# Add test (Python)
add_test(NAME ${py_port_test_exec}
	COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py
)

#
# Define test labels
#

set_property(TEST ${py_port_test_exec}
	PROPERTY LABELS ${py_port_test_exec}
)

include(TestEnvironmentVariables)

test_environment_variables(${py_port_test_exec}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
	${TESTS_ENVIRONMENT_VARIABLES_RS}
	"METACALL_INSTALL_PATH=${PROJECT_OUTPUT_DIR}"
	${TESTS_SANITIZER_PRELOAD_ENVIRONMENT_VARIABLES}
)
