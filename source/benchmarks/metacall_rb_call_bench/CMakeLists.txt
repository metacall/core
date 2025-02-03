# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_RB)
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-rb-call-bench)
message(STATUS "Benchmark ${target}")

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
	${source_path}/metacall_rb_call_bench.cpp
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

	GBench

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
# Linker options
#

add_link_options(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

#
# Define test
#

if(OPTION_BUILD_ADDRESS_SANITIZER OR OPTION_BUILD_THREAD_SANITIZER)
	# TODO: This test fails when run with sanitizers:
	#
	# Address Sanitizer:
	# ERROR: AddressSanitizer: heap-use-after-free on address 0x629000003a50 at pc 0x7fb5059be061 bp 0x6290000032d0 sp 0x629000002a80
	# WRITE of size 22 at 0x629000003a50 thread T0
	#
	# Thread Sanitizer:
	# WARNING: ThreadSanitizer: signal-unsafe call inside of a signal (pid=14308)
	# 	#0 malloc ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:647 (libtsan.so.2+0x3ebb8)
	# 	#1 <null> <null> (ld-linux-x86-64.so.2+0x28df)
	# 	#2 <null> <null> (libruby-2.7.so.2.7+0x237879)
	# 	#3 loader_impl_initialize /usr/local/metacall/source/loader/source/loader_impl.c:367 (libmetacalld.so+0x30673)
	# 	#4 loader_impl_load_from_memory /usr/local/metacall/source/loader/source/loader_impl.c:945 (libmetacalld.so+0x30df1)
	# 	#5 loader_load_from_memory /usr/local/metacall/source/loader/source/loader.c:327 (libmetacalld.so+0x2e1d1)
	# 	#6 metacall_load_from_memory /usr/local/metacall/source/metacall/source/metacall.c:357 (libmetacalld.so+0x32c00)
	# 	#7 metacall_rb_call_bench::SetUp(benchmark::State&) /usr/local/metacall/source/benchmarks/metacall_rb_call_bench/source/metacall_rb_call_bench.cpp:51 (metacall-rb-call-benchd+0x1a420)
	# 	#8 benchmark::internal::BenchmarkInstance::Run(unsigned long, int, benchmark::internal::ThreadTimer*, benchmark::internal::ThreadManager*, benchmark::internal::PerfCountersMeasurement*) const <null> (metacall-rb-call-benchd+0x5f41f)
	# 	#9 <null> <null> (libc.so.6+0x29209)
	#
	# SUMMARY: ThreadSanitizer: signal-unsafe call inside of a signal (/lib64/ld-linux-x86-64.so.2+0x28df)
	#
	# For solving this, we should enable Ruby support for sanitizers and debug it properly
	return()
endif()

add_test(NAME ${target}
	COMMAND $<TARGET_FILE:${target}>
		--benchmark_out=${CMAKE_BINARY_DIR}/benchmarks/${target}.json
)

#
# Define dependencies
#

add_dependencies(${target}
	rb_loader
)

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
