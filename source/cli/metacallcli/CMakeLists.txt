# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_EXT OR NOT OPTION_BUILD_EXTENSIONS OR NOT OPTION_BUILD_LOADERS_NODE)
	message(WARNING "The Extension and NodeJS loaders are a dependency of the CLI, in order to compile the CLI, enable them with -DOPTION_BUILD_LOADERS_EXT=ON -DOPTION_BUILD_LOADERS_NODE=ON")
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacallcli)

# Exit here if required dependencies are not met
message(STATUS "CLI ${target}")

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
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(headers
	${include_path}/application.hpp
)

set(sources
	${source_path}/application.cpp
	${source_path}/main.cpp
)

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
# Target for scripts (testing)
#

set(PROJECT_METACALL_PORTS_DIRECTORY "${CMAKE_SOURCE_DIR}/source/ports/py_port")

add_custom_target(${target}-scripts-tests ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${LOADER_SCRIPT_PATH}
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test/ ${LOADER_SCRIPT_PATH}/
	COMMAND ${CMAKE_COMMAND} -DPROJECT_METACALL_PORTS_DIRECTORY=${PROJECT_METACALL_PORTS_DIRECTORY} -DLOADER_SCRIPT_PATH=${LOADER_SCRIPT_PATH} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure_tests.cmake
)

set_target_properties(${target}-scripts-tests
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

add_dependencies(${target} ${target}-scripts-tests)

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
	${PROJECT_BINARY_DIR}/source/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include
	${DEFAULT_INCLUDE_DIRECTORIES}

	PUBLIC

	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

#
# Libraries
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LIBRARIES}
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
	cxx_std_17 # Required for filesystem
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

add_loader_dependencies(${target}
	c_loader
	cob_loader
	cs_loader
	ext_loader
	file_loader
	java_loader
	mock_loader
	py_loader
	rb_loader
	rs_loader
	rpc_loader
	ts_loader
	wasm_loader
)

add_dependencies(${target}
	node_loader
	cli_repl_plugin
	cli_core_plugin
)

if(TARGET cli_cmd_plugin)
	add_dependencies(${target}
		cli_cmd_plugin
	)
endif()

#
# Deployment
#

# Executable
install(TARGETS ${target}
	RUNTIME DESTINATION ${INSTALL_BIN} COMPONENT cli
	BUNDLE  DESTINATION ${INSTALL_BIN} COMPONENT cli
)

#
# Define test
#

# Check if tests are enabled
if(NOT OPTION_BUILD_TESTS)
	return()
endif()

# Define variables for testing the CLI interactively
set(TEST_COMMAND_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/test/commands/${target}")
set(TEST_COMMAND_RUNNER "${CMAKE_CURRENT_SOURCE_DIR}/test/commands/command_runner.cmake")

include(TestEnvironmentVariables)

add_test(NAME ${target}
	COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}.txt" -P ${TEST_COMMAND_RUNNER}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_property(TEST ${target}
	PROPERTY LABELS ${target}
)
set_tests_properties(${target} PROPERTIES
	PASS_REGULAR_EXPRESSION "function three_str\\(a_str, b_str, c_str\\)"
)
test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)

add_test(NAME ${target}-inspect-leak
	COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-inspect-leak.txt" -P ${TEST_COMMAND_RUNNER}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_property(TEST ${target}-inspect-leak
	PROPERTY LABELS ${target}-inspect-leak
)
test_environment_variables(${target}-inspect-leak
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)

if(OPTION_BUILD_LOADERS AND OPTION_BUILD_LOADERS_NODE AND OPTION_BUILD_SCRIPTS AND OPTION_BUILD_SCRIPTS_NODE)
	add_test(NAME ${target}-node
		COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-node.txt" -P ${TEST_COMMAND_RUNNER}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	set_property(TEST ${target}-node
		PROPERTY LABELS ${target}-node
	)
	set_tests_properties(${target}-node PROPERTIES
		PASS_REGULAR_EXPRESSION "4001534"
	)
	test_environment_variables(${target}-node
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)
	if(OPTION_BUILD_PORTS AND OPTION_BUILD_PORTS_NODE AND OPTION_BUILD_LOADERS_PY)
		add_test(NAME ${target}-node-port-py
			COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-node-port-py.txt" -P ${TEST_COMMAND_RUNNER}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)
		set_property(TEST ${target}-node-port-py
			PROPERTY LABELS ${target}-node-port-py
		)
		set_tests_properties(${target}-node-port-py PROPERTIES
			PASS_REGULAR_EXPRESSION "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		)
		test_environment_variables(${target}-node-port-py
			""
			${TESTS_ENVIRONMENT_VARIABLES}
		)

		if(OPTION_BUILD_LOADERS_RB)
			add_test(NAME ${target}-node-port-py-rb
				COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-node-port-py-rb.txt" -P ${TEST_COMMAND_RUNNER}
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			)
			set_property(TEST ${target}-node-port-py-rb
				PROPERTY LABELS ${target}-node-port-py-rb
			)
			test_environment_variables(${target}-node-port-py-rb
				""
				${TESTS_ENVIRONMENT_VARIABLES}
			)
		endif()
	endif()

	add_test(NAME ${target}-node-null
		COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-node-null.txt" -P ${TEST_COMMAND_RUNNER}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	set_property(TEST ${target}-node-null
		PROPERTY LABELS ${target}-node-null
	)
	set_tests_properties(${target}-node-null PROPERTIES
		PASS_REGULAR_EXPRESSION "Hello 342521512461246!"
	)
	test_environment_variables(${target}-node-null
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)

	add_test(NAME ${target}-node-null-empty
		COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-node-null-empty.txt" -P ${TEST_COMMAND_RUNNER}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	set_property(TEST ${target}-node-null-empty
		PROPERTY LABELS ${target}-node-null-empty
	)
	set_tests_properties(${target}-node-null-empty PROPERTIES
		PASS_REGULAR_EXPRESSION "Hello 342521512461246!"
	)
	test_environment_variables(${target}-node-null-empty
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)

	add_test(NAME ${target}-node-null-undefined
		COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-node-null-undefined.txt" -P ${TEST_COMMAND_RUNNER}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	set_property(TEST ${target}-node-null-undefined
		PROPERTY LABELS ${target}-node-null-undefined
	)
	set_tests_properties(${target}-node-null-undefined PROPERTIES
		PASS_REGULAR_EXPRESSION "(null)"
	)
	test_environment_variables(${target}-node-null-undefined
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)

	if(OPTION_BUILD_LOADERS_PY AND OPTION_BUILD_PORTS AND OPTION_BUILD_PORTS_PY)
		add_test(NAME ${target}-py-port
			COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-py-port.txt" -P ${TEST_COMMAND_RUNNER}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)
		set_property(TEST ${target}-py-port
			PROPERTY LABELS ${target}-py-port
		)
		set_tests_properties(${target}-py-port PROPERTIES
			PASS_REGULAR_EXPRESSION "1234"
		)
		test_environment_variables(${target}-py-port
			""
			${TESTS_ENVIRONMENT_VARIABLES}
		)

		if(OPTION_BUILD_LOADERS_RB)
			add_test(NAME ${target}-py-port-rb
				COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-py-port-rb.txt" -P ${TEST_COMMAND_RUNNER}
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			)
			set_property(TEST ${target}-py-port-rb
				PROPERTY LABELS ${target}-py-port-rb
			)
			set_tests_properties(${target}-py-port-rb PROPERTIES
				PASS_REGULAR_EXPRESSION "0123456789ABCDEFasd"
			)
			test_environment_variables(${target}-py-port-rb
				""
				${TESTS_ENVIRONMENT_VARIABLES}
			)
		endif()
	endif()
endif()

if(OPTION_BUILD_LOADERS AND OPTION_BUILD_LOADERS_FILE AND OPTION_BUILD_SCRIPTS AND OPTION_BUILD_SCRIPTS_FILE)
	add_test(NAME ${target}-file
		COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-file.txt" -P ${TEST_COMMAND_RUNNER}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	set_property(TEST ${target}-file
		PROPERTY LABELS ${target}-file
	)
	set_tests_properties(${target}-file PROPERTIES
		PASS_REGULAR_EXPRESSION "${LOADER_SCRIPT_PATH}/template.html"
	)
	test_environment_variables(${target}-file
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)
	add_test(NAME ${target}-file-fail
		COMMAND $<TARGET_FILE:${target}> this-does-not-exist
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	set_property(TEST ${target}-file-fail
		PROPERTY LABELS ${target}-file-fail
	)
	set_tests_properties(${target}-file-fail PROPERTIES
		PASS_REGULAR_EXPRESSION "Error: Failed to load script 'this-does-not-exist' with loader 'file'"
	)
	test_environment_variables(${target}-file-fail
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)
endif()

if(OPTION_BUILD_LOADERS AND OPTION_BUILD_LOADERS_PY)
	add_test(NAME ${target}-py-naming
		COMMAND $<TARGET_FILE:${target}> test.py
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	set_property(TEST ${target}-py-naming
		PROPERTY LABELS ${target}-py-naming
	)
	set_tests_properties(${target}-py-naming PROPERTIES
		PASS_REGULAR_EXPRESSION "Test: 66673332"
	)
	test_environment_variables(${target}-py-naming
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)
	add_test(NAME ${target}-py-argv
		COMMAND $<TARGET_FILE:${target}> cli-test-argv.py
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	set_property(TEST ${target}-py-argv
		PROPERTY LABELS ${target}-py-argv
	)
	set_tests_properties(${target}-py-argv PROPERTIES
		PASS_REGULAR_EXPRESSION "Test: cli-test-argv.py"
	)
	test_environment_variables(${target}-py-argv
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)
	add_test(NAME ${target}-py-main
		COMMAND $<TARGET_FILE:${target}> cli-test-main.py
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	set_property(TEST ${target}-py-main
		PROPERTY LABELS ${target}-py-main
	)
	set_tests_properties(${target}-py-main PROPERTIES
		PASS_REGULAR_EXPRESSION "Test: 1234567890abcd"
	)
	test_environment_variables(${target}-py-main
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)
	add_test(NAME ${target}-py-exception # https://github.com/metacall/core/issues/261
		COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-py-exception.txt" -P ${TEST_COMMAND_RUNNER}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
	set_property(TEST ${target}-py-exception
		PROPERTY LABELS ${target}-py-exception
	)
	set_tests_properties(${target}-py-exception PROPERTIES
		PASS_REGULAR_EXPRESSION "66"
	)
	test_environment_variables(${target}-py-exception
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)
endif()

if(OPTION_BUILD_LOADERS AND OPTION_BUILD_LOADERS_TS AND OPTION_BUILD_SCRIPTS AND OPTION_BUILD_SCRIPTS_TS)
	add_test(NAME ${target}-ts
		COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-ts.txt" -P ${TEST_COMMAND_RUNNER}
		WORKING_DIRECTORY ${LOADER_SCRIPT_PATH}/typedfunc
	)
	set_property(TEST ${target}-ts
		PROPERTY LABELS ${target}-ts
	)
	set_tests_properties(${target}-ts PROPERTIES
		PASS_REGULAR_EXPRESSION "51354"
	)
	test_environment_variables(${target}-ts
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)
	add_test(NAME ${target}-tsx-templating
		COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-tsx-templating.txt" -P ${TEST_COMMAND_RUNNER}
		WORKING_DIRECTORY ${LOADER_SCRIPT_PATH}/templating
	)
	set_property(TEST ${target}-tsx-templating
		PROPERTY LABELS ${target}-tsx-templating
	)
	set_tests_properties(${target}-tsx-templating PROPERTIES
		PASS_REGULAR_EXPRESSION "Hello metaprogrammer"
	)
	test_environment_variables(${target}-tsx-templating
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)

	if(NOT (OPTION_BUILD_THREAD_SANITIZER AND OPTION_BUILD_LOADERS_CS))
		# TODO: This test fails when run with thread sanitizer (this happens when C# loader is enabled):
		#
		# WARNING: ThreadSanitizer: signal-unsafe call inside of a signal (pid=14459)
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
		#
		# For solving this, we should enable C# support for sanitizers and debug it properly
		add_test(NAME ${target}-tsx-loop-fail
			COMMAND $<TARGET_FILE:${target}> loopfail.tsx
			WORKING_DIRECTORY ${LOADER_SCRIPT_PATH}/loopfail
		)
		set_property(TEST ${target}-tsx-loop-fail
			PROPERTY LABELS ${target}-tsx-loop-fail
		)
		set_tests_properties(${target}-tsx-loop-fail PROPERTIES
			PASS_REGULAR_EXPRESSION "Error: Cannot find module 'yeet-oof/whatever'"
		)
		test_environment_variables(${target}-tsx-loop-fail
			""
			${TESTS_ENVIRONMENT_VARIABLES}
		)
	endif()

	if(OPTION_BUILD_LOADERS_PY)
		add_test(NAME ${target}-py-tsx
			COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:${target}>" -D "INPUT=${TEST_COMMAND_INPUT}-py-tsx.txt" -P ${TEST_COMMAND_RUNNER}
			WORKING_DIRECTORY ${LOADER_SCRIPT_PATH}/templating
		)
		set_property(TEST ${target}-py-tsx
			PROPERTY LABELS ${target}-py-tsx
		)
		set_tests_properties(${target}-py-tsx PROPERTIES
			PASS_REGULAR_EXPRESSION "Hello World"
		)
		test_environment_variables(${target}-py-tsx
			""
			${TESTS_ENVIRONMENT_VARIABLES}
			PROJECT_METACALL_PORTS_DIRECTORY=${CMAKE_SOURCE_DIR}/source/ports/py_port
			# Note: Here we have to set up the LOADER_SCRIPT_PATH to the CWD as a difference to ${target}-tsx-templating test
			# which does not require this apparently (due to NodeJS resolve system), probably this has to be reviewed
			LOADER_SCRIPT_PATH=${LOADER_SCRIPT_PATH}/templating
		)
	endif()
endif()
