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
# Tests
#

add_subdirectory(preprocessor_test)
add_subdirectory(environment_test)
add_subdirectory(log_test)
add_subdirectory(adt_hash_map_test)
add_subdirectory(adt_trie_test)
add_subdirectory(reflect_value_cast_test)
add_subdirectory(reflect_function_test)
add_subdirectory(reflect_scope_test)
add_subdirectory(reflect_metadata_test)
add_subdirectory(serial_test)
add_subdirectory(configuration_test)
add_subdirectory(dynlink_test)
add_subdirectory(py_loader_test)
add_subdirectory(py_django_integration_test)
add_subdirectory(py_loader_port_test)
add_subdirectory(rb_loader_test)
add_subdirectory(rb_loader_parser_test)
add_subdirectory(rb_loader_parser_integration_test)
#add_subdirectory(rb_rails_integration_test) # TODO
add_subdirectory(cs_loader_test)
add_subdirectory(metacall_load_memory_test)
add_subdirectory(metacall_load_configuration_test)
add_subdirectory(metacall_duplicated_handle_test)
add_subdirectory(metacall_test)
add_subdirectory(metacall_distributable_test)
add_subdirectory(metacall_cast_test)
add_subdirectory(metacall_init_fini_test)
add_subdirectory(metacall_ducktype_test)
add_subdirectory(metacall_inspect_test)
add_subdirectory(metacall_integration_test)
add_subdirectory(metacall_depends_test)
add_subdirectory(metacall_configuration_exec_path_test)
add_subdirectory(metacall_clear_test)
add_subdirectory(metacall_python_gc)
add_subdirectory(metacall_map_test)
add_subdirectory(metacall_python_class_test)
