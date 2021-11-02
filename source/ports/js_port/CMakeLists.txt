# Check if port is enabled
if(NOT OPTION_BUILD_PORTS_JS)
	return()
endif()

#
# External dependencies
#

find_package(V8 5.1)

if(NOT V8_FOUND)
	message(STATUS "V8 libraries not found")
	return()
endif()

#
# Port name and options
#

# Target name
set(target js_port)

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
	${interface_path}/js_port.i
)

set(headers
	${include_path}/js_port.h
)

set(sources
	${source_path}/js_port.c
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

# Set SWIG flags (use V8 engine)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	list(APPEND CMAKE_SWIG_FLAGS "-v8" "-DDEBUG")
else()
	list(APPEND CMAKE_SWIG_FLAGS "-v8" "-DNDEBUG")
endif()

# Set SWIG include path
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/interface")

#
# Create library
#

foreach(file ${interfaces} ${headers} ${sources})

	if(${V8_VERSION} LESS "4.3.0")
		set_source_files_properties(
			${file}
			PROPERTY SWIG_FLAGS "-javascript" "-DV8_VERSION=${V8_VERSION_HEX}" "-includeall"
		)
	else()
		set_source_files_properties(
			${file}
			PROPERTY SWIG_FLAGS "-javascript" "-includeall"
		)
	endif()

	set_source_files_properties(
		${file}
		PROPERTIES CPLUSPLUS ON
	)
endforeach()

if(${CMAKE_VERSION} VERSION_LESS "3.8.0")
	swig_add_module(${target}
		javascript
		${interfaces}
		${headers}
		${sources}
	)
else()
	swig_add_library(${target}
		LANGUAGE javascript
		SOURCES ${interfaces} ${headers} ${sources}
	)
endif()

set_property(TARGET ${target} PROPERTY SWIG_USE_LIBRARY_INCLUDE_DIRECTORIES TRUE)

#
# Dependecies
#

add_dependencies(${SWIG_MODULE_${target}_REAL_NAME}
	${META_PROJECT_NAME}::metacall
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

	${V8_INCLUDE_DIR} # V8 includes

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
	${V8_LIBRARIES} # V8 libraries

	${META_PROJECT_NAME}::metacall

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
	${V8_LIBRARIES} # V8 libraries

	${META_PROJECT_NAME}::metacall

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
# Test name and options
#

# Test name
set(js_port_test "${target}_test")

# Exit here if required dependencies are not met
message(STATUS "Test ${js_port_test}")

# Set API export file and macro
string(TOUPPER ${js_port_test} js_port_test_upper)

#
# Configure test scripts
#

set(js_port_test_path "${PROJECT_OUTPUT_DIR}/${js_port_test}.js")

# Require module name
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	get_target_property(DEBUG_POSTFIX ${SWIG_MODULE_${target}_REAL_NAME} "DEBUG_POSTFIX")
	set(JS_PORT_NAME "${SWIG_MODULE_${target}_REAL_NAME}${DEBUG_POSTFIX}")
else()
	set(JS_PORT_NAME "${SWIG_MODULE_${target}_REAL_NAME}")
endif()

configure_file(test/run.js.in ${js_port_test_path})

#
# Configure test executable
#

add_executable(${js_port_test}
	MACOSX_BUNDLE
	test/main.cpp
)

#
# Dependecies
#

add_dependencies(${js_port_test}
	${META_PROJECT_NAME}::metacall
)

# Export library for downstream projects
export(TARGETS ${js_port_test} NAMESPACE ${META_PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${js_port_test}/${js_port_test}-export.cmake)

#
# Project options
#

set_target_properties(${js_port_test}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)

#
# Include directories
#

target_include_directories(${js_port_test}
	PRIVATE
	${PROJECT_BINARY_DIR}/source/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include

	${V8_INCLUDE_DIR} # V8 includes

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

target_link_libraries(${js_port_test}
	PRIVATE
	${V8_LIBRARIES} # V8 library

	${META_PROJECT_NAME}::metacall

	PUBLIC
	${DEFAULT_LIBRARIES}

	INTERFACE
)

#
# Compile definitions
#

target_compile_definitions(${js_port_test}
	PRIVATE

	PUBLIC
	$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${target_upper}_STATIC_DEFINE>
	${DEFAULT_COMPILE_DEFINITIONS}

	INTERFACE
)

#
# Compile options
#

target_compile_options(${js_port_test}
	PRIVATE

	PUBLIC
	${DEFAULT_COMPILE_OPTIONS}

	INTERFACE
)

#
# Linker options
#

target_link_libraries(${js_port_test}
	PRIVATE

	PUBLIC
	${DEFAULT_LINKER_OPTIONS}

	INTERFACE
)

#
# Define test command
#

add_test(NAME ${target}
	COMMAND ${js_port_test} ${js_port_test_path}
)

#
# Define test labels
#

set_property(TEST ${target}
	PROPERTY LABELS ${js_port_test}
)

include(TestEnvironmentVariables)

test_environment_variables(${target}
	""
	${TESTS_ENVIRONMENT_VARIABLES}
)

#
# Post generation step (support for variadic arguments)
#

set(js_port_args_check "Illegal number of arguments for _wrap_metacall\\.")
set(js_port_post_generated_file "${swig_generated_file_fullname}.post")

add_custom_command(
	TARGET ${target}
	PRE_BUILD
	COMMAND ${CMAKE_COMMAND}
		-DSWIG_GENERATED_FILE=${swig_generated_file_fullname}
		-DOUTPUT_GENERATED_FILE=${js_port_post_generated_file}
		-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/RemoveArgsCheck.cmake
	COMMAND ${CMAKE_COMMAND} -E remove ${swig_generated_file_fullname}
	COMMAND ${CMAKE_COMMAND} -E rename ${js_port_post_generated_file} ${swig_generated_file_fullname}
)
