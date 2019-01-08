#
# Setup test environment
#

# Check if tests are enabled
if(NOT OPTION_BUILD_TESTS)
	return()
endif()

# DISABLE CMP0037: Target names should not be reserved and should match a validity pattern
if("${CMAKE_VERSION}" VERSION_LESS "3.11" AND POLICY CMP0037)
	set_policy(CMP0037 OLD)
endif()

include(ExternalProject)

if(MINGW)
	set(GTEST_DISABLE_PTHREADS ON)
else()
	set(GTEST_DISABLE_PTHREADS OFF)
endif()

# Import Google Test Framework
ExternalProject_Add(googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG release-1.8.1
	CMAKE_ARGS -Dgmock_build_tests=OFF
		-Dgtest_build_samples=OFF
		-Dgtest_build_tests=OFF
		-Dgtest_disable_pthreads=${GTEST_DISABLE_PTHREADS}
		-Dgtest_force_shared_crt=ON
		-Dgtest_hide_internal_symbols=OFF
		-DINSTALL_GTEST=OFF
		-DBUILD_GMOCK=ON
	PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
	UPDATE_COMMAND "" # Disable update step
	INSTALL_COMMAND "" # Disable install step
)

# Google Test include and binary directories
ExternalProject_Get_Property(googletest source_dir binary_dir)

set(GTEST_INCLUDE_DIR "${source_dir}/googletest/include")
set(GMOCK_INCLUDE_DIR "${source_dir}/googlemock/include")
set(GTEST_LIBS_DIR "${binary_dir}/googlemock/gtest")
set(GMOCK_LIBS_DIR "${binary_dir}/googlemock")

# Create interface library to link against gmock
add_library(gmock-dev INTERFACE)

if(MSVC)
	set(GTEST_LIBS_SUFFIX "lib")
else()
	set(GTEST_LIBS_SUFFIX "a")
endif()

set(GTEST_LIBS
	"${GTEST_LIBS_DIR}/libgtest.${GTEST_LIBS_SUFFIX}"
	"${GMOCK_LIBS_DIR}/libgmock.${GTEST_LIBS_SUFFIX}"
)

target_include_directories(gmock-dev
	SYSTEM INTERFACE
	${GTEST_INCLUDE_DIR}
	${GMOCK_INCLUDE_DIR}
)

target_link_libraries(gmock-dev
	INTERFACE
	${GTEST_LIBS}
)

add_dependencies(gmock-dev googletest)

# Set memory check configuration
option(OPTION_TEST_MEMORYCHECK "Run tests with memory checker (valgrind)." OFF)

if(OPTION_TEST_MEMORYCHECK)
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --leak-check=full")
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --trace-children=yes")
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --show-reachable=yes")
	set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --num-callers=50")

	set(MEMORYCHECK_SUPPRESSIONS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/memcheck/valgrind.supp")

	# TODO: Memory check does not work properly with CoreCLR
	#
	# 	Remove MEMCHECK_IGNORE label from the following tests:
	#		- cs-loader-test
	#		- metacall-test
	#		- metacall-distributable-test
	#		- metacall-inspect-test
	#		- metacall-integration-test

	add_custom_target(memcheck
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMAND ${CMAKE_CTEST_COMMAND}
			--label-exclude MEMCHECK_IGNORE
			--force-new-ctest-process
			--test-action memcheck
			--timeout 60
		#COMMAND cat "${CMAKE_BINARY_DIR}/Testing/Temporary/MemoryChecker.*.log"
	)
endif()

#
# Tests
#

include(CTest)

add_subdirectory(preprocessor_test)
add_subdirectory(environment_test)
add_subdirectory(log_test)
add_subdirectory(adt_set_test)
add_subdirectory(adt_trie_test)
add_subdirectory(reflect_value_cast_test)
add_subdirectory(reflect_function_test)
add_subdirectory(reflect_scope_test)
add_subdirectory(reflect_metadata_test)
add_subdirectory(dynlink_test)
add_subdirectory(detour_test)
add_subdirectory(serial_test)
add_subdirectory(configuration_test)
add_subdirectory(py_loader_test)
add_subdirectory(py_django_integration_test)
add_subdirectory(py_loader_port_test)
add_subdirectory(rb_loader_test)
add_subdirectory(rb_loader_parser_test)
add_subdirectory(rb_loader_parser_integration_test)
#add_subdirectory(rb_rails_integration_test) # TODO
add_subdirectory(cs_loader_test)
add_subdirectory(node_loader_test)
add_subdirectory(file_loader_test)
add_subdirectory(loader_path_test)
add_subdirectory(metacall_load_memory_test)
add_subdirectory(metacall_load_configuration_test)
add_subdirectory(metacall_load_configuration_relative_test)
add_subdirectory(metacall_duplicated_handle_test)
add_subdirectory(metacall_test)
add_subdirectory(metacall_node_test)
add_subdirectory(metacall_node_call_test)
add_subdirectory(metacall_distributable_test)
add_subdirectory(metacall_cast_test)
add_subdirectory(metacall_init_fini_test)
add_subdirectory(metacall_ducktype_test)
add_subdirectory(metacall_inspect_test)
add_subdirectory(metacall_integration_test)
add_subdirectory(metacall_depends_test)
add_subdirectory(metacall_configuration_exec_path_test)
add_subdirectory(metacall_clear_test)
add_subdirectory(metacall_python_gc_test)
add_subdirectory(metacall_map_test)
add_subdirectory(metacall_python_class_test)
add_subdirectory(metacall_initialize_test)
add_subdirectory(metacall_initialize_ex_test)
add_subdirectory(metacall_reinitialize_test)
add_subdirectory(metacall_fork_test)
