# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_RB)
	return()
endif()

#
# External dependencies
#

find_package(Ruby)

if(NOT Ruby_FOUND)
	message(SEND_ERROR "Ruby libraries not found")
	return()
endif()

# TODO: Search Ruby_LIBRARY_NAME_PATH like in Python?
if(Ruby_LIBRARY_NAME)
	set(Ruby_LIBRARY_NAME_PATH "${Ruby_LIBRARY_NAME}")
else()
	set(Ruby_LIBRARY_NAME_PATH "${Ruby_LIBRARY}")
endif()
get_filename_component(Ruby_LIBRARY_NAME "${Ruby_LIBRARY_NAME_PATH}" NAME)

# Copy Ruby DLL into project output directory
# TODO: https://cmake.org/cmake/help/latest/command/file.html#get-runtime-dependencies
# TODO: https://gist.github.com/micahsnyder/5d98ac8548b429309ec5a35bca9366da
if(Ruby_LIBRARY_NAME_PATH AND WIN32)
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_OUTPUT_DIR})
	file(COPY "${Ruby_LIBRARY_NAME_PATH}" DESTINATION ${PROJECT_OUTPUT_DIR})
endif()

#
# Plugin name and options
#

# Target name
set(target rb_loader)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target}")

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
	${include_path}/rb_loader.h
	${include_path}/rb_loader_include.h
	${include_path}/rb_loader_impl.h
	${include_path}/rb_loader_impl_parser.h
	${include_path}/rb_loader_port.h
)

set(sources
	${source_path}/rb_loader.c
	${source_path}/rb_loader_impl.c
	${source_path}/rb_loader_impl_parser.c
	${source_path}/rb_loader_port.c
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
add_library(${target} MODULE
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
	BUNDLE $<$<BOOL:${APPLE}>:$<$<VERSION_GREATER:${PROJECT_OS_VERSION},8>>>
)

#
# Include directories
#

target_include_directories(${target}
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

target_link_libraries(${target}
	PRIVATE
	${META_PROJECT_NAME}::metacall # MetaCall library

	# Delay load for MSVC
	$<$<CXX_COMPILER_ID:MSVC>:${Ruby_LIBRARY}> # Ruby library
	$<$<CXX_COMPILER_ID:MSVC>:delayimp>

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

# Fix Ruby MacOSX LLVM bug 
# '__declspec' attributes are not enabled; use '-fdeclspec' or '-fms-extensions' to enable support for __declspec attributes
include(Portability)

if("${PROJECT_OS_FAMILY}" STREQUAL "macos" AND CMAKE_C_COMPILER_ID MATCHES "Clang")
	target_compile_options(${target} PRIVATE "-fdeclspec")
endif()

#
# Linker options
#

target_link_options(${target}
	PRIVATE
	$<$<AND:$<BOOL:${APPLE}>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-Wl,-undefined,dynamic_lookup>
	$<$<CXX_COMPILER_ID:MSVC>:/DELAYLOAD:${Ruby_LIBRARY_NAME_PATH}>

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

# Runtime (pack Ruby DLL in windows)
# TODO: https://cmake.org/cmake/help/latest/command/file.html#get-runtime-dependencies
# TODO: https://gist.github.com/micahsnyder/5d98ac8548b429309ec5a35bca9366da
set(Ruby_LIBRARY_DEVELOPMENT "${Ruby_LIBRARY_NAME_PATH}")

if(Ruby_LIBRARY_NAME_PATH AND WIN32)
	install(FILES
		"${Ruby_LIBRARY_NAME_PATH}"
		DESTINATION ${INSTALL_LIB}
		COMPONENT runtime
	)
	set(Ruby_LIBRARY_INSTALL "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB}/${Ruby_LIBRARY_NAME}")
else()
	set(Ruby_LIBRARY_INSTALL "${Ruby_LIBRARY_NAME_PATH}")
endif()

# Define search paths
set(Ruby_LIBRARY_SEARCH_PATHS_DEVELOPMENT "${Ruby_LIBRARY_SEARCH_PATHS}")

if(Ruby_LIBRARY_SEARCH_PATHS AND WIN32)
	set(Ruby_LIBRARY_SEARCH_PATHS_INSTALL "")

	foreach(SEARCH_PATH IN LISTS Ruby_LIBRARY_SEARCH_PATHS)
		install(DIRECTORY
			"${SEARCH_PATH}"
			DESTINATION ${INSTALL_LIB}
			COMPONENT runtime
		)

		get_filename_component(SEARCH_PATH_FOLDER_NAME "${SEARCH_PATH}" NAME)

		list(APPEND Ruby_LIBRARY_SEARCH_PATHS_INSTALL "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB}/${SEARCH_PATH_FOLDER_NAME}")
	endforeach()
endif()

#
# Configuration
#

# Development
loader_configuration_begin(rb_loader)
loader_configuration_paths("${Ruby_LIBRARY_SEARCH_PATHS_DEVELOPMENT}")
loader_configuration_deps(ruby "${Ruby_LIBRARY_DEVELOPMENT}")
loader_configuartion_end_development()

# Install
loader_configuration_begin(rb_loader)
loader_configuration_paths("${Ruby_LIBRARY_SEARCH_PATHS_INSTALL}")
loader_configuration_deps(ruby "${Ruby_LIBRARY_INSTALL}")
loader_configuartion_end_install()
