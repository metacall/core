# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_NODE)
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
		set(OPTION_BUILD_PORTS_NODE OFF CACHE BOOL)
		return()
	endif()

	add_custom_target(${target} ALL
		COMMAND ${NPM_EXECUTABLE} install --ignore-scripts
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/index.js ${PROJECT_OUTPUT_DIR}/node_modules/metacall/index.js
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/package.json ${PROJECT_OUTPUT_DIR}/node_modules/metacall/package.json
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/package-lock.json ${PROJECT_OUTPUT_DIR}/node_modules/metacall/package-lock.json
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		SOURCES ${sources}
	)
else()
	add_custom_target(${target} ALL
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/index.js ${PROJECT_OUTPUT_DIR}/node_modules/metacall/index.js
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
if(NOT OPTION_BUILD_CLI OR NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_RB OR NOT OPTION_BUILD_LOADERS_TS OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_SCRIPTS_RB)
	return()
endif()

set(node_port_test "${target}_test")

#
# Define test
#

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

add_test(NAME ${target}
	COMMAND ${CMAKE_COMMAND} -D "EXECUTABLE=$<TARGET_FILE:metacallcli>" -D "INPUT=${CMAKE_CURRENT_SOURCE_DIR}/test/commands/node_port.txt" -P "${CMAKE_SOURCE_DIR}/source/cli/metacallcli/test/commands/command_runner.cmake"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${node_port_test}
)

set_tests_properties(${target} PROPERTIES
	PASS_REGULAR_EXPRESSION "Tests passed without errors"
)

include(TestEnvironmentVariables)

# Enable cobol test if it is built
if(OPTION_BUILD_LOADERS_COB)
	set(TESTS_ENVIRONMENT_VARIABLES_COB "OPTION_BUILD_LOADERS_COB=1")
endif()

# Enable c test if it is built
if(OPTION_BUILD_LOADERS_C)
	set(TESTS_ENVIRONMENT_VARIABLES_C "OPTION_BUILD_LOADERS_C=1")
endif()

# Enable rust test if it is built
if(OPTION_BUILD_LOADERS_RS)
	set(TESTS_ENVIRONMENT_VARIABLES_RS "OPTION_BUILD_LOADERS_RS=1")
endif()

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
	${TESTS_ENVIRONMENT_VARIABLES_COB}
	${TESTS_ENVIRONMENT_VARIABLES_C}
	${TESTS_ENVIRONMENT_VARIABLES_RS}
)
