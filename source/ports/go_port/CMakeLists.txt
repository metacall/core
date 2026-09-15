# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_GO)
	return()
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GoLang.cmake)

execute_process(
    COMMAND clang -print-resource-dir
    OUTPUT_VARIABLE CLANG_RESOURCE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CGO_EXTRA_CFLAGS "-I${CLANG_RESOURCE_DIR}/include")

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

include(Portability)

add_custom_target(${target}
	COMMAND ${CMAKE_COMMAND}
		-DGOPATH="${GOPATH}"
		-DGO_FLAGS="${GO_FLAGS}"
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
		-DGO_FLAGS="${GO_FLAGS}"
		-DCMAKE_Go_COMPILER="${CMAKE_Go_COMPILER}"
		-DCGO_CFLAGS="-I${CMAKE_SOURCE_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/include -Wall"
		-DCGO_LDFLAGS="-L${PROJECT_OUTPUT_DIR} -l$<TARGET_PROPERTY:${META_PROJECT_NAME}::metacall,NAME>$<$<CONFIG:DEBUG>:d>"
		-DLIBRARY_PATH_NAME="${PROJECT_LIBRARY_PATH_NAME}"
		-DLIBRARY_PATH="${PROJECT_OUTPUT_DIR}"
		-DGO_TASK="test"
		-DSANITIZER_COMPILE_DEFINITIONS=${SANITIZER_COMPILE_DEFINITIONS}
		-P ${CMAKE_CURRENT_SOURCE_DIR}/source/go_port.cmake
)

# Define pprof GO test
set(GO_PPROF_FILE "${CMAKE_CURRENT_BINARY_DIR}/Goroutine_leaks_report.pprof")
set(MAX_GOROUTINES 20)
set(PPROF_ENV_VAR "PPROFDIR=${CMAKE_CURRENT_BINARY_DIR}")

add_test(NAME GO_pprof
    COMMAND sh -c 
        "if [ ! -f \"${GO_PPROF_FILE}\" ]; then echo \"Error: File ${GO_PPROF_FILE} missing\"; exit 1; fi; COUNT=\$(\"${CMAKE_Go_COMPILER}\" tool pprof -text \"${GO_PPROF_FILE}\" 2>/dev/null | awk '/^Showing nodes accounting for/ {print \$5}' | tr -d ','); COUNT=\${COUNT:-0}; echo \"Goroutine count: \$COUNT (Limit: ${MAX_GOROUTINES})\"; [ \"\$COUNT\" -le ${MAX_GOROUTINES} ]"
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
	${PPROF_ENV_VAR}
	"C_INCLUDE_PATH=/usr/include:${CLANG_RESOURCE_DIR}/include"
)
