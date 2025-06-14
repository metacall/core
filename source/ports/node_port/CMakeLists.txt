# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_NODE OR NOT OPTION_BUILD_LOADERS_NODE)
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target node_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

#
# Create target
#

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")

set(sources
	${source_path}/test/index.js
	${source_path}/index.js
	${source_path}/test.js
)

# Group source files
set(source_group "Source Files")
source_group_by_path(${source_path}  "\\\\.js$"
	${source_group} ${sources})

if(NOT OPTION_BUILD_GUIX)
	find_package(NPM)

	if(NOT NPM_FOUND)
		message(SEND_ERROR "NPM not found")
		return()
	endif()

	add_custom_target(${target} ALL
		COMMAND ${CMAKE_COMMAND} -E echo "Installing ${target}"
		COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_OUTPUT_DIR}/node_modules
		COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_OUTPUT_DIR}/node_modules/metacall
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/index.js ${PROJECT_OUTPUT_DIR}/node_modules/metacall/index.js
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/index.d.ts ${PROJECT_OUTPUT_DIR}/node_modules/metacall/index.d.ts
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/package.json ${PROJECT_OUTPUT_DIR}/node_modules/metacall/package.json
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/package-lock.json ${PROJECT_OUTPUT_DIR}/node_modules/metacall/package-lock.json
		COMMAND ${NPM_EXECUTABLE} install
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		SOURCES ${sources}
	)
else()
	add_custom_target(${target} ALL
		COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_OUTPUT_DIR}/node_modules
		COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_OUTPUT_DIR}/node_modules/metacall
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/index.js ${PROJECT_OUTPUT_DIR}/node_modules/metacall/index.js
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/index.d.ts ${PROJECT_OUTPUT_DIR}/node_modules/metacall/index.d.ts
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/package.json ${PROJECT_OUTPUT_DIR}/node_modules/metacall/package.json
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/package-lock.json ${PROJECT_OUTPUT_DIR}/node_modules/metacall/package-lock.json
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		SOURCES ${sources}
	)
endif()

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

#
# Install command
#

# Files
install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/index.js
	${CMAKE_CURRENT_SOURCE_DIR}/index.d.ts
	${CMAKE_CURRENT_SOURCE_DIR}/package.json
	${CMAKE_CURRENT_SOURCE_DIR}/package-lock.json
	${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
	DESTINATION ${INSTALL_LIB}/node_modules/metacall
	COMPONENT runtime
)

#
# Configure test
#

# Check if loaders are enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_RB OR NOT OPTION_BUILD_LOADERS_TS OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_SCRIPTS_RB OR NOT OPTION_BUILD_SCRIPTS_TS)
	return()
endif()

if(OPTION_BUILD_THREAD_SANITIZER AND OPTION_BUILD_LOADERS_CS)
	# TODO: This test fails when run with thread sanitizer:
	#
	# WARNING: ThreadSanitizer: signal-unsafe call inside of a signal (pid=13328)
	# 	#0 free ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:706 (libtsan.so.2+0x47e82)
	# 	#1 _IO_setb <null> (libc.so.6+0x818a4)
	# 	#2 <null> <null> (libruby-2.7.so.2.7+0x237b64)
	# 	#3 simple_netcore_create /usr/local/metacall/source/loaders/cs_loader/source/simple_netcore.cpp:42 (libcs_loaderd.so+0x108de)
	# 	#4 cs_loader_impl_initialize /usr/local/metacall/source/loaders/cs_loader/source/cs_loader_impl.c:236 (libcs_loaderd.so+0xf5fe)
	# 	#5 loader_impl_initialize /usr/local/metacall/source/loader/source/loader_impl.c:367 (libmetacalld.so+0x306a3)
	# 	#6 loader_impl_load_from_file /usr/local/metacall/source/loader/source/loader_impl.c:822 (libmetacalld.so+0x308b8)
	# 	#7 loader_load_from_file /usr/local/metacall/source/loader/source/loader.c:307 (libmetacalld.so+0x2e101)
	# 	#8 metacall_load_from_file /usr/local/metacall/source/metacall/source/metacall.c:348 (libmetacalld.so+0x32bef)
	# 	#9 node_loader_port_load_from_file_export(napi_env__*, napi_callback_info__*) /usr/local/metacall/source/loaders/node_loader/source/node_loader_port.cpp:395 (libnode_loaderd.so+0x113c5)
	# 	#10 <null> <null> (libnode.so.72+0x7b6344)
	#
	# SUMMARY: ThreadSanitizer: signal-unsafe call inside of a signal (/lib/x86_64-linux-gnu/libc.so.6+0x818a4) in _IO_setb
	#
	# For solving this, we should enable C# support for sanitizers and debug it properly
	return()
endif()

#
# Define environment variables
#

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

# Disable OpenSSL related tests if versions are incompatible (TODO: Review this bug and remove the workaround if possible)
set(NodeJS_EXECUTABLE_ONLY ON)

find_package(NodeJS)
find_package(Python COMPONENTS Interpreter)

if(NodeJS_FOUND AND Python_Interpreter_FOUND)
	execute_process(
		COMMAND ${NodeJS_EXECUTABLE} -e "console.log(process.versions.openssl)"
		OUTPUT_VARIABLE NODEJS_OPENSSL_VERSION
	)
	execute_process(
		COMMAND ${Python_EXECUTABLE} -c "import ssl; print(ssl.OPENSSL_VERSION.split()[1])"
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

#
# Test importing NodeJS Port from CLI
#

set(node_port_test "${target}_test")

if(OPTION_BUILD_CLI)
	message(STATUS "Test ${node_port_test}")

	add_test(NAME ${target}
		COMMAND $<TARGET_FILE:metacallcli> ${CMAKE_CURRENT_SOURCE_DIR}/test.js
	)

	#
	# Define test labels
	#

	set_property(TEST ${target}
		PROPERTY LABELS ${node_port_test}
	)

	# Add dependencies and optional dependencies
	add_dependencies(${target}
		node_loader
		mock_loader
		py_loader
		rb_loader
		ts_loader
		${COBOL_DEPENDENCY}
		${C_DEPENDENCY}
		${RS_DEPENDENCY}
	)

	# Environment variables
	test_environment_variables(${target}
		""
		${TESTS_ENVIRONMENT_VARIABLES}
		${TESTS_ENVIRONMENT_VARIABLES_COB}
		${TESTS_ENVIRONMENT_VARIABLES_C}
		${TESTS_ENVIRONMENT_VARIABLES_RS}
		${TESTS_ENVIRONMENT_VARIABLES_OPENSSL}
	)
endif()

#
# Test importing NodeJS Port from node.exe
#

set(node_port_test_exec "${node_port_test}_executable")

# TODO: Since MacOS 14 with ARM64, it does not support well NodeJS executable with preloaded sanitizers
# The NodeJS initalization fails with: Node Loader failed to hook napi_register_module_v1
if(APPLE)
	# Check if NodeJS is compiled with Address Sanitizer
	if(OPTION_BUILD_ADDRESS_SANITIZER)
		check_asan_executable("${NodeJS_EXECUTABLE}" NodeJS_ASAN)
		if(NOT NodeJS_ASAN)
			# Skip this test because it gives false positives if NodeJS is not compiled with ASan
			return()
		endif()
	endif()

	# Check if NodeJS is compiled with Thread Sanitizer
	if(OPTION_BUILD_THREAD_SANITIZER)
		check_tsan_executable("${NodeJS_EXECUTABLE}" NodeJS_TSAN)
		if(NOT NodeJS_TSAN)
			# Skip this test because it gives false positives if NodeJS is not compiled with TSan
			return()
		endif()
	endif()
endif()

message(STATUS "Test ${node_port_test_exec}")

add_test(NAME ${node_port_test_exec}
	COMMAND ${NodeJS_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/test.js"
)

# Define test labels
set_property(TEST ${node_port_test_exec}
	PROPERTY LABELS ${node_port_test_exec}
)

# Environment variables
test_environment_variables(${node_port_test_exec}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
	${TESTS_ENVIRONMENT_VARIABLES_COB}
	${TESTS_ENVIRONMENT_VARIABLES_C}
	${TESTS_ENVIRONMENT_VARIABLES_RS}
	${TESTS_ENVIRONMENT_VARIABLES_OPENSSL}
	"METACALL_INSTALL_PATH=${PROJECT_OUTPUT_DIR}"
	${TESTS_SANITIZER_ENVIRONMENT_VARIABLES}
	${TESTS_SANITIZER_PRELOAD_ENVIRONMENT_VARIABLES}
)
