# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_LOADERS_CS OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_PY)
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-python-open-test)
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
	${source_path}/metacall_python_open_test.cpp
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
# Dependencies
#

if(NOT OPTION_BUILD_GUIX)
	find_package(NPM)

	if(NOT NPM_FOUND)
		message(STATUS "NPM not found")
		return()
	endif()

	if(MSVC)
		set(DEPENDS_OUTPUT_PATH "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
	else()
		set(DEPENDS_OUTPUT_PATH "${PROJECT_BINARY_DIR}")
	endif()

	add_custom_target(${target}-depends
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${NPM_EXECUTABLE} --prefix ${CMAKE_CURRENT_BINARY_DIR} install jsonwebtoken
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/node_modules ${DEPENDS_OUTPUT_PATH}/node_modules
	)

	add_dependencies(${target} ${target}-depends)
endif()

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
