#
# Executable name and options
#

# Target name
set(target dynlink-test)
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
	${source_path}/dynlink_test.cpp
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

	${META_PROJECT_NAME}::version
	${META_PROJECT_NAME}::preprocessor
	${META_PROJECT_NAME}::environment
	${META_PROJECT_NAME}::format
	${META_PROJECT_NAME}::threading
	${META_PROJECT_NAME}::log
	${META_PROJECT_NAME}::portability
	${META_PROJECT_NAME}::dynlink
)

#
# Compile definitions
#

target_compile_definitions(${target}
	PRIVATE
	${DEFAULT_COMPILE_DEFINITIONS}

	$<$<AND:$<BOOL:${OPTION_BUILD_LOADERS}>,$<BOOL:${OPTION_BUILD_LOADERS_MOCK}>>:DYNLINK_TEST_MOCK_LOADER>
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

	$<$<AND:$<BOOL:${APPLE}>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-Wl,-export_dynamic>
	$<$<CXX_COMPILER_ID:GNU>:-rdynamic>
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

if(OPTION_BUILD_LOADERS AND OPTION_BUILD_LOADERS_MOCK)
	add_dependencies(${target}
		mock_loader
	)
endif()

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	"DYNLINK_TEST_LIBRARY_PATH=@OUTPUT_DIRECTORY_DIR@"
)
