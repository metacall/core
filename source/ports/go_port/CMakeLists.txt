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
	source/registry.go
	source/class.go
	source/object.go
	source/future.go
	source/symbolizer.go
	source/go_port.go
)

include(Portability)

#
# Sanitizers
#

# Sanitizers are only enabled in Debug or RelWithDebInfo (as in CompileOptions)
set(GO_PORT_FLAGS)
set(GO_PORT_CC)

# Platforms (apart from Linux) where the race detector of Go is supported: darwin/amd64, darwin/arm64, freebsd/amd64 and windows/amd64,
# Go does not support -asan nor -msan in them, and -race is incompatible with the sanitizer runtimes, so with sanitizers only the cgo code is instrumented
if((PROJECT_OS_FAMILY MATCHES "macos" AND (PROJECT_ARCH_AMD64 OR PROJECT_ARCH_AARCH64))
	OR (PROJECT_OS_BSD AND PROJECT_ARCH_AMD64)
	OR (PROJECT_OS_WIN AND PROJECT_ARCH_AMD64))
	if(CMAKE_BUILD_TYPE STREQUAL "Debug"
		AND NOT OPTION_BUILD_ADDRESS_SANITIZER
		AND NOT OPTION_BUILD_THREAD_SANITIZER
		AND NOT OPTION_BUILD_MEMORY_SANITIZER
		AND NOT OPTION_TEST_MEMORYCHECK)
		set(GO_PORT_FLAGS -race)
	endif()
elseif(PROJECT_OS_LINUX)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		if(OPTION_BUILD_ADDRESS_SANITIZER)
			set(GO_PORT_FLAGS -asan)
		elseif(OPTION_BUILD_THREAD_SANITIZER)
			# Go code is not instrumented because -race embeds its own runtime which is incompatible
			# with libtsan, but cgo synchronizes the calls between Go and C when C is instrumented
		elseif(OPTION_BUILD_MEMORY_SANITIZER AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
			set(GO_PORT_FLAGS -msan)
		elseif(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT OPTION_TEST_MEMORYCHECK)
			# Detect data races of the Go code when no sanitizer or memory check (valgrind) is enabled
			set(GO_PORT_FLAGS -race)
		endif()
	endif()

	# Symbolizer of cgo backtraces, only in builds with debug information (Debug or RelWithDebInfo) without address, thread or memory sanitizer or memcheck (see source/symbolizer.go)
	if((CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo") AND NOT OPTION_BUILD_ADDRESS_SANITIZER AND NOT OPTION_BUILD_THREAD_SANITIZER AND NOT OPTION_TEST_MEMORYCHECK)
		list(APPEND GO_PORT_FLAGS -tags=symbolizer)
	endif()
endif()

# Use the same sanitizer flags as metacall, they are added to the #cgo directives (see source/go_port.cmake)
set(GO_PORT_SANITIZER_CFLAGS ${SANITIZER_COMPILE_OPTIONS})
set(GO_PORT_SANITIZER_LDFLAGS ${SANITIZER_LINK_OPTIONS})

# Clang only links the C++ runtime of UBSan (vptr checks of the C++ libraries like simd_json_serial)
# when linking with clang++, but cgo links with clang, so it must be linked explicitly with the C++ standard library
if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" AND "-fsanitize=undefined" IN_LIST SANITIZER_LINK_OPTIONS)
	list(APPEND GO_PORT_SANITIZER_LDFLAGS -fsanitize-link-c++-runtime -lstdc++)
endif()

list(JOIN GO_PORT_SANITIZER_CFLAGS " " GO_PORT_SANITIZER_CFLAGS)
list(JOIN GO_PORT_SANITIZER_LDFLAGS " " GO_PORT_SANITIZER_LDFLAGS)

# Find the compiler (name-<major> or name) with the same major version as the metacall C compiler
macro(go_port_find_cc name)
	string(TOUPPER "${name}" GO_PORT_CC_NAME)
	string(REGEX MATCH "^[0-9]+" GO_PORT_CC_MAJOR "${CMAKE_C_COMPILER_VERSION}")
	get_filename_component(GO_PORT_CC_DIR "${CMAKE_C_COMPILER}" DIRECTORY)
	find_program(GO_PORT_${GO_PORT_CC_NAME} NAMES ${name}-${GO_PORT_CC_MAJOR} ${name} HINTS "${GO_PORT_CC_DIR}")
	set(GO_PORT_CC "${GO_PORT_${GO_PORT_CC_NAME}}")
endmacro()

if(GO_PORT_SANITIZER_CFLAGS)
	# Go must link with the same compiler used for building metacall, otherwise the sanitizer runtimes differ
	if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
		# Go only detects GCC if 'CC --version' starts with gcc, so generic names like cc do not work
		go_port_find_cc(gcc)
	elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
		# Go detects Clang from 'clang version' in 'CC --version' output, use the clang with the same version as metacall
		go_port_find_cc(clang)
	else()
		set(GO_PORT_CC "${CMAKE_C_COMPILER}")
	endif()
endif()

# Go flags passed to the build and test tasks
list(JOIN GO_PORT_FLAGS " " GO_PORT_BUILD_FLAGS)

# Pass memory check and sanitizer definitions to cgo
set(GO_PORT_COMPILE_DEFINITIONS
	${MEMORYCHECK_COMPILE_DEFINITIONS}
	${SANITIZER_COMPILE_DEFINITIONS}
)
list(JOIN GO_PORT_COMPILE_DEFINITIONS " " GO_PORT_COMPILE_DEFINITIONS)

set(GO_PORT_CGO_CFLAGS "-I${CMAKE_SOURCE_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/metacall/include -I${CMAKE_BINARY_DIR}/source/include -Wall")
set(GO_PORT_CGO_LDFLAGS "-L${PROJECT_OUTPUT_DIR} -l$<TARGET_PROPERTY:${META_PROJECT_NAME}::metacall,NAME>$<$<CONFIG:DEBUG>:d>")

add_custom_target(${target}
	COMMAND ${CMAKE_COMMAND}
		-DGOPATH="${GOPATH}"
		-DCMAKE_Go_COMPILER="${CMAKE_Go_COMPILER}"
		-DCGO_CFLAGS="${GO_PORT_CGO_CFLAGS}"
		-DCGO_LDFLAGS="${GO_PORT_CGO_LDFLAGS}"
		-DLIBRARY_PATH_NAME="${PROJECT_LIBRARY_PATH_NAME}"
		-DLIBRARY_PATH="${PROJECT_OUTPUT_DIR}"
		-DGO_TASK="build"
		-DGO_FLAGS="${GO_PORT_BUILD_FLAGS}"
		-DGO_CC="${GO_PORT_CC}"
		-DGO_SANITIZER_CFLAGS="${GO_PORT_SANITIZER_CFLAGS}"
		-DGO_SANITIZER_LDFLAGS="${GO_PORT_SANITIZER_LDFLAGS}"
		-DCOMPILE_DEFINITIONS="${GO_PORT_COMPILE_DEFINITIONS}"
		-P ${CMAKE_CURRENT_SOURCE_DIR}/source/go_port.cmake
	SOURCES ${sources}
	DEPENDS ${sources}
)

# Enable tests and dependencies depending on the available loaders
set(GO_PORT_LOADERS_ENV)

foreach(LOADER C PY NODE)
	string(TOLOWER "${LOADER}" LOADER_NAME)
	if(OPTION_BUILD_LOADERS AND OPTION_BUILD_LOADERS_${LOADER})
		list(APPEND GO_PORT_LOADERS_ENV "OPTION_BUILD_LOADERS_${LOADER}=ON")
		add_dependencies(${target} ${LOADER_NAME}_loader)
	else()
		list(APPEND GO_PORT_LOADERS_ENV "OPTION_BUILD_LOADERS_${LOADER}=OFF")
	endif()
endforeach()

#
# Define test
#

add_test(NAME ${target}
	COMMAND ${CMAKE_COMMAND}
		-DGOPATH="${GOPATH}"
		-DCMAKE_Go_COMPILER="${CMAKE_Go_COMPILER}"
		-DCGO_CFLAGS="${GO_PORT_CGO_CFLAGS}"
		-DCGO_LDFLAGS="${GO_PORT_CGO_LDFLAGS}"
		-DLIBRARY_PATH_NAME="${PROJECT_LIBRARY_PATH_NAME}"
		-DLIBRARY_PATH="${PROJECT_OUTPUT_DIR}"
		-DGO_TASK="test"
		-DGO_FLAGS="${GO_PORT_BUILD_FLAGS}"
		-DGO_CC="${GO_PORT_CC}"
		-DGO_SANITIZER_CFLAGS="${GO_PORT_SANITIZER_CFLAGS}"
		-DGO_SANITIZER_LDFLAGS="${GO_PORT_SANITIZER_LDFLAGS}"
		-DCOMPILE_DEFINITIONS="${GO_PORT_COMPILE_DEFINITIONS}"
		-P ${CMAKE_CURRENT_SOURCE_DIR}/source/go_port.cmake
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

# All the Go tests run in the same source directory, they patch the cgo flags of the sources
# and create files in it, so they must not run in parallel
set_tests_properties(${target}
	PROPERTIES
	FAIL_REGULAR_EXPRESSION "WARNING: DATA RACE"
	RESOURCE_LOCK ${target}_sources
)

include(TestEnvironmentVariables)

set(GO_PORT_TEST_ENVIRONMENT_VARIABLES
	${TESTS_ENVIRONMENT_VARIABLES}
	"METACALL_INCLUDE_DIR=${CMAKE_SOURCE_DIR}/source/metacall/include"
	"METACALL_API_INCLUDE_DIR=${CMAKE_BINARY_DIR}/source/metacall/include"
	"METACALL_BUILD_INCLUDE_DIR=${CMAKE_BINARY_DIR}/source/include"
	"METACALL_LIBRARY=$<TARGET_FILE:${META_PROJECT_NAME}::metacall>"
	"PROJECT_OUTPUT_DIR=${PROJECT_OUTPUT_DIR}"
	${GO_PORT_LOADERS_ENV}
)

# Profiles generated by the tests (with race detector) are stored in the binary directory instead of the sources
set(GO_PORT_TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/test")
file(MAKE_DIRECTORY "${GO_PORT_TEST_DIR}")

test_environment_variables(${target}
	""
	${GO_PORT_TEST_ENVIRONMENT_VARIABLES}
	"PPROFDIR=${GO_PORT_TEST_DIR}"
)

#
# Define profiling test
#

# Profiles are only generated in release builds (Release or RelWithDebInfo) without sanitizers or memory check (valgrind),
# because the instrumentation distorts the results and it conflicts with the profiler (i.e ThreadSanitizer intercepts
# the SIGPROF handler and delivers the signal with a stale context, which crashes the Go runtime while unwinding the stack)
if((CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	AND NOT OPTION_BUILD_ADDRESS_SANITIZER
	AND NOT OPTION_BUILD_THREAD_SANITIZER
	AND NOT OPTION_BUILD_MEMORY_SANITIZER
	AND NOT OPTION_TEST_MEMORYCHECK)
	set(GO_PORT_PPROF_DIR "${CMAKE_CURRENT_BINARY_DIR}/pprof")
	file(MAKE_DIRECTORY "${GO_PORT_PPROF_DIR}")

	set(GO_PORT_PPROF_FLAGS ${GO_PORT_FLAGS})
	list(APPEND GO_PORT_PPROF_FLAGS
		-outputdir ${GO_PORT_PPROF_DIR}
		-o ${GO_PORT_PPROF_DIR}/${target}.test
		-cpuprofile cpu.pprof
		-memprofile mem.pprof
		-blockprofile block.pprof
		-mutexprofile mutex.pprof
	)
	list(JOIN GO_PORT_PPROF_FLAGS " " GO_PORT_PPROF_FLAGS)

	# Profiles to be analyzed, the goroutine leaks profile is only generated when NodeJS loader is enabled
	set(GO_PORT_PPROF_PROFILES cpu mem block mutex goroutine)
	# TODO: TestGoRoutineLeaks is skipped on MacOS because it fails, so this profile is not generated there
	if(OPTION_BUILD_LOADERS AND OPTION_BUILD_LOADERS_NODE AND NOT PROJECT_OS_FAMILY MATCHES "macos")
	# TODO: Restore it once it works on MacOS
	# if(OPTION_BUILD_LOADERS AND OPTION_BUILD_LOADERS_NODE)
		list(APPEND GO_PORT_PPROF_PROFILES goroutine_leaks)
	endif()
	list(JOIN GO_PORT_PPROF_PROFILES " " GO_PORT_PPROF_PROFILES)

	add_test(NAME ${target}_pprof
		COMMAND ${CMAKE_COMMAND}
			-DGOPATH="${GOPATH}"
			-DCMAKE_Go_COMPILER="${CMAKE_Go_COMPILER}"
			-DCGO_CFLAGS="${GO_PORT_CGO_CFLAGS}"
			-DCGO_LDFLAGS="${GO_PORT_CGO_LDFLAGS}"
			-DLIBRARY_PATH_NAME="${PROJECT_LIBRARY_PATH_NAME}"
			-DLIBRARY_PATH="${PROJECT_OUTPUT_DIR}"
			-DGO_TASK="test"
			-DGO_FLAGS="${GO_PORT_PPROF_FLAGS}"
			-DGO_CC="${GO_PORT_CC}"
			-DGO_SANITIZER_CFLAGS="${GO_PORT_SANITIZER_CFLAGS}"
			-DGO_SANITIZER_LDFLAGS="${GO_PORT_SANITIZER_LDFLAGS}"
			-DCOMPILE_DEFINITIONS="${GO_PORT_COMPILE_DEFINITIONS}"
			-DGO_PPROF_DIR="${GO_PORT_PPROF_DIR}"
			-DGO_PPROF_BINARY="${GO_PORT_PPROF_DIR}/${target}.test"
			-DGO_PPROF_PROFILES="${GO_PORT_PPROF_PROFILES}"
			-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMakeGoProfiler.cmake
	)

	set_property(TEST ${target}_pprof
		PROPERTY LABELS ${target}
	)

	set_property(TEST ${target}_pprof
		PROPERTY RESOURCE_LOCK ${target}_sources
	)

	test_environment_variables(${target}_pprof
		""
		${GO_PORT_TEST_ENVIRONMENT_VARIABLES}
		"PPROFDIR=${GO_PORT_PPROF_DIR}"
	)
endif()

#
# Define benchmark test
#

if(NOT OPTION_BUILD_BENCHMARKS)
	return()
endif()

# Benchmarks are run without race detector because it distorts the results, sanitizers are kept
# because metacall is instrumented and it must be linked with them
set(GO_PORT_BENCH_DIR "${CMAKE_CURRENT_BINARY_DIR}/bench")
file(MAKE_DIRECTORY "${GO_PORT_BENCH_DIR}")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/benchmarks")

set(GO_PORT_BENCH_FLAGS ${GO_PORT_FLAGS})
list(REMOVE_ITEM GO_PORT_BENCH_FLAGS -race)
list(APPEND GO_PORT_BENCH_FLAGS
	-run ^$
	-bench .
	-benchmem
)
list(JOIN GO_PORT_BENCH_FLAGS " " GO_PORT_BENCH_FLAGS)

add_test(NAME ${target}_bench
	COMMAND ${CMAKE_COMMAND}
		-DGOPATH="${GOPATH}"
		-DCMAKE_Go_COMPILER="${CMAKE_Go_COMPILER}"
		-DCGO_CFLAGS="${GO_PORT_CGO_CFLAGS}"
		-DCGO_LDFLAGS="${GO_PORT_CGO_LDFLAGS}"
		-DLIBRARY_PATH_NAME="${PROJECT_LIBRARY_PATH_NAME}"
		-DLIBRARY_PATH="${PROJECT_OUTPUT_DIR}"
		-DGO_TASK="test"
		-DGO_FLAGS="${GO_PORT_BENCH_FLAGS}"
		-DGO_CC="${GO_PORT_CC}"
		-DGO_SANITIZER_CFLAGS="${GO_PORT_SANITIZER_CFLAGS}"
		-DGO_SANITIZER_LDFLAGS="${GO_PORT_SANITIZER_LDFLAGS}"
		-DCOMPILE_DEFINITIONS="${GO_PORT_COMPILE_DEFINITIONS}"
		-DGO_OUTPUT_FILE="${CMAKE_BINARY_DIR}/benchmarks/${target}_bench.txt"
		-P ${CMAKE_CURRENT_SOURCE_DIR}/source/go_port.cmake
)

set_property(TEST ${target}_bench
	PROPERTY LABELS ${target}
)

set_property(TEST ${target}_bench
	PROPERTY RESOURCE_LOCK ${target}_sources
)

# Disable the verbose output of the sanitizers, it is mixed with the benchmark results and breaks the format
set(GO_PORT_BENCH_ENVIRONMENT_VARIABLES ${GO_PORT_TEST_ENVIRONMENT_VARIABLES})
list(TRANSFORM GO_PORT_BENCH_ENVIRONMENT_VARIABLES REPLACE "verbosity=1" "verbosity=0")
list(TRANSFORM GO_PORT_BENCH_ENVIRONMENT_VARIABLES REPLACE "log_threads=1" "log_threads=0")

test_environment_variables(${target}_bench
	""
	${GO_PORT_BENCH_ENVIRONMENT_VARIABLES}
	"PPROFDIR=${GO_PORT_BENCH_DIR}"
)
