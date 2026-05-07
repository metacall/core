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
		set(PIP_BREAK_SYSTEM_PACKAGES --break-system-packages)
	else()
		set(PIP_BACKWARD_COMPATIBILITY ${CMAKE_COMMAND} -E env SETUPTOOLS_USE_DISTUTILS=stdlib)
	endif()

	# Retrieve the default system install path
	execute_process(
		COMMAND ${Python3_EXECUTABLE} -c "import sysconfig; print(sysconfig.get_paths()['purelib'])"
		OUTPUT_VARIABLE Python3_SITE_PACKAGES
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	# Install into a temporary folder so we can track the installed files in the install_manifest.txt
	set(PY_PORT_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/install")

	add_custom_target(py_port_install ALL
		COMMAND
			${PIP_BACKWARD_COMPATIBILITY}
			${Python3_EXECUTABLE} -m pip install
			--ignore-installed
			--no-deps
			${PIP_BREAK_SYSTEM_PACKAGES}
			--prefix=${PY_PORT_INSTALL_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}
	)

	# Install into the default system install path (note that --prefix replicates the site packages subtree)
	install(
		DIRECTORY
			"${PY_PORT_INSTALL_DIR}/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages/"
		DESTINATION
			"${Python3_SITE_PACKAGES}"
	)
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

if(OPTION_BUILD_CLI)
	message(STATUS "Test ${py_port_test}")

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

# Check if Python is compiled with Address Sanitizer
if(OPTION_BUILD_ADDRESS_SANITIZER)
	check_asan_executable("${Python3_EXECUTABLE}" Python3_ASAN)
	if(NOT Python3_ASAN)
		# Skip this test because it gives false positives if Python is not compiled with ASan
		return()
	endif()
endif()

message(STATUS "Test ${py_port_test_exec}")

# Add test (Python)
add_test(NAME ${py_port_test_exec}
	COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/test.py"
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
	${TESTS_SANITIZER_ENVIRONMENT_VARIABLES}
	${TESTS_SANITIZER_PRELOAD_ENVIRONMENT_VARIABLES}
)

#
# Define test (Python standalone)
#

set(py_port_test_exec_alone "${py_port_test}_executable_standalone")

message(STATUS "Test ${py_port_test_exec_alone}")

# Add test (Python standalone)
add_test(NAME ${py_port_test_exec_alone}
	COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/test.py"
)

#
# Define test labels
#

set_property(TEST ${py_port_test_exec_alone}
	PROPERTY LABELS ${py_port_test_exec_alone}
)

include(TestEnvironmentVariables)

test_environment_variables(${py_port_test_exec_alone}
	""
	"LOADER_SCRIPT_PATH=${LOADER_SCRIPT_PATH}"
	"METACALL_INSTALL_PATH=${PROJECT_OUTPUT_DIR}"
	${TESTS_SANITIZER_ENVIRONMENT_VARIABLES}
	${TESTS_SANITIZER_PRELOAD_ENVIRONMENT_VARIABLES}
)
