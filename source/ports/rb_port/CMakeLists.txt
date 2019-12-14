# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_RB)
	return()
endif()

#
# External dependencies
#

find_package(Ruby)

if(NOT RUBY_FOUND)
	message(STATUS "Ruby libraries not found")
	return()
endif()

#
# Port name and options
#

# Target name
set(target rb_port)

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

# Set API export file and macro
string(TOUPPER ${target} target_upper)
set(feature_file "include/${target}/${target}_features.h")
set(export_file  "include/${target}/${target}_api.h")
set(export_macro "${target_upper}_API")

#
# Sources
#

set(interface_path "${CMAKE_CURRENT_SOURCE_DIR}/interface/${target}")
set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target}")
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(interfaces
	${interface_path}/rb_port.i
)

set(headers
	${include_path}/rb_port.h
)

set(sources
	${source_path}/rb_port.c
)

# Group source files
set(interface_group "Interface Files (SWIG)")
set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${interface_path} "\\\\.i$"
	${interface_group} ${interfaces})
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$"
	${header_group} ${headers})
source_group_by_path(${source_path} "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$"
	${source_group} ${sources})

#
# SWIG Configuration
#

# Set SWIG flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	list(APPEND CMAKE_SWIG_FLAGS "-DDEBUG")
else()
	list(APPEND CMAKE_SWIG_FLAGS "-DNDEBUG")
endif()

# Set SWIG include path
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/interface")

#
# Create library
#

foreach(file ${interfaces} ${headers} ${sources})
	set_source_files_properties(
		${file}
		PROPERTY SWIG_FLAGS "-ruby" "-includeall"
	)

	set_source_files_properties(
		${file}
		PROPERTIES CPLUSPLUS OFF
	)
endforeach()

swig_add_module(${target}
	ruby
	${interfaces}
	${headers}
	${sources}
)

#
# Dependecies
#

# Add metacall distributable dependency
add_dependencies(${SWIG_MODULE_${target}_REAL_NAME}
	${META_PROJECT_NAME}::metacall_distributable
)

# Create namespaced alias
add_library(${META_PROJECT_NAME}::${target} ALIAS ${SWIG_MODULE_${target}_REAL_NAME})

# Export library for downstream projects
export(TARGETS ${SWIG_MODULE_${target}_REAL_NAME} NAMESPACE ${META_PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

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
generate_export_header(${SWIG_MODULE_${target}_REAL_NAME}
	EXPORT_FILE_NAME  ${export_file}
	EXPORT_MACRO_NAME ${export_macro}
)

#
# Project options
#

set_target_properties(${SWIG_MODULE_${target}_REAL_NAME}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

#
# Include directories
#
target_include_directories(${SWIG_MODULE_${target}_REAL_NAME}
	PRIVATE
	${PROJECT_BINARY_DIR}/source/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include

	${RUBY_INCLUDE_DIRS} # Ruby includes

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

swig_link_libraries(${target}
	PRIVATE
	${RUBY_LIBRARY} # Ruby libraries

	${META_PROJECT_NAME}::metacall_distributable

	PUBLIC
	${DEFAULT_LIBRARIES}

	INTERFACE
)

#
# Compile definitions
#

target_compile_definitions(${SWIG_MODULE_${target}_REAL_NAME}
	PRIVATE

	PUBLIC
	$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${target_upper}_STATIC_DEFINE>
	${DEFAULT_COMPILE_DEFINITIONS}

	INTERFACE
)

#
# Compile options
#

target_compile_options(${SWIG_MODULE_${target}_REAL_NAME}
	PRIVATE

	PUBLIC
	${DEFAULT_COMPILE_OPTIONS}

	INTERFACE
)

#
# Linker options
#

target_link_libraries(${SWIG_MODULE_${target}_REAL_NAME}
	PRIVATE
	${META_PROJECT_NAME}::metacall_distributable

	PUBLIC
	${DEFAULT_LINKER_OPTIONS}

	INTERFACE
)

#
# Deployment
#

# Library
install(TARGETS ${SWIG_MODULE_${target}_REAL_NAME}
	EXPORT  "${target}-export"			COMPONENT dev
	RUNTIME DESTINATION ${INSTALL_BIN}	COMPONENT runtime
	LIBRARY DESTINATION ${INSTALL_SHARED} COMPONENT runtime
	ARCHIVE DESTINATION ${INSTALL_LIB}	COMPONENT dev
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

#
# Configure test
#

# Check if port is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_RB OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_SCRIPTS_RB)
	return()
endif()

set(rb_port_test "${target}_test")
set(rb_port_test_path "${CMAKE_BINARY_DIR}/${rb_port_test}.rb")

# Require module name
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	get_target_property(DEBUG_POSTFIX ${SWIG_MODULE_${target}_REAL_NAME} "DEBUG_POSTFIX")
	set(RB_PORT_NAME "${SWIG_MODULE_${target}_REAL_NAME}${DEBUG_POSTFIX}")
else()
	set(RB_PORT_NAME "${SWIG_MODULE_${target}_REAL_NAME}")
endif()

# Module object instance (capitalized)
string(SUBSTRING "${RB_PORT_NAME}" 0 1 RB_PORT_FIRST)
string(TOUPPER "${RB_PORT_FIRST}" RB_PORT_FIRST_UPPER)
string(SUBSTRING "${RB_PORT_NAME}" 1 -1 RB_PORT_LAST)
set(RB_PORT_OBJ "${RB_PORT_FIRST_UPPER}${RB_PORT_LAST}")

configure_file(test/run.rb.in ${rb_port_test_path})

#
# Define test
#

add_test(NAME ${target}
	COMMAND ${RUBY_EXECUTABLE} ${rb_port_test_path}
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${rb_port_test}
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)
