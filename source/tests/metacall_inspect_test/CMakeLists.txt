#
# Executable name and options
#

# Target name
set(target metacall-inspect-test)
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
	${source_path}/metacall_inspect_test.cpp
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
# Dependecies
#

add_dependencies(${target}
	${META_PROJECT_NAME}::metacall
)

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

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

#
# Define test
#

if(OPTION_BUILD_THREAD_SANITIZER AND OPTION_BUILD_LOADERS_CS)
	# TODO: This test fails when run with thread sanitizer (this happens when C# loader is enabled):
	#
	# WARNING: ThreadSanitizer: signal-unsafe call inside of a signal (pid=13749)
	# 	#0 malloc ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:647 (libtsan.so.2+0x3ebb8)
	# 	#1 <null> <null> (ld-linux-x86-64.so.2+0x28df)
	# 	#2 <null> <null> (libruby-2.7.so.2.7+0x237879)
	# 	#3 simple_netcore_create /usr/local/metacall/source/loaders/cs_loader/source/simple_netcore.cpp:42 (libcs_loaderd.so+0x108de)
	# 	#4 cs_loader_impl_initialize /usr/local/metacall/source/loaders/cs_loader/source/cs_loader_impl.c:236 (libcs_loaderd.so+0xf5fe)
	# 	#5 loader_impl_initialize /usr/local/metacall/source/loader/source/loader_impl.c:367 (libmetacalld.so+0x306a3)
	# 	#6 loader_impl_load_from_file /usr/local/metacall/source/loader/source/loader_impl.c:822 (libmetacalld.so+0x308b8)
	# 	#7 loader_load_from_file /usr/local/metacall/source/loader/source/loader.c:307 (libmetacalld.so+0x2e101)
	# 	#8 metacall_load_from_file /usr/local/metacall/source/metacall/source/metacall.c:348 (libmetacalld.so+0x32bef)
	# 	#9 metacall_inspect_test_DefaultConstructor_Test::TestBody() /usr/local/metacall/source/tests/metacall_inspect_test/source/metacall_inspect_test.cpp:101 (metacall-inspect-testd+0x20eaa)
	# 	#10 void testing::internal::HandleExceptionsInMethodIfSupported<testing::Test, void>(testing::Test*, void (testing::Test::*)(), char const*) <null> (metacall-inspect-testd+0x54156)
	# 	#11 <null> <null> (libc.so.6+0x29209)
	#
	# SUMMARY: ThreadSanitizer: signal-unsafe call inside of a signal (/lib64/ld-linux-x86-64.so.2+0x28df)
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
	py_loader
	rb_loader
	cs_loader
	jsm_loader
	js_loader
	mock_loader
	c_loader
)

#
# Define test properties
#

set_property(TEST ${target}
	PROPERTY LABELS ${target} MEMCHECK_IGNORE
)

if(OPTION_BUILD_SANITIZER AND OPTION_BUILD_LOADERS_CS)
	# TODO: This test fails when run with sanitizers (this happens when C# loader is enabled):
	# Tracer caught signal 11: addr=0x600000590 pc=0x7fd0975400f0 sp=0x7fd091d32d10
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
