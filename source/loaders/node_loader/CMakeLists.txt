# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE)
	return()
endif()

#
# External dependencies
#

find_package(NodeJS 10.22.0)

if(NOT NODEJS_FOUND)
	message(STATUS "NodeJS libraries not found")
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target node_loader)

# Add node loader bootstrap
add_subdirectory(bootstrap)

# Add node loader trampoline implementation
add_subdirectory(trampoline)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target}")

# Set API export file and macro
string(TOUPPER ${target} target_upper)
set(feature_file "include/${target}/${target}_features.h")
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
	#${include_path}/node_loader_port.h
)

set(sources
	${source_path}/node_loader.c
	${source_path}/node_loader_impl.cpp
	#${source_path}/node_loader_port.cpp
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

include(Distributable)

# Create unity build library
set(node_loader_unity_build "${CMAKE_CURRENT_BINARY_DIR}/${target}_unity_build.c")

distributable_generate(${target} ${node_loader_unity_build}
	${LOADER_MODULE_NAMES}
)

# Build library
add_library(${target} SHARED # Needed in order to be linked with TypeScript loader
	${node_loader_unity_build}
	${sources}
	${headers}
)

# Add target dependencies
add_dependencies(${target}
	${LOADER_MODULE_NAMES}
)

# Create namespaced alias
add_library(${META_PROJECT_NAME}::${target} ALIAS ${target})

# Export library for downstream projects
export(TARGETS ${target} NAMESPACE ${META_PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

# Create feature detection header
# Compilers: https://cmake.org/cmake/help/v3.1/variable/CMAKE_LANG_COMPILER_ID.html#variable:CMAKE_%3CLANG%3E_COMPILER_ID
# Feature: https://cmake.org/cmake/help/v3.1/prop_gbl/CMAKE_CXX_KNOWN_FEATURES.html

# Check for availability of module; use pre-generated version if not found
if (WriterCompilerDetectionHeaderFound)
	write_compiler_detection_header(
		FILE ${feature_file}
		PREFIX ${target_upper}
		COMPILERS AppleClang Clang GNU MSVC
		FEATURES cxx_alignas cxx_alignof cxx_constexpr cxx_final cxx_noexcept cxx_nullptr cxx_sizeof_member cxx_thread_local
		VERSION 3.2
	)
else()
	file(
		COPY ${PROJECT_SOURCE_DIR}/codegeneration/${target}_features.h
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/${target}
		USE_SOURCE_PERMISSIONS
	)
endif()

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

	# Set valid format for NodeJS addon
	PREFIX ""
	SUFFIX ".node"
)

#
# Include directories
#

target_include_directories(${target}
	PRIVATE
	${PROJECT_BINARY_DIR}/source/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include

	${NODEJS_INCLUDE_DIRS} # NodeJS includes

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
	${NODEJS_LIBRARY} # NodeJS library

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

target_link_libraries(${target}
	PRIVATE

	PUBLIC
	${DEFAULT_LINKER_OPTIONS}

	INTERFACE
)

#
# Symlink target
#

if(WIN32)
	set(NODE_LOADER_BINARY_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
	set(NODE_LOADER_BINARY_DIR "${CMAKE_BINARY_DIR}")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	set(NODE_LOADER_BUILD_SUFFIX "d")
else()
	set(NODE_LOADER_BUILD_SUFFIX "")
endif()

set(NODE_LOADER_NAME "${target}${NODE_LOADER_BUILD_SUFFIX}.node")
set(NODE_LOADER_LIB_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}${target}${NODE_LOADER_BUILD_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")

add_custom_target(${target}_symlink ALL
	WORKING_DIRECTORY ${NODE_LOADER_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${NODE_LOADER_NAME} ${NODE_LOADER_LIB_NAME}
)

add_dependencies(${target}_symlink ${target})

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

# Runtime (pack the runtime meanwhile NodeJS is not distributed as a library)
install(FILES
	${NODEJS_LIBRARY}
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)

# Link (lib)node_loader(d).(dll|so) to node_loader(d).node
install(CODE "execute_process(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB} COMMAND ${CMAKE_COMMAND} -E create_symlink ${NODE_LOADER_NAME} ${NODE_LOADER_LIB_NAME})")
