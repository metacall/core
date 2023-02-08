# Check if port is enabled
if(NOT OPTION_BUILD_PORTS OR NOT OPTION_BUILD_PORTS_RB)
	return()
endif()

#
# External dependencies
#

find_package(Ruby)

if(NOT Ruby_FOUND)
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

if(${CMAKE_VERSION} VERSION_LESS "3.8.0")
	swig_add_module(${target}
		ruby
		${interfaces}
		${headers}
		${sources}
	)
else()
	swig_add_library(${target}
		LANGUAGE ruby
		SOURCES ${interfaces} ${headers} ${sources}
	)
endif()

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

	$<TARGET_PROPERTY:${META_PROJECT_NAME}::metacall,INCLUDE_DIRECTORIES> # MetaCall includes
	${Ruby_INCLUDE_DIRS} # Ruby includes

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
	${Ruby_LIBRARY} # Ruby libraries

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

# Fix Ruby MacOSX LLVM bug 
# '__declspec' attributes are not enabled; use '-fdeclspec' or '-fms-extensions' to enable support for __declspec attributes
include(Portability)

if("${PROJECT_OS_FAMILY}" STREQUAL "macos" AND CMAKE_C_COMPILER_ID MATCHES "Clang")
	target_compile_options(${target} PRIVATE "-fdeclspec")
endif()

#
# Linker options
#

target_link_libraries(${SWIG_MODULE_${target}_REAL_NAME}
	PRIVATE
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
	EXPORT  "${target}-export"				COMPONENT dev
	RUNTIME DESTINATION ${INSTALL_BIN}		COMPONENT runtime
	LIBRARY DESTINATION ${INSTALL_SHARED}	COMPONENT runtime
	ARCHIVE DESTINATION ${INSTALL_LIB}		COMPONENT dev
)

#
# Configure test
#

# Check if port is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY OR NOT OPTION_BUILD_LOADERS_RB OR NOT OPTION_BUILD_SCRIPTS OR NOT OPTION_BUILD_SCRIPTS_PY OR NOT OPTION_BUILD_SCRIPTS_RB)
	return()
endif()

set(rb_port_test "${target}_test")
set(rb_port_test_path "${PROJECT_OUTPUT_DIR}/${rb_port_test}.rb")

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

if(OPTION_BUILD_SANITIZER OR OPTION_BUILD_THREAD_SANITIZER)
	# TODO: This test fails when run with sanitizers:
	#
	# Sanitizer:
	# ASan runtime does not come first in initial library list; you should either link runtime to your application or manually preload it with LD_PRELOAD.
	# For solving the issue compile the test with sanitizers when enabled or preload asan
	#
	# Thread Sanitizer:
	# /usr/local/metacall/build/rb_port_test.rb:4:in `require_relative': /usr/lib/x86_64-linux-gnu/libtsan.so.2: cannot allocate memory in static TLS block - /usr/local/metacall/build/rb_portd.so (LoadError)
	# from /usr/local/metacall/build/rb_port_test.rb:4:in `<main>'
	return()
endif()

add_test(NAME ${target}
	COMMAND ${Ruby_EXECUTABLE} ${rb_port_test_path}
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
