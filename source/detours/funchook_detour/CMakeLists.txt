# Check if this	detour is enabled
if(NOT OPTION_BUILD_DETOURS OR NOT OPTION_BUILD_DETOURS_FUNCHOOK)
	return()
endif()


#
# External dependencies
#

find_package(Git REQUIRED)

# Target depends name
set(target_depends funchook_detour_depends)

include(ExternalProject)

set(FUNCHOOK_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/funchook/src/funchook)

if(WIN32)
	if(MSVC_VERSION GREATER 1900)
		set(MSVC_TOOLS_VERSION "15.0")
	elseif(MSVC_VERSION GREATER 1800)
		set(MSVC_TOOLS_VERSION "14.0")
	elseif(MSVC_VERSION GREATER 1700)
		set(MSVC_TOOLS_VERSION "12.0")
	elseif(MSVC_VERSION GREATER 1600)
		set(MSVC_TOOLS_VERSION "11.0")
	elseif(MSVC_VERSION GREATER 1500)
		set(MSVC_TOOLS_VERSION "10.0")
	elseif(MSVC_VERSION GREATER 1400)
		set(MSVC_TOOLS_VERSION "9.0")
	elseif(MSVC_VERSION GREATER 1310)
		set(MSVC_TOOLS_VERSION "8.0")
	elseif(MSVC_VERSION GREATER 1300)
		set(MSVC_TOOLS_VERSION "7.01")
	elseif(MSVC_VERSION GREATER 1200)
		set(MSVC_TOOLS_VERSION "7.0")
	elseif(MSVC_VERSION GREATER 1100)
		set(MSVC_TOOLS_VERSION "6.0")
	else()
		set(MSVC_TOOLS_VERSION "5.0")
	endif()

	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/funchook.bat.in ${CMAKE_CURRENT_BINARY_DIR}/funchook.bat @ONLY)
	set(FUNCHOOK_DOWNLOAD_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/funchook.bat)
	set(FUNCHOOK_CONFIGURE_COMMAND ${CMAKE_VS_DEVENV_COMMAND} "${FUNCHOOK_SOURCE_DIR}/win32/funchook.vcxproj" /upgrade)
	set(FUNCHOOK_PATCH_COMMAND "")
	set(FUNCHOOK_BUILD_COMMAND_OPTIONS /p:PlatformToolset=${CMAKE_VS_PLATFORM_TOOLSET} /p:WindowsTargetPlatformVersion=${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION} /tv:${MSVC_TOOLS_VERSION})
	set(FUNCHOOK_BUILD_COMMAND ${CMAKE_VS_MSBUILD_COMMAND} ${FUNCHOOK_SOURCE_DIR}/win32/funchook.vcxproj /t:Build ${FUNCHOOK_BUILD_COMMAND_OPTIONS})
else()
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/funchook.sh.in ${CMAKE_CURRENT_BINARY_DIR}/funchook.sh @ONLY)
	set(FUNCHOOK_DOWNLOAD_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/funchook.sh)
	set(FUNCHOOK_CONFIGURE_COMMAND ${FUNCHOOK_SOURCE_DIR}/autogen.sh COMMAND ${FUNCHOOK_SOURCE_DIR}/configure)
	set(FUNCHOOK_PATCH_COMMAND patch ${FUNCHOOK_SOURCE_DIR}/src/funchook_io.c < ${CMAKE_CURRENT_SOURCE_DIR}/scripts/funchook.patch)
	set(FUNCHOOK_BUILD_COMMAND make)
endif()

ExternalProject_Add(
	${target_depends}
	PREFIX funchook
	SOURCE_DIR ${FUNCHOOK_SOURCE_DIR}
	DOWNLOAD_COMMAND ${FUNCHOOK_DOWNLOAD_COMMAND}
	CONFIGURE_COMMAND ${FUNCHOOK_CONFIGURE_COMMAND}
	PATCH_COMMAND ${FUNCHOOK_PATCH_COMMAND}
	BUILD_COMMAND ${FUNCHOOK_BUILD_COMMAND}
	UPDATE_COMMAND ""
	INSTALL_COMMAND "" # TODO
	BUILD_IN_SOURCE 1
	LOG_DOWNLOAD 1
	LOG_BUILD 1
)

set(FUNCHOOK_INCLUDE_DIR ${FUNCHOOK_SOURCE_DIR}/include)
set(FUNCHOOK_LIBRARY_DIR ${FUNCHOOK_SOURCE_DIR}/src)

if(WIN32)
	set(FUNCHOOK_LIBRARY_PREFIX "")
	set(FUNCHOOK_LIBRARY_SUFFIX ".dll")
elseif(APPLE)
	set(FUNCHOOK_LIBRARY_PREFIX "lib")
	set(FUNCHOOK_LIBRARY_SUFFIX ".dylib")
else()
	set(FUNCHOOK_LIBRARY_PREFIX "lib")
	set(FUNCHOOK_LIBRARY_SUFFIX ".so")
endif()

set(FUNCHOOK_LIBRARIES "${FUNCHOOK_LIBRARY_DIR}/${FUNCHOOK_LIBRARY_PREFIX}funchook${FUNCHOOK_LIBRARY_SUFFIX}")

#
# Library name and options
#

# Target name
set(target funchook_detour)

# Exit here if required dependencies are not met
message(STATUS "Detour ${target}")

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

include(Distributable)

# Create unity build library
set(funchook_detour_unity_build "${CMAKE_CURRENT_BINARY_DIR}/${target}_unity_build.c")

distributable_generate(${target} ${funchook_detour_unity_build}
	${DETOUR_MODULE_NAMES}
)

# Build library
add_library(${target}
	${funchook_detour_unity_build}
	${sources}
	${headers}
)

# Add target dependencies
add_dependencies(${target}
	${DETOUR_MODULE_NAMES}
	${target_depends}
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

	${FUNCHOOK_LIBRARIES} # FuncHook libraries

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
