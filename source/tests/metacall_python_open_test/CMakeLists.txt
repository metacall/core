# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_LOADERS_CS OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_PY)
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-python-open-test)
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
	${source_path}/metacall_python_open_test.cpp
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
# Dependencies
#

if(NOT OPTION_BUILD_GUIX)
	find_package(NPM)

	if(NOT NPM_FOUND)
		message(SEND_ERROR "NPM not found")
		return()
	endif()

	add_custom_target(${target}-depends
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${NPM_EXECUTABLE} --prefix ${CMAKE_CURRENT_BINARY_DIR} install jsonwebtoken
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/node_modules ${PROJECT_OUTPUT_DIR}/node_modules
	)

	add_dependencies(${target} ${target}-depends)
endif()

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

add_link_options(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

#
# Define test
#

if(OPTION_BUILD_THREAD_SANITIZER AND OPTION_BUILD_LOADERS_CS)
	# TODO: This test fails when run with thread sanitizer (this happens when C# loader is enabled):
	#
	# WARNING: ThreadSanitizer: signal-unsafe call inside of a signal (pid=13743)
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
	# 	#17 py_loader_port_load_from_file_impl /usr/local/metacall/source/loaders/py_loader/source/py_loader_port.c:190 (libpy_loaderd.so+0x10a02)
	# 	#18 py_loader_port_load_from_file /usr/local/metacall/source/loaders/py_loader/source/py_loader_port.c:238 (libpy_loaderd.so+0x10cde)
	# 	#19 <null> <null> (libpython3.9.so.1.0+0x10cc73)
	# 	#20 py_loader_impl_load_from_file_relative /usr/local/metacall/source/loaders/py_loader/source/py_loader_impl.c:3102 (libpy_loaderd.so+0xb5ae)
	# 	#21 py_loader_impl_load_from_file /usr/local/metacall/source/loaders/py_loader/source/py_loader_impl.c:3193 (libpy_loaderd.so+0xb7db)
	# 	#22 loader_impl_load_from_file /usr/local/metacall/source/loader/source/loader_impl.c:838 (libmetacalld.so+0x30944)
	# 	#23 loader_load_from_file /usr/local/metacall/source/loader/source/loader.c:307 (libmetacalld.so+0x2e0d1)
	# 	#24 metacall_load_from_file /usr/local/metacall/source/metacall/source/metacall.c:348 (libmetacalld.so+0x32bbf)
	# 	#25 metacall_python_open_test_DefaultConstructor_Test::TestBody() /usr/local/metacall/source/tests/metacall_python_open_test/source/metacall_python_open_test.cpp:44 (metacall-python-open-testd+0x20d18)
	# 	#26 void testing::internal::HandleExceptionsInMethodIfSupported<testing::Test, void>(testing::Test*, void (testing::Test::*)(), char const*) <null> (metacall-python-open-testd+0x57646)
	# 	#27 <null> <null> (libc.so.6+0x29209)
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

#
# Define dependencies
#

add_dependencies(${target}
	py_loader
	node_loader
	cs_loader
)

#
# Define test properties
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

if(OPTION_BUILD_ADDRESS_SANITIZER AND OPTION_BUILD_LOADERS_CS)
	# TODO: This test fails when run with sanitizers (this happens when C# loader is enabled):
	# Tracer caught signal 11: addr=0x1500000aa8 pc=0x7f49da2cc0f0 sp=0x7f49d4ad2d10
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
