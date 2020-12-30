#
# Executable name and options
#

# Target name
set(target metacall-load-configuration-test)
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
	${source_path}/metacall_load_configuration_test.cpp
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

	${META_PROJECT_NAME}::metacall_distributable
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

include(GeneratorExpressions)

cmake_expand_generator_expressions(DEPENDENCIES
	$<$<BOOL:${OPTION_BUILD_LOADERS_NODE}>:node_loader>
	$<$<BOOL:${OPTION_BUILD_LOADERS_PY}>:py_loader>
	$<$<BOOL:${OPTION_BUILD_LOADERS_RB}>:rb_loader>
)

add_dependencies(${target} ${DEPENDENCIES})

#
# Configure test data
#

if(MSVC)
	configure_file(data/metacall_load_from_configuration_py_test_a.json.in ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/metacall_load_from_configuration_py_test_a.json)
	configure_file(data/metacall_load_from_configuration_py_test_b.json.in ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/metacall_load_from_configuration_py_test_b.json)
	configure_file(data/metacall_load_from_configuration_rb_test.json.in ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/metacall_load_from_configuration_rb_test.json)
	configure_file(data/metacall_load_from_configuration_node_test.json.in ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/metacall_load_from_configuration_node_test.json)
endif()

configure_file(data/metacall_load_from_configuration_py_test_a.json.in ${CMAKE_CURRENT_BINARY_DIR}/metacall_load_from_configuration_py_test_a.json)
configure_file(data/metacall_load_from_configuration_py_test_b.json.in ${CMAKE_CURRENT_BINARY_DIR}/metacall_load_from_configuration_py_test_b.json)
configure_file(data/metacall_load_from_configuration_rb_test.json.in ${CMAKE_CURRENT_BINARY_DIR}/metacall_load_from_configuration_rb_test.json)
configure_file(data/metacall_load_from_configuration_node_test.json.in ${CMAKE_CURRENT_BINARY_DIR}/metacall_load_from_configuration_node_test.json)


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
