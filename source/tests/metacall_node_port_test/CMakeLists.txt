# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_LOADERS_MOCK OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_RB OR NOT OPTION_BUILD_LOADERS_TS OR NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_NODE OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_SCRIPTS_RB)
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-node-port-test)
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
	${source_path}/metacall_node_port_test.cpp
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

	# NodeJS Port Test path
	METACALL_NODE_PORT_TEST_PATH="${CMAKE_SOURCE_DIR}/source/ports/node_port/test.js"
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
	# WARNING: ThreadSanitizer: signal-unsafe call inside of a signal (pid=13603)
	# 	#0 malloc ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:647 (libtsan.so.2+0x3ebb8)
	# 	#1 <null> <null> (ld-linux-x86-64.so.2+0x28df)
	# 	#2 <null> <null> (libruby-2.7.so.2.7+0x237879)
	# 	#3 simple_netcore_create /usr/local/metacall/source/loaders/cs_loader/source/simple_netcore.cpp:42 (libcs_loaderd.so+0x108de)
	# 	#4 cs_loader_impl_initialize /usr/local/metacall/source/loaders/cs_loader/source/cs_loader_impl.c:236 (libcs_loaderd.so+0xf5fe)
	# 	#5 loader_impl_initialize /usr/local/metacall/source/loader/source/loader_impl.c:367 (libmetacalld.so+0x306a3)
	# 	#6 loader_impl_load_from_file /usr/local/metacall/source/loader/source/loader_impl.c:822 (libmetacalld.so+0x308b8)
	# 	#7 loader_load_from_file /usr/local/metacall/source/loader/source/loader.c:307 (libmetacalld.so+0x2e101)
	# 	#8 metacall_load_from_file /usr/local/metacall/source/metacall/source/metacall.c:348 (libmetacalld.so+0x32bef)
	# 	#9 node_loader_port_load_from_file_export(napi_env__*, napi_callback_info__*) /usr/local/metacall/source/loaders/node_loader/source/node_loader_port.cpp:395 (libnode_loaderd.so+0x113c5)
	# 	#10 <null> <null> (libnode.so.72+0x7b6344)
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
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

include(TestEnvironmentVariables)

# Enable cobol test if it is built
if(OPTION_BUILD_LOADERS_COB)
	set(COBOL_DEPENDENCY cob_loader)
	set(TESTS_ENVIRONMENT_VARIABLES_COB "OPTION_BUILD_LOADERS_COB=1")
endif()

# Enable c test if it is built
if(OPTION_BUILD_LOADERS_C)
	set(C_DEPENDENCY c_loader)
	set(TESTS_ENVIRONMENT_VARIABLES_C "OPTION_BUILD_LOADERS_C=1")
endif()

# Enable rust test if it is built
if(OPTION_BUILD_LOADERS_RS)
	set(RS_DEPENDENCY rs_loader)
	set(TESTS_ENVIRONMENT_VARIABLES_RS "OPTION_BUILD_LOADERS_RS=1")
endif()

# Add dependencies and optional dependencies
add_dependencies(${target}
	node_port
	node_loader
	mock_loader
	py_loader
	rb_loader
	ts_loader
	${COBOL_DEPENDENCY}
	${C_DEPENDENCY}
	${RS_DEPENDENCY}
)

# Disable OpenSSL related tests if versions are incompatible
set(NodeJS_EXECUTABLE_ONLY ON)

find_package(NodeJS)
find_package(Python3 COMPONENTS Interpreter)

if(NodeJS_FOUND AND Python3_Interpreter_FOUND)
	execute_process(
		COMMAND ${NodeJS_EXECUTABLE} -e "console.log(process.versions.openssl)"
		OUTPUT_VARIABLE NODEJS_OPENSSL_VERSION
	)
	execute_process(
		COMMAND ${Python3_EXECUTABLE} -c "import ssl; print(ssl.OPENSSL_VERSION.split()[1])"
		OUTPUT_VARIABLE PYTHON_OPENSSL_VERSION
	)

	if(NOT "${NODEJS_OPENSSL_VERSION}" STREQUAL "" AND NOT "${PYTHON_OPENSSL_VERSION}" STREQUAL "")
		string(REGEX MATCHALL "-.*$|[0-9]+" NODEJS_OPENSSL_PARTIAL_VERSION_LIST "${NODEJS_OPENSSL_VERSION}")
		list(GET NODEJS_OPENSSL_PARTIAL_VERSION_LIST 0 NODEJS_OPENSSL_VERSION_MAJOR)
		list(GET NODEJS_OPENSSL_PARTIAL_VERSION_LIST 1 NODEJS_OPENSSL_VERSION_MINOR)
		list(GET NODEJS_OPENSSL_PARTIAL_VERSION_LIST 2 NODEJS_OPENSSL_VERSION_PATCH)

		string(REGEX MATCHALL "-.*$|[0-9]+" PYTHON_OPENSSL_PARTIAL_VERSION_LIST "${PYTHON_OPENSSL_VERSION}")
		list(GET PYTHON_OPENSSL_PARTIAL_VERSION_LIST 0 PYTHON_OPENSSL_VERSION_MAJOR)
		list(GET PYTHON_OPENSSL_PARTIAL_VERSION_LIST 1 PYTHON_OPENSSL_VERSION_MINOR)
		list(GET PYTHON_OPENSSL_PARTIAL_VERSION_LIST 2 PYTHON_OPENSSL_VERSION_PATCH)

		# If major and minor version match, then enable the OpenSSL related tests (https://github.com/metacall/core/issues/31#issuecomment-1736039845)
		if(NODEJS_OPENSSL_VERSION_MAJOR VERSION_EQUAL PYTHON_OPENSSL_VERSION_MAJOR
			AND NODEJS_OPENSSL_VERSION_MINOR VERSION_EQUAL PYTHON_OPENSSL_VERSION_MINOR)
			set(TESTS_ENVIRONMENT_VARIABLES_OPENSSL "OPTION_NODEJS_PYTHON_OPENSSL_MATCH=1")
		endif()
	endif()
endif()

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
	${TESTS_ENVIRONMENT_VARIABLES_COB}
	${TESTS_ENVIRONMENT_VARIABLES_C}
	${TESTS_ENVIRONMENT_VARIABLES_RS}
	${TESTS_ENVIRONMENT_VARIABLES_OPENSSL}
)
