#
# Setup distributable environment
#

# Check if python plugin and distributable libs are enabled
if(NOT OPTION_BUILD_PLUGINS OR NOT OPTION_BUILD_PLUGINS_PY OR NOT BUILD_DISTRIBUTABLE_LIBS)
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

set(Python_ADDITIONAL_VERSIONS 3.4)

find_package(PythonInterp REQUIRED)

if(NOT PYTHONINTERP_FOUND)
	message(STATUS "Python interpreter not found")
	return()
endif()

# MetaCall Library
find_package(MetaCall REQUIRED)

if(NOT METACALL_FOUND)
	message(STATUS "MetaCall libraries not found")
	return()
endif()

#
# Executable name and options
#

# Target name
set(target py-django-integration-test)
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
	${source_path}/py_django_integration_test.cpp
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
# Dependecies
#

# Add metacall distributable dependency
add_dependencies(${target}
	${META_PROJECT_NAME}::metacall_distributable
)

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

	${METACALL_INCLUDE_DIRS} # MetaCall includes
)

#
# Libraries
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LIBRARIES}

	gmock-dev

	${CMAKE_DL_LIBS}

	${METACALL_LIBRARIES} # MetaCall libraries
)

#
# Compile definitions
#

target_compile_definitions(${target}
	PRIVATE
	${DEFAULT_COMPILE_DEFINITIONS}
)

#
# Compile options
#

target_compile_options(${target}
	PRIVATE
	${DEFAULT_COMPILE_OPTIONS}
)

#
# Linker options
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

#
# Define test
#

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	get_target_property(DEBUG_POSTFIX ${target} "DEBUG_POSTFIX")
	set(TEST_SCRIPT_LAUNCHER_BASE_NAME
		${target}${DEBUG_POSTFIX}
	)
else()
	set(TEST_SCRIPT_LAUNCHER_BASE_NAME
		${target}
	)
endif()

set(TEST_SCRIPT_LAUNCHER_NAME
	${TEST_SCRIPT_LAUNCHER_BASE_NAME}.py
)

set(TEST_SCRIPT_LAUNCHER_CLIENT_NAME
	${TEST_SCRIPT_LAUNCHER_BASE_NAME}-client.py
)

set(TEST_SCRIPT_LAUNCHER_FILE_NAME
	${TEST_SCRIPT_LAUNCHER_BASE_NAME}-file.txt
)

set(TEST_SCRIPT_LAUNCHER_NAME_IN
	"data/test.py.in"
)

set(TEST_SCRIPT_LAUNCHER_CLIENT_NAME_IN
	"data/client.py.in"
)

set(TEST_SCRIPT_LAUNCHER_FILE_NAME_IN
	"data/django_file.txt"
)

#
# Configure test files
#

if(MSVC)
	set(SERIAL_LIBRARY_PATH "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
	set(LOADER_LIBRARY_PATH "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
	set(TEST_SCRIPT_LAUNCHER_APP "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${TEST_SCRIPT_LAUNCHER_BASE_NAME}.exe")
else()
	set(SERIAL_LIBRARY_PATH "${PROJECT_BINARY_DIR}")
	set(LOADER_LIBRARY_PATH "${PROJECT_BINARY_DIR}")
	set(TEST_SCRIPT_LAUNCHER_APP "${PROJECT_BINARY_DIR}/${TEST_SCRIPT_LAUNCHER_BASE_NAME}")
endif()

configure_file(${TEST_SCRIPT_LAUNCHER_NAME_IN} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_SCRIPT_LAUNCHER_NAME})

configure_file(${TEST_SCRIPT_LAUNCHER_CLIENT_NAME_IN} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_SCRIPT_LAUNCHER_CLIENT_NAME})

configure_file(${TEST_SCRIPT_LAUNCHER_FILE_NAME_IN} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_SCRIPT_LAUNCHER_FILE_NAME} COPYONLY)

if(NOT ${PYTHON_VERSION_MAJOR} EQUAL 3)
	message(WARNING "Invalid Python executable version: ${PYTHON_VERSION_STRING}")
	message(WARNING "Define a valid Python 3 executable in variable PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE} or ${target} test won't be compiled")
	message(WARNING "Use: \"sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10\" to modify default python executable")
	return()
endif()

add_test(NAME ${target}
	COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_SCRIPT_LAUNCHER_NAME}
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
)
