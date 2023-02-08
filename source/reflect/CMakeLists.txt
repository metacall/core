#
# Library name and options
#

# Target name
set(target reflect)

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
	${include_path}/reflect.h
	${include_path}/reflect_accessor.h
	${include_path}/reflect_async.h
	${include_path}/reflect_type.h
	${include_path}/reflect_type_id.h
	${include_path}/reflect_signature.h
	${include_path}/reflect_function.h
	${include_path}/reflect_attribute_decl.h
	${include_path}/reflect_attribute.h
	${include_path}/reflect_constructor_decl.h
	${include_path}/reflect_constructor.h
	${include_path}/reflect_memory_tracker.h
	${include_path}/reflect_method_decl.h
	${include_path}/reflect_method.h
	${include_path}/reflect_class_decl.h
	${include_path}/reflect_class_visibility.h
	${include_path}/reflect_class.h
	${include_path}/reflect_object.h
	${include_path}/reflect_future.h
	${include_path}/reflect_exception.h
	${include_path}/reflect_throwable.h
	${include_path}/reflect_scope.h
	${include_path}/reflect_context.h
	${include_path}/reflect_value.h
	${include_path}/reflect_value_type.h
	${include_path}/reflect_value_type_id_size.h
	${include_path}/reflect_value_type_promotion.h
	${include_path}/reflect_value_type_demotion.h
	${include_path}/reflect_value_type_cast.h
)

set(sources
	${source_path}/reflect.c
	${source_path}/reflect_type.c
	${source_path}/reflect_type_id.c
	${source_path}/reflect_signature.c
	${source_path}/reflect_function.c
	${source_path}/reflect_attribute.c
	${source_path}/reflect_constructor.c
	${source_path}/reflect_memory_tracker.c
	${source_path}/reflect_method.c
	${source_path}/reflect_class_visibility.c
	${source_path}/reflect_class.c
	${source_path}/reflect_object.c
	${source_path}/reflect_future.c
	${source_path}/reflect_exception.c
	${source_path}/reflect_throwable.c
	${source_path}/reflect_scope.c
	${source_path}/reflect_context.c
	${source_path}/reflect_value.c
	${source_path}/reflect_value_type.c
	${source_path}/reflect_value_type_id_size.c
	${source_path}/reflect_value_type_promotion.c
	${source_path}/reflect_value_type_demotion.c
	${source_path}/reflect_value_type_cast.c
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
	${META_PROJECT_NAME}::memory
	${META_PROJECT_NAME}::portability
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

target_link_libraries(${target}
	PRIVATE

	PUBLIC
	${DEFAULT_LINKER_OPTIONS}

	INTERFACE
)
