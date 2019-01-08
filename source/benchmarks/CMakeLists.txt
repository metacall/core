#
# Setup benchmark environment
#

# Check if benchmarks are enabled
if(NOT OPTION_BUILD_BENCHMARKS)
	return()
endif()

# DISABLE CMP0037: Target names should not be reserved and should match a validity pattern
if("${CMAKE_VERSION}" VERSION_LESS "3.11" AND POLICY CMP0037)
	set_policy(CMP0037 OLD)
endif()

include(ExternalProject)

# # Import Google Test Framework
# ExternalProject_Add(googletest
# 	GIT_REPOSITORY https://github.com/google/googletest.git
# 	GIT_TAG release-1.8.1
# 	CMAKE_ARGS -Dgmock_build_tests=OFF
# 		-Dgtest_build_samples=OFF
# 		-Dgtest_build_tests=OFF
# 		-Dgtest_disable_pthreads=${GTEST_DISABLE_PTHREADS}
# 		-Dgtest_force_shared_crt=ON
# 		-Dgtest_hide_internal_symbols=OFF
# 		-DINSTALL_GTEST=OFF
# 		-DBUILD_GMOCK=ON
# 	PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
# 	UPDATE_COMMAND "" # Disable update step
# 	INSTALL_COMMAND "" # Disable install step
# )

# # Google Test include and binary directories
# ExternalProject_Get_Property(googletest source_dir binary_dir)

# set(GTEST_INCLUDE_DIR "${source_dir}/googletest/include")
# set(GMOCK_INCLUDE_DIR "${source_dir}/googlemock/include")
# set(GTEST_LIBS_DIR "${binary_dir}/googlemock/gtest")
# set(GMOCK_LIBS_DIR "${binary_dir}/googlemock")

# # Create interface library to link against gmock
# add_library(gtest INTERFACE)

# if(MSVC)
# 	set(GTEST_LIBS_SUFFIX "lib")
# else()
# 	set(GTEST_LIBS_SUFFIX "a")
# endif()

# set(GTEST_LIBS
# 	"${GTEST_LIBS_DIR}/libgtest.${GTEST_LIBS_SUFFIX}"
# 	"${GMOCK_LIBS_DIR}/libgmock.${GTEST_LIBS_SUFFIX}"
# )

# target_include_directories(gtest
# 	SYSTEM INTERFACE
# 	${GTEST_INCLUDE_DIR}
# 	${GMOCK_INCLUDE_DIR}
# )

# target_link_libraries(gtest
# 	INTERFACE
# 	${GTEST_LIBS}
# )

# add_dependencies(gtest googletest)

#
# Benchmarks
#

include(CTest)

# add_subdirectory(python_call_benchmark_test)
# add_subdirectory(node_call_benchmark_test)
