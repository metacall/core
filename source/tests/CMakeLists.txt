#
# Setup test environment
#

# Check if tests are enabled
if(NOT OPTION_BUILD_TESTS)
	return()
endif()

# DISABLE CMP0037: Target names should not be reserved and should match a validity pattern.
set_policy(CMP0037 OLD)

# Build gmock
set(gmock_build_tests			OFF CACHE BOOL "")
set(gtest_build_samples			OFF CACHE BOOL "")
set(gtest_build_tests			OFF CACHE BOOL "")
set(gtest_disable_pthreads		OFF CACHE BOOL "")
set(gtest_force_shared_crt		ON  CACHE BOOL "")
set(gtest_hide_internal_symbols	OFF CACHE BOOL "")

add_subdirectory(googletest/googlemock)

# Create interface library to link against gmock
add_library(gmock-dev INTERFACE)

target_include_directories(gmock-dev
	SYSTEM INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include
	${CMAKE_CURRENT_SOURCE_DIR}/googletest/googlemock/include
)

target_link_libraries(gmock-dev
	INTERFACE
	gmock
)

#
# Configure variables
#

set(TESTS_LOADER_ENVIRONMENT_VARIABLES
	"LOADER_LIBRARY_PATH=${LOADER_LIBRARY_PATH};LOADER_SCRIPT_PATH=${LOADER_SCRIPT_PATH}"
)

set(TESTS_CONFIGURATION_ENVIRONMENT_VARIABLES
	"CONFIGURATION_PATH=${CONFIGURATION_PATH};CONFIGURATION_LIBRARY_PATH=${CONFIGURATION_LIBRARY_PATH}"
)

set(TESTS_ENVIRONMENT_VARIABLES
	"${TESTS_LOADER_ENVIRONMENT_VARIABLES};${TESTS_CONFIGURATION_ENVIRONMENT_VARIABLES}"
)

#
# Tests
#

add_subdirectory(preprocessor_test)
add_subdirectory(log_test)
add_subdirectory(hash_map_test)
add_subdirectory(trie_test)
add_subdirectory(value_stringify_test)
add_subdirectory(value_cast_test)
add_subdirectory(function_test)
add_subdirectory(configuration_test)
add_subdirectory(dyncall_test)
add_subdirectory(py_loader_test)
add_subdirectory(rb_loader_test)
add_subdirectory(rb_loader_parser_test)
add_subdirectory(cs_loader_test)
add_subdirectory(metacall_load_memory_test)
add_subdirectory(metacall_test)
add_subdirectory(metacall_distributable_test)
add_subdirectory(metacall_cast_test)
add_subdirectory(integration_test)
