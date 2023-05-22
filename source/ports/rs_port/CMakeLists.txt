# Check if this port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_RS)
	return()
endif()

#
# Port name and options
#

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/rust-toolchain TARGET_TOOLCHAIN)
string(STRIP "${TARGET_TOOLCHAIN}" TARGET_TOOLCHAIN)

find_package(Rust COMPONENTS ${TARGET_TOOLCHAIN})

if(NOT Rust_FOUND)
	message(STATUS "Rust not found")
	return()
endif()

# Target name
set(target rs_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

# Find bindgen
find_program(Rust_BINDGEN_EXECUTABLE bindgen
	HINTS ${Rust_CARGO_HOME}
	PATH_SUFFIXES "bin"
)

if(NOT Rust_BINDGEN_EXECUTABLE AND NOT OPTION_BUILD_GUIX)
	execute_process(COMMAND ${Rust_CARGO_EXECUTABLE} install bindgen)

	find_program(Rust_BINDGEN_EXECUTABLE bindgen
		HINTS ${Rust_CARGO_HOME}
		PATH_SUFFIXES "bin"
	)
endif()

# Generate bindings
if(Rust_BINDGEN_EXECUTABLE)
	add_custom_target(${target}_bindings
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND ${Rust_BINDGEN_EXECUTABLE}
			--no-include-path-detection
			"${CMAKE_SOURCE_DIR}/source/metacall/include/metacall/metacall.h"
			-o "${CMAKE_CURRENT_SOURCE_DIR}/src/bindings.rs"
			--
			-I${CMAKE_SOURCE_DIR}/source/metacall/include
			-I${CMAKE_BINARY_DIR}/source/metacall/include
			-I${CMAKE_BINARY_DIR}/source/include
		DEPENDS ${META_PROJECT_NAME}::metacall
	)
endif()

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/bindings.rs" PROPERTIES GENERATED TRUE)

# Build Rust Port
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(TARGET_BUILD_TYPE "--release")
else()
	set(TARGET_BUILD_TYPE)
endif()

if(OPTION_BUILD_SANITIZER)
	set(SANITIZER_FLAGS
		CMAKE_ADDRESS_SANITIZER=1
	)
elseif(OPTION_BUILD_THREAD_SANITIZER)
	set(SANITIZER_FLAGS
		CMAKE_THREAD_SANITIZER=1
	)
else()
	set(SANITIZER_FLAGS)
endif()


add_custom_target(${target} ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} -E env CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} PROJECT_OUTPUT_DIR=${PROJECT_OUTPUT_DIR} ${SANITIZER_FLAGS}
		${Rust_CARGO_EXECUTABLE} build ${TARGET_BUILD_TYPE}
	DEPENDS ${META_PROJECT_NAME}::metacall
)

#
# Dependecies
#

if(Rust_BINDGEN_EXECUTABLE)
	add_dependencies(${target}
		${target}_bindings
	)
endif()

#
# Define test
#

if(OPTION_BUILD_THREAD_SANITIZER)
	# Disable unit tests because bindgen layout tests fail when building with thread sanitizers
	set(RUN_INTEGRATION_TESTS_ONLY "--test '*'")
endif()

add_test(NAME ${target}
	COMMAND ${Rust_CARGO_EXECUTABLE} test ${RUN_INTEGRATION_TESTS_ONLY}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

include(TestEnvironmentVariables)
include(Portability)

project_library_path(TEST_LIB_PATH
	${PROJECT_OUTPUT_DIR}
)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
	"${PROJECT_LIBRARY_PATH_NAME}=${TEST_LIB_PATH}"
	"CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
	"PROJECT_OUTPUT_DIR=${PROJECT_OUTPUT_DIR}"
	"RUST_BACKTRACE=1"
	${SANITIZER_FLAGS}
)
