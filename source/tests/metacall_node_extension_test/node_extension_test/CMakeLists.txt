# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE)
	return()
endif()

#
# External dependencies
#

find_package(NodeJS)

if(NOT NodeJS_FOUND)
	message(SEND_ERROR "NodeJS libraries not found")
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target node_extension_test)

# Exit here if required dependencies are not met
message(STATUS "Script ${target}")

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

set(sources
	${source_path}/node_extension_test.c
)

if(WIN32 AND MSVC_VERSION GREATER_EQUAL 1200)
	set(sources
		${sources}
		${source_path}/node_extension_test_win32_delay_load.cpp
	)
endif()

# Group source files
set(source_group "Source Files")
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

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "Scripts" # Put it on scripts even if it is on tests folder
	BUNDLE $<$<BOOL:${APPLE}>:$<$<VERSION_GREATER:${PROJECT_OS_VERSION},8>>>

	# Set NodeJS extension properies
	SUFFIX ".node"
	PREFIX ""
	DEBUG_POSTFIX ""
)

#
# Include directories
#

target_include_directories(${target}
	PRIVATE
	${PROJECT_BINARY_DIR}/source/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include

	${NodeJS_INCLUDE_DIRS} # NodeJS includes

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
	${NodeJS_LIBRARY} # NodeJS library
	$<$<BOOL:${MSVC}>:delayimp.lib> # Delayed library

	PUBLIC
	${DEFAULT_LIBRARIES}

	INTERFACE
)

#
# Compile definitions
#

target_compile_definitions(${target}
	PRIVATE
	$<$<BOOL:${MSVC}>:NODEJS_LIBRARY_NAME="${NodeJS_LIBRARY_NAME}">
	$<$<NOT:$<BOOL:${MSVC}>>:_LARGEFILE_SOURCE>
	$<$<NOT:$<BOOL:${MSVC}>>:_FILE_OFFSET_BITS=64>
	$<$<AND:$<BOOL:${APPLE}>,$<CXX_COMPILER_ID:AppleClang,Clang>>:_DARWIN_USE_64_BIT_INODE=1>

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
	$<$<CXX_COMPILER_ID:MSVC>:/IGNORE:4199>
	$<$<CXX_COMPILER_ID:MSVC>:/DELAYLOAD:${NodeJS_LIBRARY_NAME}>
	$<$<CXX_COMPILER_ID:MSVC>:/DELAYLOAD:${NodeJS_LIBRARY_NAME}>
	$<$<AND:$<BOOL:${APPLE}>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-undefined dynamic_lookup>

	PUBLIC
	${DEFAULT_LINKER_OPTIONS}

	INTERFACE
)
