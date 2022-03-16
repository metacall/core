#
# Library name and options
#

# Target name
set(target dynlink)

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

set(generated_include_path "${CMAKE_CURRENT_BINARY_DIR}/include/${target}")
set(generated_source_path  "${CMAKE_CURRENT_BINARY_DIR}/source")

# Configure cross-platform support implementation
if(APPLE AND PROJECT_OS_VERSION GREATER 8)
	# From macOS 10.5 (version 9), dlopen compatible library is available
	set(DYNLINK_IMPL_INTERFACE_NAME unix)
else()
	set(DYNLINK_IMPL_INTERFACE_NAME ${PROJECT_OS_FAMILY})
endif()

configure_file(${include_path}/dynlink_interface.h.in ${generated_include_path}/dynlink_interface.h)
configure_file(${source_path}/dynlink_interface.c.in ${generated_source_path}/dynlink_interface.c)

set(headers
	${include_path}/dynlink.h
	${include_path}/dynlink_flags.h
	${include_path}/dynlink_type.h
	${include_path}/dynlink_impl_name.h
	${include_path}/dynlink_impl_type.h
	${include_path}/dynlink_impl.h
	${include_path}/dynlink_impl_symbol_${DYNLINK_IMPL_INTERFACE_NAME}.h
	${include_path}/dynlink_impl_${DYNLINK_IMPL_INTERFACE_NAME}.h
	${generated_include_path}/dynlink_interface.h
	${include_path}/dynlink_symbol.h
)

set(sources
	${source_path}/dynlink.c
	${source_path}/dynlink_impl.c
	${source_path}/dynlink_impl_${DYNLINK_IMPL_INTERFACE_NAME}.c
	${generated_source_path}/dynlink_interface.c
	${source_path}/dynlink_symbol.c
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

	${DYNAMIC_LOADING_INCLUDE_DIR} # Native dynamic load library
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
	${META_PROJECT_NAME}::threading
	${META_PROJECT_NAME}::log
	${META_PROJECT_NAME}::portability

	PUBLIC
	${DEFAULT_LIBRARIES}

	$<$<BOOL:${BUILD_SHARED_LIBS}>:${CMAKE_DL_LIBS}> # Native dynamic load library

	INTERFACE
)

#
# Compile definitions
#

target_compile_definitions(${target}
	PRIVATE
	${target_upper}_EXPORTS # Export API

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
