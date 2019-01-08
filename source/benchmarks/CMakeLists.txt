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
		message(STATUS "GBench libraries not found")
		return()
	endif()
endif()

# Create interface library to link against Google Bench
add_library(GBench INTERFACE)

target_include_directories(GBench
	SYSTEM INTERFACE
	${GBENCH_INCLUDE_DIR}
)

target_link_libraries(GBench
	INTERFACE
	${GBENCH_LIBRARY}
)

add_dependencies(GBench GoogleBench)

#
# Benchmarks
#

include(CTest)

add_subdirectory(metacall_py_call_bench)
