#
# External dependencies
#

# find_package(THIRDPARTY REQUIRED)

#
# Library name and options
#

# Target name
set(target filesystem)

# Exit here if required dependencies are not met
message(STATUS "Lib ${target}")

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

set(generated_include_path "${CMAKE_CURRENT_BINARY_DIR}/include/${target}")
set(generated_source_path  "${CMAKE_CURRENT_BINARY_DIR}/source")

# Configure cross-platform support implementation
set(FILESYSTEM_IMPL_INTERFACE_NAME ${PROJECT_OS_FAMILY})

#configure_file(${include_path}/filesystem_interface.h.in ${generated_include_path}/filesystem_interface.h)
#configure_file(${source_path}/filesystem_interface.c.in ${generated_source_path}/filesystem_interface.c)

set(headers
	${include_path}/filesystem.h
	${include_path}/filesystem_file_descriptor.h
	${include_path}/filesystem_directory_descriptor.h

#	${include_path}/filesystem_impl.h
#	${include_path}/filesystem_impl_file_${FILESYSTEM_IMPL_INTERFACE_NAME}.h
#	${include_path}/filesystem_impl_directory_${FILESYSTEM_IMPL_INTERFACE_NAME}.h
#	${include_path}/filesystem_impl_${FILESYSTEM_IMPL_INTERFACE_NAME}.h
#	${generated_include_path}/filesystem_interface.h
)

set(sources
	${source_path}/filesystem.c
	${source_path}/filesystem_file_descriptor.c
	${source_path}/filesystem_directory_descriptor.c

#	${source_path}/filesystem_impl.c
#	${source_path}/filesystem_impl_file_${FILESYSTEM_IMPL_INTERFACE_NAME}.c
#	${source_path}/filesystem_impl_directory_${FILESYSTEM_IMPL_INTERFACE_NAME}.c
#	${source_path}/filesystem_impl_${DYNLINK_IMPL_INTERFACE_NAME}.c
#	${generated_source_path}/filesystem_interface.c
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
	${META_PROJECT_NAME}::format
	${META_PROJECT_NAME}::threading
	${META_PROJECT_NAME}::log
	${META_PROJECT_NAME}::adt

	PUBLIC
	${DEFAULT_LIBRARIES}

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

target_link_options(${target}
	PRIVATE

	PUBLIC
	${DEFAULT_LINKER_OPTIONS}

	INTERFACE
)
