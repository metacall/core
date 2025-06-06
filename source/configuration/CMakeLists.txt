#
# Library name and options
#

# Target name
set(target configuration)

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
	${META_PROJECT_NAME}::threading
	${META_PROJECT_NAME}::log
	${META_PROJECT_NAME}::memory
	${META_PROJECT_NAME}::portability
	${META_PROJECT_NAME}::adt
	${META_PROJECT_NAME}::reflect
	${META_PROJECT_NAME}::dynlink
	${META_PROJECT_NAME}::plugin
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

#
# Global configuration
#

function(configurations_write config_dir config_path)
	set(CONFIGURATION_GLOBAL "{")

	# TODO: Make this automatic for all loaders

	if(OPTION_BUILD_LOADERS)
		if(OPTION_BUILD_LOADERS_CS)
			set(CONFIGURATION_GLOBAL "${CONFIGURATION_GLOBAL}\n\t\"cs_loader\":\"${config_dir}/cs_loader.json\",")
		endif()

		if(OPTION_BUILD_LOADERS_NODE)
			set(CONFIGURATION_GLOBAL "${CONFIGURATION_GLOBAL}\n\t\"node_loader\":\"${config_dir}/node_loader.json\",")
		endif()

		if(OPTION_BUILD_LOADERS_PY)
			set(CONFIGURATION_GLOBAL "${CONFIGURATION_GLOBAL}\n\t\"py_loader\":\"${config_dir}/py_loader.json\",")
		endif()


		if(OPTION_BUILD_LOADERS_RB)
			set(CONFIGURATION_GLOBAL "${CONFIGURATION_GLOBAL}\n\t\"rb_loader\":\"${config_dir}/rb_loader.json\",")
		endif()

		#if(OPTION_BUILD_LOADERS_JS)
		#	set(CONFIGURATION_GLOBAL "${CONFIGURATION_GLOBAL}\n\t\"js_loader\":\"${config_dir}/js_loader.json\",")
		#endif()
	endif()

	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(CONFIGURATION_GLOBAL "${CONFIGURATION_GLOBAL}\n\t\"log_level\":\"Debug\"")
	elseif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		set(CONFIGURATION_GLOBAL "${CONFIGURATION_GLOBAL}\n\t\"log_level\":\"Error\"")
	else()
		set(CONFIGURATION_GLOBAL "${CONFIGURATION_GLOBAL}\n\t\"log_level\":\"Error\"")
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
