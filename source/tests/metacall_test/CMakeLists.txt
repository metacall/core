#
# Executable name and options
#

# Target name
set(target metacall-test)
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
	${source_path}/metacall_test.cpp
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
# Linker options
#

target_link_options(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

#
# Define test
#

if(OPTION_BUILD_THREAD_SANITIZER AND OPTION_BUILD_LOADERS_CS)
	# TODO: This test fails when run with thread sanitizer (this happens when C# loader is enabled):
	#
	# WARNING: ThreadSanitizer: data race (pid=13427)
	# Write of size 8 at 0x7b5c00010680 by thread T8:
	# 	#0 free ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:706 (libtsan.so.2+0x47e82)
	# 	#1 <null> <null> (libcoreclr.so+0x36ba88)
	#
	# Previous write of size 8 at 0x7b5c00010680 by main thread:
	# 	[failed to restore the stack]
	#
	# Thread T8 '.NET ThreadPool' (tid=13563, running) created by thread T7 at:
	# 	#0 pthread_create ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:1001 (libtsan.so.2+0x5e686)
	# 	#1 <null> <null> (libcoreclr.so+0x4e90ce)
	#
	# SUMMARY: ThreadSanitizer: data race (/usr/share/dotnet/shared/Microsoft.NETCore.App/5.0.17/libcoreclr.so+0x36ba88)
	#
	# For solving this, we should enable C# support for sanitizers and debug it properly
	return()
endif()

add_test(NAME ${target}
	COMMAND $<TARGET_FILE:${target}>
)

#
# Define dependencies
#

add_loader_dependencies(${target}
	node_loader
	py_loader
	rb_loader
	cs_loader
	jsm_loader
	js_loader
	mock_loader
	c_loader
	file_loader
)

#
# Define test properties
#

set_property(TEST ${target}
	PROPERTY LABELS ${target} MEMCHECK_IGNORE
)

if(OPTION_BUILD_ADDRESS_SANITIZER AND OPTION_BUILD_LOADERS_CS)
	# TODO: This test fails when run with sanitizer (this happens when C# loader is enabled):
	#
	# Tracer caught signal 11: addr=0x600000690 pc=0x7f3a7b6710f0 sp=0x7f3a75e32d10
	# LeakSanitizer has encountered a fatal error.
	# HINT: For debugging, try setting environment variable LSAN_OPTIONS=verbosity=1:log_threads=1
	# HINT: LeakSanitizer does not work under ptrace (strace, gdb, etc)
	#
	# For solving this, we should enable C# support for sanitizers and debug it properly
	set_tests_properties(${target} PROPERTIES
		PASS_REGULAR_EXPRESSION "[  PASSED  ]"
	)
endif()

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)
