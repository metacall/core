# Check if python loader is enabled
if(NOT OPTION_BUILD_LOADERS_PY)
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-configuration-exec-path-test)
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
	${source_path}/metacall_configuration_exec_path_test.cpp
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

add_test(NAME ${target}
	COMMAND $<TARGET_FILE:${target}>
)

#
# Define dependencies
#

add_dependencies(${target}
	py_loader
)

#
# Set test variables
#

set(PY_LOADER_SCRIPT_PATH "${CMAKE_CURRENT_BINARY_DIR}/scripts")
set(PY_CONFIGURATION_PATH "${CMAKE_CURRENT_BINARY_DIR}/configurations")
set(PY_EXECUTION_PATH "${PY_LOADER_SCRIPT_PATH}/a/b/c/d/e")

#
# Define test properties
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	"LOADER_LIBRARY_PATH=${LOADER_LIBRARY_PATH}"
	"LOADER_SCRIPT_PATH=${PY_LOADER_SCRIPT_PATH}"
	"CONFIGURATION_PATH=${PY_CONFIGURATION_PATH}/global.json"
	"SERIAL_LIBRARY_PATH=${SERIAL_LIBRARY_PATH}"
	"DETOUR_LIBRARY_PATH=${DETOUR_LIBRARY_PATH}"
)

#
# Configure test data
#

configure_file(data/configurations/global.json.in ${PY_CONFIGURATION_PATH}/global.json @ONLY)

configure_file(data/configurations/py_loader.json.in ${PY_CONFIGURATION_PATH}/py_loader.json @ONLY)

file(COPY data/scripts/main.py DESTINATION ${PY_LOADER_SCRIPT_PATH})

file(COPY data/scripts/metacall_configuration_exec_path_test.py DESTINATION ${PY_EXECUTION_PATH})
