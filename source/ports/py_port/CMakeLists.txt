# TODO
return()

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

set(Python_ADDITIONAL_VERSIONS 3.7)

find_package(PythonInterp REQUIRED)

if(NOT PYTHONINTERP_FOUND)
	message(STATUS "Python interpreter not found")
	return()
endif()

#
# Port name and options
#

# Target name
set(target py_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")






# TODO




# TODO: This is a nasty mock. It only will work if python port library (binary) is already installed and the
# environment variable correctly set up. This must be removed when metacall is correctly distributed
# and we can automate all distributions for all architectures, operative systems and all ports for all languages
if(NOT OPTION_BUILD_GUIX)
	install(CODE "execute_process(COMMAND pip3 install ${CMAKE_CURRENT_SOURCE_DIR}/package)")
endif()

#
# Configure test
#

# Check if loaders are enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_RB OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_SCRIPTS_RB)
	return()
endif()

set(py_port_test "${target}_test")
set(py_port_test_path "${CMAKE_CURRENT_BINARY_DIR}/${py_port_test}.py")

# Import module name
if(NOT CMAKE_BUILD_TYPE)
	message(WARNING "CMAKE_BUILD_TYPE must be specified in order to generate Python port module")
	return()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	get_target_property(DEBUG_POSTFIX ${SWIG_MODULE_${target}_REAL_NAME} "DEBUG_POSTFIX")
	set(PY_PORT_NAME "${SWIG_MODULE_${target}_REAL_NAME}${DEBUG_POSTFIX}")
else()
	set(PY_PORT_NAME "${SWIG_MODULE_${target}_REAL_NAME}")
endif()

if(MSVC)
	# TODO:
	#	Windows loads Release pyhton libraries with working with Debug
	#	The library name doesn't has the "_" at the beginning
	#	Also, exporting "d" postfix in release mode for the "PyInit__portd" entry point
	set(PY_PORT_NAME "_${PY_PORT_NAME}")

	message(WARNING "Deploy of python port library not properly working in MSVC (errors with naming & symbols)")
endif()

configure_file(test/run.py.in ${py_port_test_path} @ONLY)

#
# Define test
#

if(NOT ${PYTHON_VERSION_MAJOR} EQUAL 3)
	message(WARNING "Invalid Python executable version: ${PYTHON_VERSION_STRING}")
	message(WARNING "Define a valid Python 3 executable in variable PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE} or ${target} test won't be compiled")
	message(WARNING "Use: \"sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10\" to modify default python executable")
	return()
endif()

add_test(NAME ${target}
	COMMAND ${PYTHON_EXECUTABLE} ${py_port_test_path}
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
