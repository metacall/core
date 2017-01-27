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

# Generate object files for target
function(generate_object_library target)

	# Get target source files
	get_target_property(target_sources
		${META_PROJECT_NAME}::${target}
		SOURCES
	)

	set(target_object ${target}_object)

	# Create distributable library
	add_library(${target_object} OBJECT
		${target_sources}
	)

	# Create namespaced alias
	add_library(${META_PROJECT_NAME}::${target_object} ALIAS ${target_object})

	# Enable full compilation of all objects
	if(WIN32)
		if(MSVC)
			if(MSVC_VERSION GREATER 1800)
				set_target_properties(${target_object} PROPERTIES
					COMPILE_FLAGS "/Zc:inline-"
					LINK_FLAGS "/WHOLEARCHIVE"
				)
			else()
				set_target_properties(${target_object} PROPERTIES
					COMPILE_FLAGS "/Zc:inline-"
					LINK_FLAGS "/OPT:NOREF"
				)
			endif()
		else()
			message(WARNING "Unsuported MinGW/CygWin object compilation for unused modules")
		endif()
	elseif(APPLE OR IRIX)
		set_target_properties(${target_object} PROPERTIES
			LINK_FLAGS "-Wl,-all_load"
		)
	else()
		set_target_properties(${target_object} PROPERTIES
			LINK_FLAGS "-Wl,--whole-archive"
		)
	endif()

endfunction()

# Generate object libraries
macro(generate_object_libraries target)
	set(targets "${ARGN}")

	set(target_objects)

	foreach(object_library ${targets})
		generate_object_library(${object_library})

		set(target_objects "${target_objects} $<TARGET_OBJECTS:${META_PROJECT_NAME}::${object_library}_object>")
	endforeach()

	add_library(${target}
		${target_objects}
	)

	add_dependencies(${target}
		${targets}
	)
endmacro()

# Build library
generate_object_libraries(${target}
	${MODULE_NAMES}
)

# Set custom library output directory
#set_target_properties(${target} PROPERTIES
#	LIBRARY_OUTPUT_DIRECTORY "$"
#)

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
