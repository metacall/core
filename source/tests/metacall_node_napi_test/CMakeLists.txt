# Check if this regression test is applicable
if(NOT WIN32 OR NOT MSVC)
	return()
endif()

if(NOT OPTION_BUILD_CLI OR NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_NODE)
	return()
endif()

if(OPTION_BUILD_GUIX)
	return()
endif()

#
# Test name and dependencies
#

set(target metacall-node-napi-test)
message(STATUS "Test ${target}")

find_package(NPM)

if(NOT NPM_FOUND)
	message(SEND_ERROR "NPM not found")
	return()
endif()

include(TestEnvironmentVariables)

set(node_napi_test_path "${CMAKE_CURRENT_BINARY_DIR}/node_napi")

file(MAKE_DIRECTORY "${node_napi_test_path}")
configure_file(data/node_napi/index.js "${node_napi_test_path}/index.js" COPYONLY)
configure_file(data/node_napi/commands.txt "${node_napi_test_path}/commands.txt" COPYONLY)

add_test(NAME ${target}
	COMMAND ${CMAKE_COMMAND}
		-D "EXECUTABLE=$<TARGET_FILE:metacallcli>"
		-D "TEST_DIR=${node_napi_test_path}"
		-D "NPM_EXECUTABLE=${NPM_EXECUTABLE}"
		-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/run_test.cmake
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

set_tests_properties(${target} PROPERTIES
	PASS_REGULAR_EXPRESSION "rspack loaded, version:"
	FAIL_REGULAR_EXPRESSION "Node-API symbol .* has not been loaded;Failed to call the function, function 'check' does not exist;TypeError: Cannot read properties of undefined"
	TIMEOUT 1800
)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
	"LOADER_SCRIPT_PATH=${node_napi_test_path}"
)
