# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_LOADERS_TS
	OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_NODE OR NOT OPTION_BUILD_SCRIPTS_TS)
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-typescript-tsx-loop-fail-test)
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
	${source_path}/metacall_typescript_tsx_loop_fail_test.cpp
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
# Compile features
#

target_compile_features(${target}
	PRIVATE
	cxx_std_17
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
	# WARNING: ThreadSanitizer: signal-unsafe call inside of a signal (pid=13916)
	# 	#0 malloc ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:647 (libtsan.so.2+0x3ebb8)
	# 	#1 <null> <null> (ld-linux-x86-64.so.2+0x28df)
	# 	#2 <null> <null> (libruby-2.7.so.2.7+0x237879)
	# 	#3 simple_netcore_create /usr/local/metacall/source/loaders/cs_loader/source/simple_netcore.cpp:42 (libcs_loaderd.so+0x108de)
	# 	#4 cs_loader_impl_initialize /usr/local/metacall/source/loaders/cs_loader/source/cs_loader_impl.c:236 (libcs_loaderd.so+0xf5fe)
	# 	#5 loader_impl_initialize /usr/local/metacall/source/loader/source/loader_impl.c:367 (libmetacalld.so+0x30673)
	# 	#6 loader_impl_load_from_file /usr/local/metacall/source/loader/source/loader_impl.c:822 (libmetacalld.so+0x30888)
	# 	#7 loader_load_from_file /usr/local/metacall/source/loader/source/loader.c:307 (libmetacalld.so+0x2e0d1)
	# 	#8 metacall_load_from_file /usr/local/metacall/source/metacall/source/metacall.c:348 (libmetacalld.so+0x32bbf)
	# 	#9 node_loader_port_load_from_file_export(napi_env__*, napi_callback_info__*) /usr/local/metacall/source/loaders/node_loader/source/node_loader_port.cpp:395 (libnode_loaderd.so+0x113c5)
	# 	#10 <null> <null> (libnode.so.72+0x7b6344)
	# 	#11 node_loader_impl_async_func_call_safe /usr/local/metacall/source/loaders/node_loader/source/node_loader_impl.cpp:2040 (libnode_loaderd.so+0xe2e8)
	# 	#12 <null> <null> (libnode.so.72+0x7b6344)
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

add_dependencies(${target}
	node_loader
	ts_loader
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
