#
# External dependencies
#

# find_package(THIRDPARTY REQUIRED)

#
# Library name and options
#

# Target name
set(target metacall_distributable)

# Exit here if required dependencies are not met
message(STATUS "Distributable ${target}")

# Set API export file and macro
string(TOUPPER ${target} target_upper)

#
# Compiler warnings
#

include(Warnings)

#
# Compiler security
#

include(SecurityFlags)

#
# Create library
#

function(generate_unity_build unity_build_file)

	set(target_list "${ARGN}")

	set(unity_build_source)

	foreach(target ${target_list})

		# Get target source files
		get_target_property(target_sources
			${META_PROJECT_NAME}::${target}
			SOURCES
		)

		# Add private linkage for all targets except metacall
		if(NOT "${target}" STREQUAL "metacall")
			string(TOUPPER ${target} target_upper)
			set(unity_build_source "${unity_build_source}\n\#ifndef ${target_upper}_API")

			if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
				set(unity_build_source "${unity_build_source}\n\#	define ${target_upper}_API __attribute__ ((visibility(\"hidden\")))")
			else()
				set(unity_build_source "${unity_build_source}\n\#	define ${target_upper}_API")
			endif()

			set(unity_build_source "${unity_build_source}\n\#endif /* ${target_upper}_API */")
		else()
			set(unity_build_source "${unity_build_source}\n\#ifndef ${target}_EXPORTS")
			set(unity_build_source "${unity_build_source}\n\#	define ${target}_EXPORTS")
			set(unity_build_source "${unity_build_source}\n\#endif /* ${target}_EXPORTS */")
		endif()

		# Add include paths
		include_directories("${CMAKE_BINARY_DIR}/source/${target}/include")
		include_directories("${CMAKE_SOURCE_DIR}/source/${target}/include")

		# Write auto-generated includes into unity build
		set(unity_build_source "${unity_build_source}\n\#include <${CMAKE_BINARY_DIR}/source/${target}/include/${target}/${target}_api.h>")
		set(unity_build_source "${unity_build_source}\n\#include <${CMAKE_BINARY_DIR}/source/${target}/include/${target}/${target}_features.h>")

		# Write all includes into unity build
		foreach(source ${target_sources})
			set(unity_build_source "${unity_build_source}\n\#include <${source}>")
		endforeach()

	endforeach()

	file(WRITE ${unity_build_file} "${unity_build_source}")

	message(STATUS "Unity build written in ${unity_build_file}")

endfunction()

# Create unity build library
set(metacall_unity_build "${CMAKE_CURRENT_BINARY_DIR}/metacall_unity_build.c")

generate_unity_build(${metacall_unity_build}
	${MODULE_NAMES}
)

# Build library
add_library(${target}
	${metacall_unity_build}
)

# Add target dependencies
add_dependencies(${target}
	${MODULE_NAMES}
)

# Create namespaced alias
add_library(${META_PROJECT_NAME}::${target} ALIAS ${target})

# Export library for downstream projects
export(TARGETS ${target} NAMESPACE ${META_PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

#
# Project options
#

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
	DEBUG_POSTFIX ""
	OUTPUT_NAME "metacall"
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/distributable"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/distributable"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/distributable"
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
