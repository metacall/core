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
	message(SEND_ERROR "Rust not found")
	return()
endif()

# Target name
set(target rs_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

# Find bindgen (disable when using sanitizers)
if(NOT (OPTION_BUILD_ADDRESS_SANITIZER OR OPTION_BUILD_THREAD_SANITIZER OR OPTION_BUILD_MEMORY_SANITIZER))
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

# Fix version for using LLVM 16 with instrumentation
set(RS_PORT_INSTRUMENTATION_NIGHTLY_VERSION 2023-05-24)

if(OPTION_BUILD_ADDRESS_SANITIZER AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	if ("${Rust_TOOLCHAIN_TRIPLET}" STREQUAL "aarch64-apple-darwin" OR
		"${Rust_TOOLCHAIN_TRIPLET}" STREQUAL "aarch64-unknown-fuchsia" OR
		"${Rust_TOOLCHAIN_TRIPLET}" STREQUAL "aarch64-unknown-linux-gnu" OR
		"${Rust_TOOLCHAIN_TRIPLET}" STREQUAL "x86_64-apple-darwin" OR
		"${Rust_TOOLCHAIN_TRIPLET}" STREQUAL "x86_64-unknown-fuchsia" OR
		"${Rust_TOOLCHAIN_TRIPLET}" STREQUAL "x86_64-unknown-freebsd" OR
		"${Rust_TOOLCHAIN_TRIPLET}" STREQUAL "x86_64-unknown-linux-gnu")
		set(RS_PORT_RUN_INSTRUMENTED_TEST ON)
		set(RUSTUP_NIGTHLY_INSTALL_SRC ON)
		set(SANITIZER_FLAGS
			CMAKE_ADDRESS_SANITIZER=1
			RUSTFLAGS=-Zsanitizer=address
		)
		set(NIGHTLY_FLAGS
			+nightly-${RS_PORT_INSTRUMENTATION_NIGHTLY_VERSION}
		)
		set(BUILD_STD_FLAGS
			-Zbuild-std
			--target ${Rust_TOOLCHAIN_TRIPLET}
		)
	endif()
elseif(OPTION_BUILD_THREAD_SANITIZER AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	if ("${Rust_TOOLCHAIN_TRIPLET}" STREQUAL "aarch64-apple-darwin" OR
		"${Rust_TOOLCHAIN_TRIPLET}" STREQUAL "aarch64-unknown-linux-gnu" OR
		"${Rust_TOOLCHAIN_TRIPLET}" STREQUAL "x86_64-apple-darwin" OR
		"${Rust_TOOLCHAIN_TRIPLET}" STREQUAL "x86_64-unknown-freebsd" OR
		"${Rust_TOOLCHAIN_TRIPLET}" STREQUAL "x86_64-unknown-linux-gnu")
		set(RS_PORT_RUN_INSTRUMENTED_TEST ON)
		set(RUSTUP_NIGTHLY_INSTALL_SRC ON)
		set(SANITIZER_FLAGS
			CMAKE_THREAD_SANITIZER=1
			RUSTFLAGS=-Zsanitizer=thread
		)
		set(NIGHTLY_FLAGS
			+nightly-${RS_PORT_INSTRUMENTATION_NIGHTLY_VERSION}
		)
		set(BUILD_STD_FLAGS
			-Zbuild-std
			--target ${Rust_TOOLCHAIN_TRIPLET}
		)
	endif()
else()
	set(RS_PORT_RUN_INSTRUMENTED_TEST OFF)
	set(SANITIZER_FLAGS)
	set(BUILD_STD_FLAGS)
endif()

add_custom_target(${target} ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} -E env CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} PROJECT_OUTPUT_DIR=${PROJECT_OUTPUT_DIR} ${SANITIZER_FLAGS}
		${Rust_CARGO_EXECUTABLE} ${NIGHTLY_FLAGS} build ${BUILD_STD_FLAGS} ${TARGET_BUILD_TYPE}
	DEPENDS ${META_PROJECT_NAME}::metacall
)

if(RUSTUP_NIGTHLY_INSTALL_SRC)
	add_custom_target(${target}_rustup_nightly ALL
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND ${Rust_RUSTUP_EXECUTABLE} install --profile complete nightly-${RS_PORT_INSTRUMENTATION_NIGHTLY_VERSION}
	)
	add_dependencies(${target}
		${target}_rustup_nightly
	)
endif()

#
# Dependecies
#

# Find bindgen (disable bindgen dependency for Clang as a workaround for stdlib.h not found issue)
if(Rust_BINDGEN_EXECUTABLE AND NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	add_dependencies(${target}
		${target}_bindings
	)
endif()

#
# Define test
#

if((OPTION_BUILD_ADDRESS_SANITIZER OR OPTION_BUILD_THREAD_SANITIZER) AND NOT RS_PORT_RUN_INSTRUMENTED_TEST)
	return()
endif()

add_test(NAME ${target}
	COMMAND ${Rust_CARGO_EXECUTABLE} ${NIGHTLY_FLAGS} test ${BUILD_STD_FLAGS}
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
