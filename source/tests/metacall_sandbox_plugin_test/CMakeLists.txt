# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_EXT OR NOT OPTION_BUILD_EXTENSIONS OR NOT OPTION_BUILD_PLUGINS_SANDBOX)
	return()
endif()

include(Portability)

# Sandbox is only supported in Linux
if(NOT PROJECT_OS_FAMILY STREQUAL unix)
	return()
endif()

# Thread Sanitizer deadlocks with ASSERT_EXIT (TODO: Review this when fork safety is implemented in sandbox_plugin)
if(OPTION_BUILD_THREAD_SANITIZER)
  return()
endif()

# TODO: Test if we can make it run properly with use_sigaltstack=1 (https://groups.google.com/a/chromium.org/g/chromium-reviews/c/xX4xDy21714/m/1yKX1JLE620J)
if(OPTION_BUILD_ADDRESS_SANITIZER)
	return()
endif()

#
# Executable name and options
#

# Target name
set(target metacall-sandbox-plugin-test)
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
	${source_path}/metacall_sandbox_plugin_test.cpp
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
# Define test
#

add_test(NAME ${target}
	COMMAND $<TARGET_FILE:${target}>
)

#
# Define dependencies
#

add_dependencies(${target}
	ext_loader
	sandbox_plugin
)

#
# Define test properties
#

set_property(TEST ${target}
	# Valgrind does not work well with libseccomp
	PROPERTY LABELS ${target} MEMCHECK_IGNORE
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)
