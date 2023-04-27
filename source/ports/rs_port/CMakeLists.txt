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
endif()

# Build Rust Port
add_custom_target(${target} ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} -E env CMAKE_BINDGEN=1 ${SANITIZER_FLAGS} ${Rust_CARGO_EXECUTABLE} build ${TARGET_BUILD_TYPE}
)

# Copy the include files
set(include_dirs
	"${CMAKE_SOURCE_DIR}/source/metacall/include"
	"${CMAKE_BINARY_DIR}/source/metacall/include"
	"${CMAKE_BINARY_DIR}/source/include"
)

add_custom_target(${target}_includes
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${include_dirs} "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_dependencies(${target} ${target}_includes)

# Copy bindings
add_custom_command(
	TARGET ${target}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/target/bindings/bindings.rs" "${CMAKE_CURRENT_SOURCE_DIR}/src/bindings.rs"
)

#
# Dependecies
#

add_dependencies(${target}
	${META_PROJECT_NAME}::metacall
)

#
# Define test
#

add_test(NAME ${target}
	COMMAND ${Rust_CARGO_EXECUTABLE} test
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
