# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_CS OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_CS)
	return()
endif()

#
# External dependencies
#

find_package(DotNET)

if(NOT DOTNET_FOUND)
	message(SEND_ERROR "DotNET command not found")
	return()
endif()

if(DOTNET_VERSION VERSION_LESS "5.0")
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-csharp-function-test)
message(STATUS "Test ${target}")

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
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(sources
	${source_path}/main.cpp
	${source_path}/metacall_csharp_function_test.cpp
	${source_path}/metacall_csharp_metadata_test.cpp
	${source_path}/metacall_csharp_native_metadata_test.cpp
)

# Group source files
set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$"
	${header_group} ${headers})
source_group_by_path(${source_path}  "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$"
	${source_group} ${sources})

#
# Create executable
#

# Build executable
add_executable(${target}
	${sources}
)

# Compile the production bridge for native delegate fault-injection tests.
target_sources(${target}
	PRIVATE
	${PROJECT_SOURCE_DIR}/source/loaders/cs_loader/source/netcore.cpp
)

# Create namespaced alias
add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})

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
	${PROJECT_SOURCE_DIR}/source/loaders/cs_loader/include
	${DEFAULT_INCLUDE_DIRECTORIES}
	${PROJECT_BINARY_DIR}/source/include
)

#
# Libraries
#

target_link_libraries(${target}
	PRIVATE
	${META_PROJECT_NAME}::log
	${DEFAULT_LIBRARIES}

	GTest

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
	cxx_std_17
)

#
# Linker options
#

target_link_options(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

#
# Define dependencies
#

add_dependencies(${target}
	cs_loader
)

#
# Define test
#

if(OPTION_BUILD_THREAD_SANITIZER)
	check_tsan_executable("${DOTNET_CORE_LIBRARY}" DotNET_TSAN)
	if(NOT DotNET_TSAN)
		# This test fails when run with thread sanitizer due to C# when CoreCLR is not compiled with TSAN:
		# coreclr_initialize status (0x8007ff0b)
		# For solving this, we should enable C# support for sanitizers and debug it properly
		return()
	endif()
endif()

if(OPTION_TEST_MEMORYCHECK)
	# C# NetCore instrumentation for Valgrind is not supported right now on MetaCall
	# and we do not have a way to check if is instrumented or not checking the library
	# so we skip directly the test to avoid false positives
	return()
endif()

# CoreCLR must not be shut down and restarted between cases in one process.
# Register each case separately while preserving the test environment.
include(TestEnvironmentVariables)

set(function_test_cases
	metacall_csharp_function_test.DefaultConstructor
	metacall_csharp_function_test.MoreThan100Functions
	metacall_csharp_function_test.FunctionMetadataSurvivesRepeatedLoad
	metacall_csharp_metadata_test.ManagedMetadataCapacityContract
	metacall_csharp_metadata_test.ManagedNativeMetadataLayoutsMatch
	metacall_csharp_metadata_test.ManagedMetadataEmptyAndFailureContract
	metacall_csharp_native_metadata_test.NativeMetadataRetryAndFailureContract
)

foreach(test_case IN LISTS function_test_cases)
	string(REGEX REPLACE "^.*\\." "" case_name "${test_case}")
	set(test_name "${target}-${case_name}")

	add_test(NAME ${test_name}
		COMMAND $<TARGET_FILE:${target}>
			"--gtest_filter=${test_case}"
	)

	set_property(TEST ${test_name}
		PROPERTY LABELS ${target} MEMCHECK_IGNORE
	)

	test_environment_variables(${test_name}
		""
		${TESTS_ENVIRONMENT_VARIABLES}
	)
endforeach()
