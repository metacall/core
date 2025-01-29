# Check if this loader loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_CS)
	return()
endif()

# TODO: Make this part of FindCoreCLR.cmake (automatically detect it)
set(DOTNET_CORE_PATH "" CACHE PATH "Dotnet runtime path")

#
# External dependencies
#

find_package(CoreCLR)

if(NOT CORECLR_FOUND)
	message(SEND_ERROR "CoreCLR libraries not found")
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

# Build library
add_library(${target} MODULE
	${sources}
	${headers}
)

# Add target dependencies
add_dependencies(${target}
	cs_loader_impl
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
	${META_PROJECT_NAME}::metacall # MetaCall library
	$<$<NOT:$<BOOL:${MSVC}>>:stdc++fs> # C++ FileSystem

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
# Compile features
#

target_compile_features(${target}
	PRIVATE
	cxx_std_11
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

set(CS_LOADER_CONFIG_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/data/cs_loader.json.in")
string(REPLACE "\\" "/" DOTNET_CORE_PATH "${DOTNET_CORE_PATH}")

# Development
loader_configuration_begin(cs_loader "${CS_LOADER_CONFIG_TEMPLATE}")
set(DOTNET_CORE_LOADER_ASSEMBLY_PATH ${PROJECT_OUTPUT_DIR}/CSLoader.dll)
loader_configuartion_end_development()

# Install
loader_configuration_begin(cs_loader "${CS_LOADER_CONFIG_TEMPLATE}")
set(DOTNET_CORE_LOADER_ASSEMBLY_PATH ${PROJECT_OUTPUT_DIR}/CSLoader.dll)
loader_configuartion_end_install()

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
