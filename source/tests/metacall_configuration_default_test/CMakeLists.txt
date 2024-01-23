#
# Executable name and options
#

# Target name
set(target metacall-configuration-default-test)
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
	${source_path}/metacall_configuration_default_test.cpp
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

# Create a custom directory different from default for forcing metacall not to find the configuration
set(CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIR}/configuration-default-test")

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY})

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"

	# Define custom build output directory
	LIBRARY_OUTPUT_DIRECTORY "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"
	LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"
	LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"
	LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"

	RUNTIME_OUTPUT_DIRECTORY "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"

	ARCHIVE_OUTPUT_DIRECTORY "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"
	ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}"
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

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

#
# Define test
#

add_test(NAME ${target}
	COMMAND $<TARGET_FILE:${target}>
)

if(WIN32)
	# Windows requires metacall to be in the same folder as the executable in order to run the test properly
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metacall> ${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rapid_json_serial> ${CONFIGURATION_DEFAULT_OUTPUT_DIRECTORY}
	)
endif()

#
# Define test properties
#

set_property(TEST ${target}
	PROPERTY LABELS ${target}
)
