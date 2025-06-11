# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_EXT OR NOT OPTION_BUILD_EXTENSIONS OR NOT OPTION_BUILD_PLUGINS_BACKTRACE)
	return()
endif()

#
# External dependencies
#

if(NOT (OPTION_BUILD_GUIX OR BackwardCpp_SOURCE))
	include(FetchContent)

	FetchContent_Declare(BackwardCpp
		GIT_REPOSITORY https://github.com/metacall/backward-cpp
		GIT_TAG 0bfd0a07a61551413ccd2ab9a9099af3bad40681
	)

	FetchContent_MakeAvailable(BackwardCpp)

	FetchContent_GetProperties(BackwardCpp
		SOURCE_DIR BackwardCpp_SOURCE
		POPULATED BackwardCpp_POPULATED
	)

	if(NOT BackwardCpp_POPULATED)
		FetchContent_Populate(BackwardCpp)
	endif()
endif()

if(NOT BackwardCpp_POPULATED OR NOT BackwardCpp_SOURCE)
	message(STATUS "BackwardCpp could not be installed, trying to find it on the system")
endif()

find_package(Backward
	CONFIG
	PATHS ${BackwardCpp_SOURCE}
)

if(NOT BACKWARD_FOUND)
	message(WARNING "BackwardCpp could not be found, skipping backtrace plugin compilation")
	return()
endif()

include(${BackwardCpp_SOURCE}/BackwardConfig.cmake)

#
# Plugin name and options
#

# Target name
set(target backtrace_plugin)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target}")

# Set API export file and macro
string(TOUPPER ${target} target_upper)
set(export_file  "include/${target}/${target}_api.h")
set(export_macro "${target_upper}_API")

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

set(headers
	${include_path}/backtrace_plugin.h
)

set(sources
	${source_path}/backtrace_plugin.cpp
)

# Group source files
set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$"
	${header_group} ${headers})
source_group_by_path(${source_path}  "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$"
	${source_group} ${sources})

#
# Create library
#

# Build library
add_library(${target} MODULE
	${sources}
	${headers}
)

# Create namespaced alias
add_library(${META_PROJECT_NAME}::${target} ALIAS ${target})

# Export library for downstream projects
export(TARGETS ${target} NAMESPACE ${META_PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

# Create API export header
generate_export_header(${target}
	EXPORT_FILE_NAME  ${export_file}
	EXPORT_MACRO_NAME ${export_macro}
)

#
# Project options
#

set(PLUGIN_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIR}/plugins/${target}")

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
	BUNDLE $<$<BOOL:${APPLE}>:$<$<VERSION_GREATER:${PROJECT_OS_VERSION},8>>>

	# Define custom build output directory
	LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIRECTORY}"
	LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PLUGIN_OUTPUT_DIRECTORY}"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PLUGIN_OUTPUT_DIRECTORY}"
	LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PLUGIN_OUTPUT_DIRECTORY}"
	LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${PLUGIN_OUTPUT_DIRECTORY}"

	RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIRECTORY}"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PLUGIN_OUTPUT_DIRECTORY}"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PLUGIN_OUTPUT_DIRECTORY}"
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PLUGIN_OUTPUT_DIRECTORY}"
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${PLUGIN_OUTPUT_DIRECTORY}"

	ARCHIVE_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIRECTORY}"
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PLUGIN_OUTPUT_DIRECTORY}"
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${PLUGIN_OUTPUT_DIRECTORY}"
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PLUGIN_OUTPUT_DIRECTORY}"
	ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${PLUGIN_OUTPUT_DIRECTORY}"
)

#
# Include directories
#

target_include_directories(${target}
	PRIVATE
	${PROJECT_BINARY_DIR}/source/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include

	$<TARGET_PROPERTY:${META_PROJECT_NAME}::metacall,INCLUDE_DIRECTORIES> # MetaCall includes

	PUBLIC
	${DEFAULT_INCLUDE_DIRECTORIES}

	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

#
# Libraries
#

target_link_libraries(${target}
	PRIVATE
	${META_PROJECT_NAME}::metacall # MetaCall library

	Backward::Backward # Backward-cpp library

	PUBLIC
	${DEFAULT_LIBRARIES}

	INTERFACE
)

#
# Compile definitions
#

target_compile_definitions(${target}
	PRIVATE

	PUBLIC
	$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${target_upper}_STATIC_DEFINE>
	${DEFAULT_COMPILE_DEFINITIONS}

	INTERFACE
)

#
# Compile options
#

target_compile_options(${target}
	PRIVATE

	PUBLIC
	${DEFAULT_COMPILE_OPTIONS}

	INTERFACE
)

#
# Linker options
#

target_link_options(${target}
	PRIVATE

	PUBLIC
	${DEFAULT_LINKER_OPTIONS}

	INTERFACE
)

#
# Define dependencies
#

# Copy metacall.json
add_custom_target(${target}_config ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${PLUGIN_OUTPUT_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E copy ${source_path}/metacall.json ${PLUGIN_OUTPUT_DIRECTORY}/metacall.json
)

set_target_properties(${target}_config
	PROPERTIES
	FOLDER "${IDE_FOLDER}"
)

add_dependencies(${target}
	${target}_config
	plugin_extension
)
