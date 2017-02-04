#
# External dependencies
#

# find_package(THIRDPARTY REQUIRED)

#
# Library name and options
#

# Target name
set(target metacall_distributable)

# Library name
set(target_root "metacall")

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

include(Distributable)

# Create unity build library
set(metacall_unity_build "${CMAKE_CURRENT_BINARY_DIR}/${target_root}_unity_build.c")

distributable_generate(${metacall_unity_build}
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
	OUTPUT_NAME "${target_root}"
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

	${CMAKE_DL_LIBS}

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
	EXPORT  "${target_root}-export"			COMPONENT dev
	RUNTIME DESTINATION ${INSTALL_BIN}		COMPONENT runtime
	LIBRARY DESTINATION ${INSTALL_SHARED}	COMPONENT runtime
	ARCHIVE DESTINATION ${INSTALL_LIB}		COMPONENT dev
)
