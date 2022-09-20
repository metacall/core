# Check if loaders are enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_PY OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_NODE)
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-python-port-import-test)
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
	${source_path}/metacall_python_port_import_test.cpp
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

	# Python Port Test path
	METACALL_PYTHON_PORT_PATH="${CMAKE_SOURCE_DIR}/source/ports/py_port"
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
	# WARNING: ThreadSanitizer: signal-unsafe call inside of a signal (pid=13823)
	# 	#0 free ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:706 (libtsan.so.2+0x47e82)
	# 	#1 <null> <null> (libdw.so.1+0x56c62)
	# 	#2 backward::TraceResolverImpl<backward::system_tag::linux_tag>::~TraceResolverImpl() /usr/local/metacall/build/_deps/backwardcpp-src/backward.hpp:3479 (libbacktrace_plugind.so+0xf95c)
	# 	#3 backward::TraceResolver::~TraceResolver() /usr/local/metacall/build/_deps/backwardcpp-src/backward.hpp:3694 (libbacktrace_plugind.so+0xf95c)
	# 	#4 backward::Printer::~Printer() /usr/local/metacall/build/_deps/backwardcpp-src/backward.hpp:3986 (libbacktrace_plugind.so+0xf95c)
	# 	#5 backward::SignalHandling::handleSignal(int, siginfo_t*, void*) /usr/local/metacall/build/_deps/backwardcpp-src/backward.hpp:4265 (libbacktrace_plugind.so+0xf95c)
	# 	#6 backward::SignalHandling::sig_handler(int, siginfo_t*, void*) /usr/local/metacall/build/_deps/backwardcpp-src/backward.hpp:4276 (libbacktrace_plugind.so+0xfff0)
	# 	#7 <null> <null> (libcoreclr.so+0x4afbdc)
	# 	#8 backward::SignalHandling::sig_handler(int, siginfo_t*, void*) /usr/local/metacall/build/_deps/backwardcpp-src/backward.hpp:4279 (libbacktrace_plugind.so+0xffff)
	# 	#9 <null> <null> (libcoreclr.so+0x4afbdc)
	# 	#10 simple_netcore_create /usr/local/metacall/source/loaders/cs_loader/source/simple_netcore.cpp:42 (libcs_loaderd.so+0x108de)
	# 	#11 cs_loader_impl_initialize /usr/local/metacall/source/loaders/cs_loader/source/cs_loader_impl.c:236 (libcs_loaderd.so+0xf5fe)
	# 	#12 loader_impl_initialize /usr/local/metacall/source/loader/source/loader_impl.c:367 (libmetacalld.so+0x306a3)
	# 	#13 loader_impl_load_from_file /usr/local/metacall/source/loader/source/loader_impl.c:822 (libmetacalld.so+0x308b8)
	# 	#14 loader_load_from_file /usr/local/metacall/source/loader/source/loader.c:307 (libmetacalld.so+0x2e101)
	# 	#15 metacall_load_from_file /usr/local/metacall/source/metacall/source/metacall.c:348 (libmetacalld.so+0x32bef)
	# 	#16 py_loader_port_load_from_file_impl /usr/local/metacall/source/loaders/py_loader/source/py_loader_port.c:190 (libpy_loaderd.so+0x10a02)
	# 	#17 py_loader_port_load_from_file_export /usr/local/metacall/source/loaders/py_loader/source/py_loader_port.c:244 (libpy_loaderd.so+0x10d34)
	# 	#18 <null> <null> (libpython3.9.so.1.0+0x10cc73)
	# 	#19 loader_impl_load_from_memory /usr/local/metacall/source/loader/source/loader_impl.c:968 (libmetacalld.so+0x30eba)
	# 	#20 loader_load_from_memory /usr/local/metacall/source/loader/source/loader.c:327 (libmetacalld.so+0x2e201)
	# 	#21 metacall_load_from_memory /usr/local/metacall/source/metacall/source/metacall.c:357 (libmetacalld.so+0x32c30)
	# 	#22 metacall_python_port_import_test_DefaultConstructor_Test::TestBody() /usr/local/metacall/source/tests/metacall_python_port_import_test/source/metacall_python_port_import_test.cpp:48 (metacall-python-port-import-testd+0x20aa5)
	# 	#23 void testing::internal::HandleExceptionsInMethodIfSupported<testing::Test, void>(testing::Test*, void (testing::Test::*)(), char const*) <null> (metacall-python-port-import-testd+0x4e186)
	# 	#24 <null> <null> (libc.so.6+0x29209)
	#
	# SUMMARY: ThreadSanitizer: signal-unsafe call inside of a signal (/usr/lib/x86_64-linux-gnu/libdw.so.1+0x56c62)
	#
	# For solving this, we should enable C# support for sanitizers and debug it properly
	return()
endif()

add_test(NAME ${target}
	COMMAND $<TARGET_FILE:${target}>
	WORKING_DIRECTORY ${LOADER_SCRIPT_PATH}/ramda
)

#
# Define dependencies
#

add_dependencies(${target}
	py_loader
	node_loader
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
	"LOADER_SCRIPT_PATH=${LOADER_SCRIPT_PATH}/ramda"
)
