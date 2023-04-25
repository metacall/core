#
# External dependencies
#

find_package(Python3 COMPONENTS Development)

if(NOT Python3_Development_FOUND)
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-py-c-api-bench)
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
	${source_path}/metacall_py_c_api_bench.cpp
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

	${Python_INCLUDE_DIRS} # Python includes
)

#
# Libraries
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LIBRARIES}

	GBench

	${Python_LIBRARIES} # Python libraries
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
	# ERROR: LeakSanitizer: detected memory leaks
	#
	# Direct leak of 449717 byte(s) in 224 object(s) allocated from:
	# 	#0 0x7fd0798519cf in __interceptor_malloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:69
	# 	#1 0x7fd078cf29c7  (/usr/lib/x86_64-linux-gnu/libpython3.9.so.1.0+0x1169c7)
	#
	# Indirect leak of 108494 byte(s) in 110 object(s) allocated from:
	# 	#0 0x7fd0798519cf in __interceptor_malloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:69
	# 	#1 0x7fd078cf29c7  (/usr/lib/x86_64-linux-gnu/libpython3.9.so.1.0+0x1169c7)
	#
	# SUMMARY: AddressSanitizer: 558211 byte(s) leaked in 334 allocation(s).
	#
	# For solving this, we should enable Python support for sanitizers and debug it properly
	return()
endif()

add_test(NAME ${target}
	COMMAND $<TARGET_FILE:${target}>
)

#
# Define test properties
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)
