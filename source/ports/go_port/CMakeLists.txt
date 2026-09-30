# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_GO)
	return()
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GoLang.cmake)

if(CMAKE_HOST_WIN32)
	set(PATH_SEP ";")
else()
	set(PATH_SEP ":")
endif()

# Check C compilers
# cgo supports only GCC, Clang, AppleClang
if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
	execute_process(
		COMMAND ${CMAKE_C_COMPILER} -print-resource-dir
		OUTPUT_VARIABLE C_RESOURCE_DIR
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	if(CMAKE_HOST_LINUX OR CMAKE_HOST_UNIX)
		set(C_INCLUDE_PATH_VAL "C_INCLUDE_PATH=/usr/include${PATH_SEP}${C_RESOURCE_DIR}/include")
	elseif(CMAKE_HOST_WIN32)
		set(C_INCLUDE_PATH_VAL "C_INCLUDE_PATH=${C_RESOURCE_DIR}/include")
	elseif(CMAKE_HOST_APPLE)
		set(C_INCLUDE_PATH_VAL "C_INCLUDE_PATH=${CMAKE_OSX_SYSROOT}/usr/include${PATH_SEP}${C_RESOURCE_DIR}/include")
	endif()
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	execute_process(
		COMMAND ${CMAKE_C_COMPILER} -print-file-name=include
		OUTPUT_VARIABLE C_RESOURCE_DIR
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	if(CMAKE_HOST_LINUX OR CMAKE_HOST_UNIX)
		set(C_INCLUDE_PATH_VAL "C_INCLUDE_PATH=/usr/include${PATH_SEP}${C_RESOURCE_DIR}")
	elseif(CMAKE_HOST_WIN32)
		set(C_INCLUDE_PATH_VAL "C_INCLUDE_PATH=${C_RESOURCE_DIR}")
	endif()
else()
	message(WARNING "Unsupported C compiler family: ${CMAKE_C_COMPILER_ID}. cgo supports only GCC, Clang, AppleClang.")
	set(C_INCLUDE_PATH_VAL "")
endif()

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

# Flags
set(GO_FLAGS "-race")

# set to debug so that the pprof server starts and records the data
set(GO_MODE "MODE=debug")


include(Portability)

add_custom_target(${target}
	COMMAND ${CMAKE_COMMAND}
		-DGOPATH="${GOPATH}"
		-DGO_FLAGS="${GO_FLAGS}"
		-DCC="${CMAKE_C_COMPILER}"
		-DCMAKE_Go_COMPILER="${CMAKE_Go_COMPILER}"
		-DCGO_CFLAGS="-I${CMAKE_SOURCE_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/include ${CGO_EXTRA_CFLAGS} -Wall"
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
		-DGO_FLAGS="${GO_FLAGS}"
		-DCC="${CMAKE_C_COMPILER}"
		-DCMAKE_Go_COMPILER="${CMAKE_Go_COMPILER}"
		-DCGO_CFLAGS="-I${CMAKE_SOURCE_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/include ${CGO_EXTRA_CFLAGS} -Wall"
		-DCGO_LDFLAGS="-L${PROJECT_OUTPUT_DIR} -l$<TARGET_PROPERTY:${META_PROJECT_NAME}::metacall,NAME>$<$<CONFIG:DEBUG>:d>"
		-DLIBRARY_PATH_NAME="${PROJECT_LIBRARY_PATH_NAME}"
		-DLIBRARY_PATH="${PROJECT_OUTPUT_DIR}"
		-DGO_TASK="test"
		-DSANITIZER_COMPILE_DEFINITIONS=${SANITIZER_COMPILE_DEFINITIONS}
		-P ${CMAKE_CURRENT_SOURCE_DIR}/source/go_port.cmake
)

# Define pprof GO test
set(GO_PPROF_GOROUTINE "${CMAKE_CURRENT_BINARY_DIR}/goroutine.pprof")
set(MAX_GOROUTINES 20)
set(PPROF_ENV_VAR "PPROFDIR=${CMAKE_CURRENT_BINARY_DIR}")

add_test(NAME GO_pprof
    COMMAND ${CMAKE_COMMAND}
	-DGOPATH="${GOPATH}"
	-DCMAKE_Go_COMPILER="${CMAKE_Go_COMPILER}"
	-DGO_PPROF_TASK="tool"
	-DGO_PPROF_GOROUTINE="${GO_PPROF_GOROUTINE}"
	-P ${CMAKE_CURRENT_SOURCE_DIR}/source/go_pprof.cmake
)

#
# Define go port test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

set_tests_properties(${target}
	PROPERTIES
	FAIL_REGULAR_EXPRESSION "WARNING: DATA RACE"
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
	${PPROF_ENV_VAR}
	${GO_MODE}
	${C_INCLUDE_PATH_VAL}
)

#
# Define pprof test labels
#

# Fail on any number of leaks above 20
set_tests_properties(GO_pprof
	PROPERTIES
	FAIL_REGULAR_EXPRESSION "[2-9][0-9]+ total"
)

test_environment_variables(GO_pprof
	""
	${PPROF_ENV_VAR}
	${GO_MODE}
)