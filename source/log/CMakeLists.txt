#
# Library name and options
#

# Target name
set(target log)

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

set(headers
	${include_path}/log.h
	${include_path}/log_preprocessor.h
	${include_path}/log_valid_size.h
	${include_path}/log_map.h
	${include_path}/log_level.h
	${include_path}/log_record.h
	${include_path}/log_handle.h
	${include_path}/log_policy.h
	${include_path}/log_aspect.h
	${include_path}/log_impl.h
	${include_path}/log_singleton.h

	${include_path}/log_policy_format.h
	${include_path}/log_policy_format_binary.h
	${include_path}/log_policy_format_custom.h
	${include_path}/log_policy_format_text.h

	${include_path}/log_policy_schedule.h
	${include_path}/log_policy_schedule_async.h
	${include_path}/log_policy_schedule_sync.h

	${include_path}/log_policy_storage.h
	${include_path}/log_policy_storage_batch.h
	${include_path}/log_policy_storage_sequential.h

	${include_path}/log_policy_stream.h
	${include_path}/log_policy_stream_custom.h
	${include_path}/log_policy_stream_file.h
	${include_path}/log_policy_stream_nginx.h
	${include_path}/log_policy_stream_socket.h
	${include_path}/log_policy_stream_stdio.h
	${include_path}/log_policy_stream_syslog.h

	${include_path}/log_aspect_format.h
	${include_path}/log_aspect_schedule.h
	${include_path}/log_aspect_stream.h
	${include_path}/log_aspect_storage.h
)

set(sources
	${source_path}/log.c
	${source_path}/log_valid_size.c
	${source_path}/log_map.c
	${source_path}/log_level.c
	${source_path}/log_record.c
	${source_path}/log_handle.c
	${source_path}/log_policy.c
	${source_path}/log_aspect.c
	${source_path}/log_impl.c
	${source_path}/log_singleton.c

	${source_path}/log_policy_format.c
	${source_path}/log_policy_format_binary.c
	${source_path}/log_policy_format_custom.c
	${source_path}/log_policy_format_text.c

	${source_path}/log_policy_schedule.c
	${source_path}/log_policy_schedule_async.c
	${source_path}/log_policy_schedule_sync.c

	${source_path}/log_policy_storage.c
	${source_path}/log_policy_storage_batch.c
	${source_path}/log_policy_storage_sequential.c

	${source_path}/log_policy_stream.c
	${source_path}/log_policy_stream_custom.c
	${source_path}/log_policy_stream_file.c
	${source_path}/log_policy_stream_nginx.c
	${source_path}/log_policy_stream_socket.c
	${source_path}/log_policy_stream_stdio.c
	${source_path}/log_policy_stream_syslog.c

	${source_path}/log_aspect_format.c
	${source_path}/log_aspect_schedule.c
	${source_path}/log_aspect_stream.c
	${source_path}/log_aspect_storage.c
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
	${META_PROJECT_NAME}::portability

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
