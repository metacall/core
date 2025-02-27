#
# Library name and options
#

# Target name
set(target metacall)

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
# Configure templates
#

if(OPTION_FORK_SAFE)
	set(METACALL_FORK_SAFE 1)
endif()

if(OPTION_THREAD_SAFE)
	set(METACALL_THREAD_SAFE 1)
endif()

set(include_bin_path ${CMAKE_CURRENT_BINARY_DIR}/include/${target})

# Generate loaders plugin header
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/metacall_def.h.in ${include_bin_path}/metacall_def.h)

#
# Sources
#

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target}")
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(headers
	${include_bin_path}/metacall_def.h
	${include_path}/metacall.h
	${include_path}/metacall_value.h
	${include_path}/metacall_log.h
	${include_path}/metacall_allocator.h
	${include_path}/metacall_error.h
	${include_path}/metacall_link.h
)

set(sources
	${source_path}/metacall.c
	${source_path}/metacall_value.c
	${source_path}/metacall_log.c
	${source_path}/metacall_allocator.c
	${source_path}/metacall_error.c
	${source_path}/metacall_link.c
)

if(OPTION_FORK_SAFE)
	set(headers ${headers}
		${include_path}/metacall_fork.h
	)
	set(sources ${sources}
		${source_path}/metacall_fork.c
	)
endif()

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

# Include here any library which metacall depends on
set(unity_build_depends
	version
	preprocessor
	environment
	format
	threading
	log
	memory
	portability
	adt
	filesystem
	dynlink
	plugin
	detour
	reflect
	serial
	configuration
	loader
)

set(unity_build_source_list)
set(unity_build_definition_list)
set(unity_build_include_list)

foreach(tgt ${unity_build_depends})
	# Get target source files
	get_target_property(target_sources
		${META_PROJECT_NAME}::${tgt}
		SOURCES
	)

	set(unity_build_source_list
		${unity_build_source_list}
		${target_sources}
	)

	# Set target definitions
	set(unity_build_definition_list
		${unity_build_definition_list}
		$<TARGET_PROPERTY:${META_PROJECT_NAME}::${tgt},COMPILE_DEFINITIONS>
	)

	# Set target include paths
	set(unity_build_include_list
		${unity_build_include_list}
		$<TARGET_PROPERTY:${META_PROJECT_NAME}::${tgt},INCLUDE_DIRECTORIES>
	)
endforeach()

# Build library
add_library(${target}
	${unity_build_source_list}
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
	# Dependencies Includes
	${unity_build_include_list}

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
	# Dependencies Export API
	${unity_build_definition_list}

	# MetaCall Export API
	${target_upper}_EXPORTS

	$<$<BOOL:${OPTION_FORK_SAFE}>:${target_upper}_FORK_SAFE>
	$<$<BOOL:${OPTION_THREAD_SAFE}>:${target_upper}_THREAD_SAFE>

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

#
# Deployment
#

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
install(TARGETS ${target}
	EXPORT  "${target}-export"				COMPONENT dev
	RUNTIME DESTINATION ${INSTALL_BIN}		COMPONENT runtime
	LIBRARY DESTINATION ${INSTALL_SHARED}	COMPONENT runtime
	ARCHIVE DESTINATION ${INSTALL_LIB}		COMPONENT dev
)
