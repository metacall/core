# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_RPC)
	return()
endif()

#
# External dependencies
#

find_package(CURL REQUIRED)

# Copy cURL DLL into project output directory
# TODO: https://cmake.org/cmake/help/latest/command/file.html#get-runtime-dependencies
# TODO: https://gist.github.com/micahsnyder/5d98ac8548b429309ec5a35bca9366da
if(WIN32 AND CURL_LIBRARY_NAME)
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_OUTPUT_DIR})
	file(COPY "${CURL_LIBRARY_NAME}" DESTINATION ${PROJECT_OUTPUT_DIR})
endif()

#
# Plugin name and options
#

# Target name
set(target rpc_loader)

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
	${include_path}/rpc_loader.h
	${include_path}/rpc_loader_impl.h
)

set(sources
	${source_path}/rpc_loader.c
	${source_path}/rpc_loader_impl.cpp
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

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
	BUNDLE $<$<BOOL:${APPLE}>:$<$<VERSION_GREATER:${PROJECT_OS_VERSION},8>>>
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
	${CURL_INCLUDE_DIRS} # cURL includes

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

	${CURL_LIBRARIES} # cURL libraries

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
# Deployment
#

# Library
install(TARGETS ${target}
	EXPORT  "${target}-export"				COMPONENT dev
	RUNTIME DESTINATION ${INSTALL_BIN}		COMPONENT runtime
	LIBRARY DESTINATION ${INSTALL_SHARED}	COMPONENT runtime
	ARCHIVE DESTINATION ${INSTALL_LIB}		COMPONENT dev
)

# Runtime (pack cURL DLL in windows)
# TODO: https://cmake.org/cmake/help/latest/command/file.html#get-runtime-dependencies
# TODO: https://gist.github.com/micahsnyder/5d98ac8548b429309ec5a35bca9366da
if(WIN32 AND CURL_LIBRARY_NAME)
	install(FILES
		"${CURL_LIBRARY_NAME}"
		DESTINATION ${INSTALL_LIB}
		COMPONENT runtime
	)
endif()
