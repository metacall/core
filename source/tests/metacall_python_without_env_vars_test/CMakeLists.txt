# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY)
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-python-without-env-vars-test)
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
	${source_path}/metacall_python_without_env_vars_test.cpp
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
# Linker options
#

add_link_options(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

if(OPTION_BUILD_ADDRESS_SANITIZER OR OPTION_BUILD_THREAD_SANITIZER)
	# TODO: This test leaks and I am not sure if it is a false positive or not:
	#
	# Direct leak of 18682 byte(s) in 12 object(s) allocated from:
	#     #0 0x7fa978386bd7 in malloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:69
	#     #1 0x7fa9734ddbb1 in _PyMem_RawMalloc ../Objects/obmalloc.c:101

	# Indirect leak of 2775 byte(s) in 2 object(s) allocated from:
	#     #0 0x7fa978386bd7 in malloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:69
	#     #1 0x7fa9734ddbb1 in _PyMem_RawMalloc ../Objects/obmalloc.c:101

	# SUMMARY: AddressSanitizer: 21457 byte(s) leaked in 14 allocation(s).
	#
	# Valgrind does not show anything:
	# valgrind --tool=memcheck --leak-check=full --show-leak-kinds=possibly --track-origins=yes --num-callers=500 --suppressions=../source/tests/memcheck/valgrind-python.supp ./metacall-python-without-env-vars-testd
	# For checking all the leaks (and including false positives), check:
	# valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --num-callers=500 --suppressions=../source/tests/memcheck/valgrind-python.supp ./metacall-python-without-env-vars-testd
	return()
endif()

#
# Define test
#

add_test(NAME ${target}
	COMMAND $<TARGET_FILE:${target}>
)

#
# Define dependencies
#

add_dependencies(${target}
	py_loader
)

#
# Define test properties
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)
