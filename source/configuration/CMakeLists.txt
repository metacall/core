#
# Library name and options
#

# Target name
set(target configuration)

# Exit here if required dependencies are not met
message(STATUS "Lib ${target}")

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
	${include_path}/configuration.h
	${include_path}/configuration_object_handle.h
	${include_path}/configuration_singleton.h
	${include_path}/configuration_object.h
	${include_path}/configuration_impl.h
)

set(sources
	${source_path}/configuration.c
	${source_path}/configuration_singleton.c
	${source_path}/configuration_object.c
	${source_path}/configuration_impl.c
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
	${META_PROJECT_NAME}::environment
	${META_PROJECT_NAME}::format
	${META_PROJECT_NAME}::log
	${META_PROJECT_NAME}::adt
	${META_PROJECT_NAME}::reflect
	${META_PROJECT_NAME}::dynlink
	${META_PROJECT_NAME}::serial

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
# Global configuration
#

function(configurations_write config_dir config_path)
	set(CONFIGURATION_GLOBAL "{")

	if(OPTION_BUILD_PLUGINS)
		set(CONFIGURATION_GLOBAL_PLUGINS 0)

		if(OPTION_BUILD_PLUGINS_CS)
			set(CONFIGURATION_GLOBAL "${CONFIGURATION_GLOBAL}\n\t\"cs_loader\":\"${config_dir}/cs_loader.json\",")
			set(CONFIGURATION_GLOBAL_PLUGINS 1)
		endif()

		#if(OPTION_BUILD_PLUGINS_JS)
		#	set(CONFIGURATION_GLOBAL "${CONFIGURATION_GLOBAL}\n\t\"js_loader\":\"${config_dir}/js_loader.json\",")
		#	set(CONFIGURATION_GLOBAL_PLUGINS 1)
		#endif()

		if(CONFIGURATION_GLOBAL_PLUGINS EQUAL 1)
			string(LENGTH "${CONFIGURATION_GLOBAL}" CONFIGURATION_GLOBAL_LENGTH)

			math(EXPR CONFIGURATION_GLOBAL_LENGTH "${CONFIGURATION_GLOBAL_LENGTH} - 1")

			string(SUBSTRING "${CONFIGURATION_GLOBAL}" 0 ${CONFIGURATION_GLOBAL_LENGTH} CONFIGURATION_GLOBAL)
		endif()
	endif()

	set(CONFIGURATION_GLOBAL "${CONFIGURATION_GLOBAL}\n}")

	file(WRITE "${config_path}" "${CONFIGURATION_GLOBAL}")
endfunction()

#
# Development Configuration
#

# Generate runtime configuration
configurations_write(${CONFIGURATION_DIR} ${CONFIGURATION_PATH})

#
# Install Configuration
#

# Generate runtime configuration
configurations_write(${CMAKE_INSTALL_PREFIX}/${INSTALL_DATA}/configurations ${CONFIGURATION_DIR}/install/configurations/global.json)

# Install runtime configuration
install(DIRECTORY ${CONFIGURATION_DIR}/install/configurations DESTINATION ${INSTALL_DATA} COMPONENT runtime)
