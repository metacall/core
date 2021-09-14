# Check if python loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY)
	return()
endif()

#
# External dependencies
#

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# Detect if Django is available
execute_process(
	COMMAND ${Python3_EXECUTABLE} -c "import django"
	RESULT_VARIABLE PYTHON_DJANGO_FOUND
	ERROR_QUIET
	OUTPUT_QUIET
)

if("${PYTHON_DJANGO_FOUND}" EQUAL "1")
	message(WARNING "Python Django not found, skipping the python django integration test")
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-python-django-integration-test)
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
	${source_path}/metacall_python_django_integration_test.cpp
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

add_dependencies(${target}
	${META_PROJECT_NAME}::metacall
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
)

#
# Libraries
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LIBRARIES}

	GTest

	${CMAKE_DL_LIBS}

	${META_PROJECT_NAME}::metacall
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
	set(DETOUR_LIBRARY_PATH "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
	set(SERIAL_LIBRARY_PATH "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
	set(LOADER_LIBRARY_PATH "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
	set(TEST_SCRIPT_LAUNCHER_APP "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${TEST_SCRIPT_LAUNCHER_BASE_NAME}.exe")
else()
	set(DETOUR_LIBRARY_PATH "${PROJECT_BINARY_DIR}")
	set(SERIAL_LIBRARY_PATH "${PROJECT_BINARY_DIR}")
	set(LOADER_LIBRARY_PATH "${PROJECT_BINARY_DIR}")
	set(TEST_SCRIPT_LAUNCHER_APP "${PROJECT_BINARY_DIR}/${TEST_SCRIPT_LAUNCHER_BASE_NAME}")
endif()

configure_file(${TEST_SCRIPT_LAUNCHER_NAME_IN} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_SCRIPT_LAUNCHER_NAME})

configure_file(${TEST_SCRIPT_LAUNCHER_CLIENT_NAME_IN} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_SCRIPT_LAUNCHER_CLIENT_NAME})

configure_file(${TEST_SCRIPT_LAUNCHER_FILE_NAME_IN} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_SCRIPT_LAUNCHER_FILE_NAME} COPYONLY)

add_test(NAME ${target}
	COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_SCRIPT_LAUNCHER_NAME}
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
