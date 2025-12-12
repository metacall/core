# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_GO)
	return()
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GoLang.cmake)

#
# Port name and options
#

# Target name
set(target go_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

set(sources
	source/await.go
	source/pointer.go
	source/go_port.go
)

include(Portability)

add_custom_target(${target}
	COMMAND ${CMAKE_COMMAND}
		-DGOPATH="${GOPATH}"
		-DCMAKE_Go_COMPILER="${CMAKE_Go_COMPILER}"
		-DCGO_CFLAGS="-I${CMAKE_SOURCE_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/include -Wall"
		-DCGO_LDFLAGS="-L${PROJECT_OUTPUT_DIR} -l$<TARGET_PROPERTY:${META_PROJECT_NAME}::metacall,NAME>$<$<CONFIG:DEBUG>:d>"
		-DLIBRARY_PATH_NAME="${PROJECT_LIBRARY_PATH_NAME}"
		-DLIBRARY_PATH="${PROJECT_OUTPUT_DIR}"
		-DGO_TASK="build"
		-DSANITIZER_COMPILE_DEFINITIONS=${SANITIZER_COMPILE_DEFINITIONS}
		-P ${CMAKE_CURRENT_SOURCE_DIR}/source/go_port.cmake
	SOURCES ${sources}
	DEPENDS ${sources}
)

#
# Define test
#

if(OPTION_BUILD_ADDRESS_SANITIZER OR OPTION_BUILD_THREAD_SANITIZER OR OPTION_BUILD_MEMORY_SANITIZER)
	# TODO: Not implemented (https://go.googlesource.com/go/+/go1.7/misc/cgo/testsanitizers/test.bash)
	return()
endif()

add_test(NAME ${target}
	COMMAND ${CMAKE_COMMAND}
		-DGOPATH="${GOPATH}"
		-DCMAKE_Go_COMPILER="${CMAKE_Go_COMPILER}"
		-DCGO_CFLAGS="-I${CMAKE_SOURCE_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/include -Wall"
		-DCGO_LDFLAGS="$<$<NOT:$<BOOL:${APPLE}>>:-L${PROJECT_OUTPUT_DIR} -l$<TARGET_PROPERTY:${META_PROJECT_NAME}::metacall,NAME>$<$<CONFIG:DEBUG>:d>> $<$<AND:$<BOOL:${APPLE}>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-Wl,-undefined,dynamic_lookup>"
		-DLIBRARY_PATH_NAME="${PROJECT_LIBRARY_PATH_NAME}"
		-DLIBRARY_PATH="${PROJECT_OUTPUT_DIR}"
		-DGO_TASK="test"
		-DSANITIZER_COMPILE_DEFINITIONS=${SANITIZER_COMPILE_DEFINITIONS}
		-P ${CMAKE_CURRENT_SOURCE_DIR}/source/go_port.cmake
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)
