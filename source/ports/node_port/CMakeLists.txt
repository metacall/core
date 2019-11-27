# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_NODE)
	return()
endif()

#
# External dependencies
#

set(NODEJS_EXECUTABLE_ONLY ON)

find_package(NodeJS)

if(NOT NODEJS_FOUND)
	message(STATUS "NodeJS libraries not found")
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target node_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

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
	${include_path}/node_port.h
)

set(sources
	${source_path}/node_port.cpp
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
	${META_PROJECT_NAME}::metacall_distributable
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
	SUFFIX ".node"
)

#
# Include directories
#

execute_process(
	COMMAND npm install
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

execute_process(
	COMMAND node -p "require('node-addon-api').include"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE NODE_ADDON_API_INCLUDE_DIR
)

string(REPLACE "\n" "" NODE_ADDON_API_INCLUDE_DIR ${NODE_ADDON_API_INCLUDE_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_INCLUDE_DIR ${NODE_ADDON_API_INCLUDE_DIR})

target_include_directories(${target}
	PRIVATE
	${PROJECT_BINARY_DIR}/source/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include

	${NODE_ADDON_API_INCLUDE_DIR} # N-API wrappers

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
	${META_PROJECT_NAME}::metacall_distributable # MetaCall Library

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
# Deployment
#

# Library
install(TARGETS ${target}
	EXPORT  "${target}-export"				COMPONENT dev
	RUNTIME DESTINATION ${INSTALL_BIN}		COMPONENT runtime
	LIBRARY DESTINATION ${INSTALL_SHARED}	COMPONENT runtime
	ARCHIVE DESTINATION ${INSTALL_LIB}		COMPONENT dev
)

# Files
install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/index.js
	${CMAKE_CURRENT_SOURCE_DIR}/package.json
	${CMAKE_CURRENT_SOURCE_DIR}/package-lock.json
	${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
	DESTINATION ${INSTALL_LIB}/node_modules/metacall
	COMPONENT runtime
)

#
# Configure test
#

# Check if loaders are enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_RB OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_SCRIPTS_RB)
	return()
endif()

set(node_port_test "${target}_test")

#
# Define test
#

add_test(NAME ${target}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND npm test
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${node_port_test}
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)
