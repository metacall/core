# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_CS OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_CS)
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-cs-test)
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

set(headers
	${include_path}/environment.hpp
)

set(sources
	${source_path}/main.cpp
	${source_path}/environment.cpp
	${source_path}/metacall_cs_test.cpp
)

# Group source files
set(header_group "Header Files")
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
	${headers}
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
	${CMAKE_CURRENT_SOURCE_DIR}/include
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
# Define dependencies
#

add_dependencies(${target}
	cs_loader
)

#
# Define test
#

if(OPTION_BUILD_THREAD_SANITIZER)
	# TODO: This test fails when run with thread sanitizer:
	#
	# WARNING: ThreadSanitizer: signal-unsafe call inside of a signal (pid=13983)
	# 	#0 operator new(unsigned long) ../../../../src/libsanitizer/tsan/tsan_new_delete.cpp:64 (libtsan.so.2+0x87323)
	# 	#1 std::__new_allocator<void*>::allocate(unsigned long, void const*) /usr/include/c++/12/bits/new_allocator.h:137 (libbacktrace_plugind.so+0x7096)
	# 	#2 std::allocator_traits<std::allocator<void*> >::allocate(std::allocator<void*>&, unsigned long) /usr/include/c++/12/bits/alloc_traits.h:464 (libbacktrace_plugind.so+0x7096)
	# 	#3 std::_Vector_base<void*, std::allocator<void*> >::_M_allocate(unsigned long) /usr/include/c++/12/bits/stl_vector.h:378 (libbacktrace_plugind.so+0x7096)
	# 	#4 std::vector<void*, std::allocator<void*> >::_M_default_append(unsigned long) /usr/include/c++/12/bits/vector.tcc:650 (libbacktrace_plugind.so+0x7096)
	# 	#5 std::vector<void*, std::allocator<void*> >::resize(unsigned long) /usr/include/c++/12/bits/stl_vector.h:1011 (libbacktrace_plugind.so+0x7453)
	# 	#6 backward::StackTraceImpl<backward::system_tag::linux_tag>::load_here(unsigned long, void*, void*) /usr/local/metacall/build/_deps/backwardcpp-src/backward.hpp:879 (libbacktrace_plugind.so+0x7453)
	# 	#7 backward::StackTraceImpl<backward::system_tag::linux_tag>::load_from(void*, unsigned long, void*, void*) /usr/local/metacall/build/_deps/backwardcpp-src/backward.hpp:887 (libbacktrace_plugind.so+0xe4da)
	# 	#8 backward::SignalHandling::handleSignal(int, siginfo_t*, void*) /usr/local/metacall/build/_deps/backwardcpp-src/backward.hpp:4249 (libbacktrace_plugind.so+0xe4da)
	# 	#9 backward::SignalHandling::sig_handler(int, siginfo_t*, void*) /usr/local/metacall/build/_deps/backwardcpp-src/backward.hpp:4276 (libbacktrace_plugind.so+0xfff0)
	# 	#10 <null> <null> (libcoreclr.so+0x4afbdc)
	# 	#11 simple_netcore_create /usr/local/metacall/source/loaders/cs_loader/source/simple_netcore.cpp:42 (libcs_loaderd.so+0x108de)
	# 	#12 cs_loader_impl_initialize /usr/local/metacall/source/loaders/cs_loader/source/cs_loader_impl.c:236 (libcs_loaderd.so+0xf5fe)
	# 	#13 loader_impl_initialize /usr/local/metacall/source/loader/source/loader_impl.c:367 (libmetacalld.so+0x30673)
	# 	#14 loader_impl_load_from_file /usr/local/metacall/source/loader/source/loader_impl.c:822 (libmetacalld.so+0x30888)
	# 	#15 loader_load_from_file /usr/local/metacall/source/loader/source/loader.c:307 (libmetacalld.so+0x2e0d1)
	# 	#16 metacall_load_from_file /usr/local/metacall/source/metacall/source/metacall.c:348 (libmetacalld.so+0x32bbf)
	# 	#17 environment::SetUp() /usr/local/metacall/source/tests/metacall_cs_test/source/environment.cpp:38 (metacall-cs-testd+0x21968)
	# 	#18 testing::internal::UnitTestImpl::RunAllTests() <null> (metacall-cs-testd+0x483c1)
	# 	#19 <null> <null> (libc.so.6+0x29209)
	#
	# SUMMARY: ThreadSanitizer: signal-unsafe call inside of a signal ../../../../src/libsanitizer/tsan/tsan_new_delete.cpp:64 in operator new(unsigned long)
	#
	#
	# For solving this, we should enable C# support for sanitizers and debug it properly
	return()
endif()

add_test(NAME ${target}
	COMMAND $<TARGET_FILE:${target}>
)

if(OPTION_BUILD_SANITIZER)
	# TODO: This test fails when run with sanitizers (this happens when C# loader is enabled):
	# Tracer caught signal 11: addr=0x81e000278 pc=0x7f21968e07c8 sp=0x7ee064c83d20
	# LeakSanitizer has encountered a fatal error.
	# HINT: For debugging, try setting environment variable LSAN_OPTIONS=verbosity=1:log_threads=1
	# HINT: LeakSanitizer does not work under ptrace (strace, gdb, etc)
	#
	# For solving this, we should enable C# support for sanitizers and debug it properly
	set_tests_properties(${target} PROPERTIES
		PASS_REGULAR_EXPRESSION "[  PASSED  ]"
	)
endif()

#
# Define test properties
#

set_property(TEST ${target}
	# TODO: Valgrind also fails with C#
	PROPERTY LABELS ${target} MEMCHECK_IGNORE
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)
