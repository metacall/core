# Check if this loader loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_CS)
	return()
endif()

set(DOTNET_CORE_PATH "" CACHE PATH "Dotnet runtime path")

#
# External dependencies
#

find_package(CoreCLR)

if(NOT CORECLR_FOUND)
	message(STATUS "CoreCLR libraries not found")
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target cs_loader)

add_subdirectory(netcore)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target}")

# Set API export file and macro
string(TOUPPER ${target} target_upper)
set(feature_file "include/${target}/${target}_features.h")
set(export_file  "include/${target}/${target}_api.h")
set(export_macro "${target_upper}_API")

# Force Unicode over Multi-byte
if(MSVC)
	add_definitions(-DUNICODE -D_UNICODE)
endif()

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


if(MSVC)
	set(headers_platform
		${include_path}/string_buffer.h
		${include_path}/netcore_win.h
		${include_path}/host_environment.h
	)

	set(sources_platform
		${source_path}/string_buffer.cpp
		${source_path}/netcore_win.cpp
		${source_path}/host_environment.cpp
	)
else()
	set(headers_platform
		${include_path}/netcore_linux.h
	)
	set(sources_platform
		${source_path}/netcore_linux.cpp
	)
endif()


set(headers
	${include_path}/cs_loader.h
	${include_path}/cs_loader_impl.h
	${include_path}/defs.h
	${include_path}/netcore.h
	${include_path}/simple_netcore.h
	${headers_platform}
)

set(sources
	${source_path}/cs_loader.c
	${source_path}/cs_loader_impl.c
	${source_path}/netcore.cpp
	${source_path}/simple_netcore.cpp
	${sources_platform}
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
set(cs_loader_unity_build "${CMAKE_CURRENT_BINARY_DIR}/${target}_unity_build.c")

distributable_generate(${target} ${cs_loader_unity_build}
	${LOADER_MODULE_NAMES}
)

# Build library
add_library(${target} MODULE
	${cs_loader_unity_build}
	${sources}
	${headers}
)

# Add target dependencies
add_dependencies(${target}
	${LOADER_MODULE_NAMES}
	cs_loader_impl
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
)

#
# Include directories
#

target_include_directories(${target}
	PRIVATE
	${PROJECT_BINARY_DIR}/source/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include

	${CORECLR_INCLUDE_DIR} # CoreCLR includes

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
	$<$<NOT:$<BOOL:${MSVC}>>:stdc++fs>

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
# Configuration
#

string(REPLACE "\\" "/" DOTNET_CORE_PATH "${DOTNET_CORE_PATH}")

set(DOTNET_CORE_LOADER_ASSEMBLY_PATH  ${CMAKE_BINARY_DIR}/CSLoader.dll)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/cs_loader.json.in ${CONFIGURATION_DIR}/cs_loader.json)

set(DOTNET_CORE_LOADER_ASSEMBLY_PATH  ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB}/CSLoader.dll)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/cs_loader.json.in ${CONFIGURATION_DIR}/install/configurations/cs_loader.json)

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
