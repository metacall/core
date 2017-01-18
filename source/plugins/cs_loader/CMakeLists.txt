# Check if this plugin is enabled
if(NOT OPTION_BUILD_PLUGINS OR NOT OPTION_BUILD_PLUGINS_CS)
	return()
endif()

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

# force Unicode over Multi-byte
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
	${include_path}/logger.h
	${include_path}/simple_netcore.h
	${headers_platform}
)

set(sources
	${source_path}/cs_loader.c
	${source_path}/cs_loader_impl.c
	${source_path}/netcore.cpp
	${source_path}/logger.cpp
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
add_library(${target}
	${sources}
	${headers}
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
	${META_PROJECT_NAME}::version
	${META_PROJECT_NAME}::preprocessor
	${META_PROJECT_NAME}::format
	${META_PROJECT_NAME}::log
	${META_PROJECT_NAME}::adt
	${META_PROJECT_NAME}::dynlink
	${META_PROJECT_NAME}::reflect
	${META_PROJECT_NAME}::loader

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
# Deployment
#

# Library
install(TARGETS ${target}
	EXPORT  "${target}-export"			COMPONENT dev
	RUNTIME DESTINATION ${INSTALL_BIN}	COMPONENT runtime
	LIBRARY DESTINATION ${INSTALL_SHARED} COMPONENT runtime
	ARCHIVE DESTINATION ${INSTALL_LIB}	COMPONENT dev
)

# Header files
install(DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}/include/${target} DESTINATION ${INSTALL_INCLUDE}
	COMPONENT dev
)

# Generated header files
install(DIRECTORY
	${CMAKE_CURRENT_BINARY_DIR}/include/${target} DESTINATION ${INSTALL_INCLUDE}
	COMPONENT dev
)

# CMake config
install(EXPORT ${target}-export
	NAMESPACE   ${META_PROJECT_NAME}::
	DESTINATION ${INSTALL_CMAKE}/${target}
	COMPONENT   dev
)
