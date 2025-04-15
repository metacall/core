#
# Executable name and options
#

# Target name
set(target configuration-test)
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
	${source_path}/configuration_test.cpp
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

	$<TARGET_PROPERTY:${META_PROJECT_NAME}::version,INCLUDE_DIRECTORIES>
	$<TARGET_PROPERTY:${META_PROJECT_NAME}::preprocessor,INCLUDE_DIRECTORIES>
	$<TARGET_PROPERTY:${META_PROJECT_NAME}::environment,INCLUDE_DIRECTORIES>
	$<TARGET_PROPERTY:${META_PROJECT_NAME}::format,INCLUDE_DIRECTORIES>
	$<TARGET_PROPERTY:${META_PROJECT_NAME}::threading,INCLUDE_DIRECTORIES>
	$<TARGET_PROPERTY:${META_PROJECT_NAME}::log,INCLUDE_DIRECTORIES>
	$<TARGET_PROPERTY:${META_PROJECT_NAME}::memory,INCLUDE_DIRECTORIES>
	$<TARGET_PROPERTY:${META_PROJECT_NAME}::portability,INCLUDE_DIRECTORIES>
	$<TARGET_PROPERTY:${META_PROJECT_NAME}::adt,INCLUDE_DIRECTORIES>
	$<TARGET_PROPERTY:${META_PROJECT_NAME}::reflect,INCLUDE_DIRECTORIES>
	$<TARGET_PROPERTY:${META_PROJECT_NAME}::dynlink,INCLUDE_DIRECTORIES>
	$<TARGET_PROPERTY:${META_PROJECT_NAME}::serial,INCLUDE_DIRECTORIES>
	$<TARGET_PROPERTY:${META_PROJECT_NAME}::configuration,INCLUDE_DIRECTORIES>
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
# Compile features
#

target_compile_features(${target}
	PRIVATE
	cxx_std_17
)

#
# Linker options
#

target_link_options(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

#
# Define test
#

add_test(NAME ${target}
	COMMAND $<TARGET_FILE:${target}>
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

#
# Configure test data
#

configure_file(data/global.json.in ${CMAKE_CURRENT_BINARY_DIR}/global.json)
configure_file(data/child_a.json.in ${CMAKE_CURRENT_BINARY_DIR}/child_a.json)
configure_file(data/child_b.json.in ${CMAKE_CURRENT_BINARY_DIR}/child_b.json)
configure_file(data/child_c.json.in ${CMAKE_CURRENT_BINARY_DIR}/child_c.json)
configure_file(data/child_d.json.in ${CMAKE_CURRENT_BINARY_DIR}/child_d.json)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	"SERIAL_LIBRARY_PATH=${SERIAL_LIBRARY_PATH}"
	"CONFIGURATION_PATH=${CMAKE_CURRENT_BINARY_DIR}/global.json"
)
