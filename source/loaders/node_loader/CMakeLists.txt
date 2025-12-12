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

# Copy NodeJS DLL into project output directory
# TODO: https://cmake.org/cmake/help/latest/command/file.html#get-runtime-dependencies
# TODO: https://gist.github.com/micahsnyder/5d98ac8548b429309ec5a35bca9366da
if(NodeJS_LIBRARY_NAME_PATH AND WIN32)
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_OUTPUT_DIR})
	file(COPY "${NodeJS_LIBRARY_NAME_PATH}" DESTINATION ${PROJECT_OUTPUT_DIR})
endif()

# Runtime (pack NodeJS DLL in windows)
# TODO: https://cmake.org/cmake/help/latest/command/file.html#get-runtime-dependencies
# TODO: https://gist.github.com/micahsnyder/5d98ac8548b429309ec5a35bca9366da
set(NodeJS_LIBRARY_DEVELOPMENT "${NodeJS_LIBRARY}")

if(NodeJS_LIBRARY_NAME_PATH AND WIN32)
	install(FILES
		"${NodeJS_LIBRARY_NAME_PATH}"
		DESTINATION ${INSTALL_LIB}
		COMPONENT runtime
	)

	get_filename_component(NodeJS_LIBRARY_NAME "${NodeJS_LIBRARY_NAME_PATH}" NAME)
	set(NodeJS_LIBRARY_DEVELOPMENT "${NodeJS_LIBRARY_NAME_PATH}")
	set(NodeJS_LIBRARY_INSTALL "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB}/${NodeJS_LIBRARY_NAME}")
elseif(NodeJS_BUILD_FROM_SOURCE AND NOT WIN32)
	install(FILES
		"${NodeJS_LIBRARY}"
		DESTINATION ${INSTALL_LIB}
		COMPONENT runtime
	)

	get_filename_component(NodeJS_LIBRARY_NAME "${NodeJS_LIBRARY}" NAME)
	set(NodeJS_LIBRARY_INSTALL "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB}/${NodeJS_LIBRARY_NAME}")
else()
	set(NodeJS_LIBRARY_INSTALL "${NodeJS_LIBRARY}")
endif()

#
# Plugin name and options
#

# Target name
set(target node_loader)

# Add node loader bootstrap
add_subdirectory(bootstrap)

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
	${include_path}/node_loader.h
	${include_path}/node_loader_impl.h
	${include_path}/node_loader_port.h
	${include_path}/node_loader_trampoline.h
	${include_path}/node_loader_bootstrap.h
)

set(sources
	${source_path}/node_loader.c
	${source_path}/node_loader_impl.cpp
	${source_path}/node_loader_port.cpp
	${source_path}/node_loader_trampoline.cpp
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

# Add target dependencies
add_dependencies(${target}
	node_loader_bootstrap
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
	# On macOS, we use dynamic lookup to avoid loading libmetacall twice
	$<$<NOT:$<BOOL:${APPLE}>>:${META_PROJECT_NAME}::metacall>

	# Delay load for MSVC
	$<$<CXX_COMPILER_ID:MSVC>:${NodeJS_LIBRARY}> # NodeJS library
	$<$<CXX_COMPILER_ID:MSVC>:delayimp>

	PUBLIC
	${DEFAULT_LIBRARIES}

	INTERFACE
)

#
# Compile definitions
#

target_compile_definitions(${target}
	PRIVATE
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
	# On macOS, we use dynamic lookup for using existing symbols
	$<$<AND:$<BOOL:${APPLE}>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-Wl,-undefined,dynamic_lookup>

	# Delay load for MSVC
	$<$<CXX_COMPILER_ID:MSVC>:/DELAYLOAD:${NodeJS_LIBRARY_NAME}>

	PUBLIC
	${DEFAULT_LINKER_OPTIONS}

	INTERFACE
)

#
# Standard options
#

if(NodeJS_VERSION VERSION_GREATER_EQUAL "16.0.0")
	set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
endif()

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

#
# Configuration
#

# Development
loader_configuration_begin(node_loader)
loader_configuration_deps(node "${NodeJS_LIBRARY_DEVELOPMENT}")
loader_configuartion_end_development()

# Install
loader_configuration_begin(node_loader)
loader_configuration_deps(node "${NodeJS_LIBRARY_INSTALL}")
loader_configuartion_end_install()
