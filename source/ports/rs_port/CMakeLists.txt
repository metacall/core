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
		execute_process(COMMAND ${Rust_CARGO_EXECUTABLE} install bindgen-cli)

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
			--allowlist-function 'metacall.*'
			--rustified-enum 'metacall_.*'
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
		set(SANITIZER_FLAGS "RUSTFLAGS=-Zsanitizer=address")
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
		set(SANITIZER_FLAGS "RUSTFLAGS=-Zsanitizer=thread")
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

# If we have cargo-valgrind, run the tests with it
# For installing it, first install valgrind on your system, then run:
# cargo install cargo-valgrind
# It will install the tooling necesary for running with valgrind.
# For debugging the project run:
#
# cmake -DCMAKE_BUILD_TYPE=Debug -DOPTION_BUILD_PORTS=ON \
# -DOPTION_BUILD_PORTS_RS=ON -DOPTION_TEST_MEMORYCHECK ..
#
# You can also add loaders for this command so you can test with them.
# Finally, build and run the tests:
#
# make -j8 rs_port
# ctest -VV -R rs_port
#
# CTest will run the tests and show valgrind output if there is any error.
if(OPTION_TEST_MEMORYCHECK AND NOT (OPTION_BUILD_ADDRESS_SANITIZER OR OPTION_BUILD_THREAD_SANITIZER))
	# Check if cargo-valgrind is installed
	execute_process(
		COMMAND ${Rust_CARGO_EXECUTABLE} install --list
		OUTPUT_VARIABLE CARGO_VALGRIND_INSTALL
	)
	string(FIND "${CARGO_VALGRIND_INSTALL}" "cargo-valgrind" CARGO_VALGRIND_INSTALLED)
	if(NOT ${CARGO_VALGRIND_INSTALLED} EQUAL -1)
		set(CARGO_VALGRIND valgrind)
		set(CARGO_VALGRIND_FLAGS "VALGRINDFLAGS=${MEMORYCHECK_COMMAND_OPTIONS} --suppressions=${CMAKE_CURRENT_SOURCE_DIR}/valgrind-rust.supp")
	endif()
endif()

# For running one test only, you can add:
# --test=metacall_test --package=metacall
# To the command, and it will run metacall_test only
add_test(NAME ${target}
	COMMAND ${Rust_CARGO_EXECUTABLE} ${CARGO_VALGRIND} ${NIGHTLY_FLAGS} test ${BUILD_STD_FLAGS}
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

# Define environment variables
test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
	"${PROJECT_LIBRARY_PATH_NAME}=${TEST_LIB_PATH}"
	"CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
	"PROJECT_OUTPUT_DIR=${PROJECT_OUTPUT_DIR}"
	"RUST_BACKTRACE=1"
	"${SANITIZER_FLAGS}"
	"${CARGO_VALGRIND_FLAGS}"
)

# Create .env file for supporting vscode debugging
set(RS_PORT_DEBUG_ENVIRONMENT_VARIABLES
	"LOADER_LIBRARY_PATH=${PROJECT_OUTPUT_DIR}"
	"LOADER_SCRIPT_PATH=${PROJECT_OUTPUT_DIR}/scripts"
	"CONFIGURATION_PATH=${PROJECT_OUTPUT_DIR}/configurations/global.json"
	"SERIAL_LIBRARY_PATH=${PROJECT_OUTPUT_DIR}"
	"DETOUR_LIBRARY_PATH=${PROJECT_OUTPUT_DIR}"
	"${PROJECT_LIBRARY_PATH_NAME}=${TEST_LIB_PATH}"
)

set(RS_PORT_ENVIRONMENT_VARIABLES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/.vscode/.env")

file(WRITE ${RS_PORT_ENVIRONMENT_VARIABLES_FILE} "")

foreach(ENV_VAR IN LISTS RS_PORT_DEBUG_ENVIRONMENT_VARIABLES)
	file(APPEND ${RS_PORT_ENVIRONMENT_VARIABLES_FILE} "${ENV_VAR}\n")
endforeach()

# Create config.toml for supporting vscode debugging
set(RS_PORT_BUILD_ENVIRONMENT_VARIABLES
	"CMAKE_BUILD_TYPE = { value = \"${CMAKE_BUILD_TYPE}\", force = true }"
	"PROJECT_OUTPUT_DIR = { value = \"${PROJECT_OUTPUT_DIR}\", force = true }"
)

set(RS_PORT_CONFIG_ENV_FILE "${CMAKE_CURRENT_SOURCE_DIR}/.cargo/config.toml")

file(WRITE ${RS_PORT_CONFIG_ENV_FILE} "[env]\n")

foreach(ENV_VAR IN LISTS RS_PORT_BUILD_ENVIRONMENT_VARIABLES)
	file(APPEND ${RS_PORT_CONFIG_ENV_FILE} "${ENV_VAR}\n")
endforeach()
