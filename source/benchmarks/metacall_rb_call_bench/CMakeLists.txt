# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_RB)
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-rb-call-bench)
message(STATUS "Benchmark ${target}")

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
	${source_path}/metacall_rb_call_bench.cpp
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
	${DEFAULT_INCLUDE_DIRECTORIES}
	${PROJECT_BINARY_DIR}/source/include
)

#
# Libraries
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LIBRARIES}

	GBench

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
# Linker options
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

#
# Define test
#

if(OPTION_BUILD_SANITIZER)
	# TODO: This test fails when run with sanitizers:
	# ERROR: AddressSanitizer: heap-use-after-free on address 0x629000003a50 at pc 0x7fb5059be061 bp 0x6290000032d0 sp 0x629000002a80
	# WRITE of size 22 at 0x629000003a50 thread T0
	#
	# For solving this, we should enable Ruby support for sanitizers and debug it properly
	return()
endif()

add_test(NAME ${target}
	COMMAND $<TARGET_FILE:${target}>
)

#
# Define dependencies
#

add_dependencies(${target}
	rb_loader
)

#
# Define test properties
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)
