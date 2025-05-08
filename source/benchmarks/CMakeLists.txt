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

find_package(GBench)

if(NOT GBENCH_FOUND)
	include(InstallGBench)

	if(NOT GBENCH_FOUND)
		message(SEND_ERROR "GBench libraries not found")
		return()
	endif()

	set(GBENCH_INSTALL TRUE)
endif()

# Create interface library to link against Google Bench
add_library(GBench INTERFACE)

target_include_directories(GBench
	SYSTEM INTERFACE
	${GBENCH_INCLUDE_DIR}
)

if(MSVC)
	set(DEFAULT_GBENCH_OPTIONS Shlwapi.lib)
else()
	set(DEFAULT_GBENCH_OPTIONS)
endif()

target_link_libraries(GBench
	INTERFACE
	${GBENCH_LIBRARY}
	${DEFAULT_GBENCH_OPTIONS}
)

if(GBENCH_INSTALL)
	add_dependencies(GBench google-bench-depends)
endif()

# Create output directory
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/benchmarks)

#
# Benchmarks
#

include(CTest)

add_subdirectory(set_bench)
add_subdirectory(log_bench)
add_subdirectory(metacall_py_c_api_bench)
add_subdirectory(metacall_py_call_bench)
add_subdirectory(metacall_py_init_bench)
add_subdirectory(metacall_node_call_bench)
add_subdirectory(metacall_rb_call_bench)
add_subdirectory(metacall_cs_call_bench)
