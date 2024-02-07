# Check if this	detour is enabled
if(NOT OPTION_FORK_SAFE OR NOT OPTION_BUILD_DETOURS OR NOT OPTION_BUILD_DETOURS_FUNCHOOK)
	return()
endif()

#
# External dependencies
#

find_package(Git REQUIRED)

# Target depends name
set(target_depends funchook_detour_depends)

include(ExternalProject)

set(FUNCHOOK_VERSION 1.1.1)

if(WIN32)
	set(FUNCHOOK_LIBRARY_PREFIX "")
	set(FUNCHOOK_LIBRARY_SUFFIX "lib")
	set(FUNCHOOK_LIBRARY_INSTALL_SUFFIX "dll")
elseif(APPLE)
	set(FUNCHOOK_LIBRARY_PREFIX "lib")
	set(FUNCHOOK_LIBRARY_SUFFIX "dylib")
	set(FUNCHOOK_LIBRARY_INSTALL_SUFFIX "dylib")
else()
	set(FUNCHOOK_LIBRARY_PREFIX "lib")
	set(FUNCHOOK_LIBRARY_SUFFIX "so")
	set(FUNCHOOK_LIBRARY_INSTALL_SUFFIX "so")
endif()

set(FUNCHOOK_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/funchook/src/funchook")

if(WIN32)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/download.bat.in ${CMAKE_CURRENT_BINARY_DIR}/download.bat @ONLY)
	set(FUNCHOOK_DOWNLOAD_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/download.bat)
	set(FUNCHOOK_BUILD_TARGET "INSTALL")
else()
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/download.sh.in ${CMAKE_CURRENT_BINARY_DIR}/download.sh @ONLY)
	set(FUNCHOOK_DOWNLOAD_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/download.sh)
	set(FUNCHOOK_BUILD_TARGET "install")
endif()

set(FUNCHOOK_INSTALL_DIR "${PROJECT_OUTPUT_DIR}")
set(FUNCHOOK_INCLUDE_DIR "${FUNCHOOK_SOURCE_DIR}/include")
set(FUNCHOOK_LIBRARY_DIR "${FUNCHOOK_SOURCE_DIR}/${FUNCHOOK_LIBRARY_PREFIX}funchook.${FUNCHOOK_LIBRARY_SUFFIX}")
set(FUNCHOOK_LIBRARY_INSTALL_DIR "${FUNCHOOK_SOURCE_DIR}/${FUNCHOOK_LIBRARY_PREFIX}funchook.${FUNCHOOK_LIBRARY_INSTALL_SUFFIX}")

ExternalProject_Add(
	${target_depends}
	PREFIX funchook
	SOURCE_DIR ${FUNCHOOK_SOURCE_DIR}
	INSTALL_DIR ${FUNCHOOK_INSTALL_DIR}
	DOWNLOAD_COMMAND ${FUNCHOOK_DOWNLOAD_COMMAND}
	CONFIGURE_COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" -DCMAKE_BUILD_PARALLEL_LEVEL=1 -DCMAKE_PLATFORM_NO_VERSIONED_SONAME=ON -DCMAKE_INSTALL_PREFIX=${FUNCHOOK_INSTALL_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DFUNCHOOK_BUILD_SHARED=ON -DFUNCHOOK_BUILD_TESTS=OFF -DFUNCHOOK_BUILD_STATIC=OFF .
	BUILD_COMMAND ${CMAKE_COMMAND} -E env CMAKE_BUILD_PARALLEL_LEVEL=1 ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE} --target ${FUNCHOOK_BUILD_TARGET}
	UPDATE_COMMAND ""
	BUILD_IN_SOURCE ON
	LOG_DOWNLOAD ON
	LOG_CONFIGURE ON
	LOG_BUILD ON
	LOG_INSTALL ON
)

#
# Library name and options
#

# Target name
set(target funchook_detour)

# Exit here if required dependencies are not met
message(STATUS "Detour ${target}")

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
	${include_path}/funchook_detour.h
	${include_path}/funchook_detour_impl.h
)

set(sources
	${source_path}/funchook_detour.c
	${source_path}/funchook_detour_impl.c
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
	${target_depends}
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
	${FUNCHOOK_INCLUDE_DIR} # FuncHook includes

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
	${FUNCHOOK_LIBRARY_DIR} # FuncHook libraries

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

# Dependency
install(FILES
	${FUNCHOOK_LIBRARY_INSTALL_DIR}
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)

# Library
install(TARGETS ${target}
	EXPORT  "${target}-export"				COMPONENT dev
	RUNTIME DESTINATION ${INSTALL_BIN}		COMPONENT runtime
	LIBRARY DESTINATION ${INSTALL_SHARED}	COMPONENT runtime
	ARCHIVE DESTINATION ${INSTALL_LIB}		COMPONENT dev
)
