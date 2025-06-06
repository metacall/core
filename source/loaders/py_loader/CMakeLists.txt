# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_PY)
	return()
endif()

#
# External dependencies
#

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(Python3_FIND_ABI "ON" "ANY" "ANY")
	find_package(Python3 COMPONENTS Interpreter Development)

	# Fallback to release if not found
	if(NOT Python3_Development_FOUND)
		set(Python3_FIND_ABI)
		find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
	endif()
else()
	find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
endif()

# Select the proper library
if(NOT Python3_LIBRARY AND Python3_LIBRARIES)
	# Go through the list and handle keyword-separated structure
	set(index 0)
	list(LENGTH Python3_LIBRARIES lib_len)
	while(index LESS lib_len)
		list(GET Python3_LIBRARIES ${index} item)

		# Check if it's a keyword (debug/optimized)
		if(item STREQUAL "debug" OR item STREQUAL "optimized" OR item STREQUAL "general")
			set(keyword ${item})
			math(EXPR next "${index} + 1")
			list(GET Python3_LIBRARIES ${next} lib_path)

			# Match the right keyword
			if((CMAKE_BUILD_TYPE STREQUAL "Debug" AND keyword STREQUAL "debug") OR
			   (NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND keyword STREQUAL "optimized") OR
			   (keyword STREQUAL "general"))  # general applies to all configs
				set(Python3_LIBRARY ${lib_path})
			endif()

			math(EXPR index "${index} + 2")  # Skip keyword and path
		else()
			# Plain list without keywords (single-config or fallback)
			set(Python3_LIBRARY ${item})
			math(EXPR index "${index} + 1")
		endif()
	endwhile()
endif()

# Copy Python DLL into project output directory
# TODO: https://cmake.org/cmake/help/latest/command/file.html#get-runtime-dependencies
# TODO: https://gist.github.com/micahsnyder/5d98ac8548b429309ec5a35bca9366da
include(Portability)

if(PROJECT_OS_FAMILY STREQUAL win32 AND Python3_LIBRARY)
	# Get the library path with dll suffix
	string(REGEX REPLACE "[.]lib$" ".dll" LIB_PATH "${Python3_LIBRARY}")
	# Get the library name
	get_filename_component(LIB_NAME "${LIB_PATH}" NAME)
	# Get the python root folder
	if(NOT Python3_ROOT_DIR)
		get_filename_component(Python3_ROOT_DIR "${Python3_EXECUTABLE}" DIRECTORY)
	endif()
	# Find the library in the Python3 root path
	find_file(Python3_LIBRARY_NAME_PATH "${LIB_NAME}"
		PATHS "${Python3_ROOT_DIR}"
		NO_DEFAULT_PATH
	)
	# Copy the DLL to the output directory
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_OUTPUT_DIR}")
	file(COPY "${Python3_LIBRARY_NAME_PATH}" DESTINATION "${PROJECT_OUTPUT_DIR}")
endif()

if(NOT Python3_LIBRARY_NAME_PATH)
	set(Python3_LIBRARY_NAME_PATH "${Python3_LIBRARY}")
endif()

#
# Plugin name and options
#

# Target name
set(target py_loader)

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
	${include_path}/py_loader.h
	${include_path}/py_loader_impl.h
	${include_path}/py_loader_port.h
	${include_path}/py_loader_threading.h
	${include_path}/py_loader_dict.h
	${include_path}/py_loader_symbol_fallback.h
)

set(sources
	${source_path}/py_loader.c
	${source_path}/py_loader_impl.c
	${source_path}/py_loader_port.c
	${source_path}/py_loader_threading.cpp
	${source_path}/py_loader_dict.c
	${source_path}/py_loader_symbol_fallback.c
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
	${Python3_INCLUDE_DIRS} # Python includes

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
	$<$<CXX_COMPILER_ID:MSVC>:${Python3_LIBRARY}> # Python library
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

#
# Linker options
#

get_filename_component(Python3_LIBRARY_NAME "${Python3_LIBRARY_NAME_PATH}" NAME)

target_link_options(${target}
	PRIVATE
	$<$<AND:$<BOOL:${APPLE}>,$<CXX_COMPILER_ID:AppleClang,Clang>>:-Wl,-undefined,dynamic_lookup>
	$<$<CXX_COMPILER_ID:MSVC>:/DELAYLOAD:${Python3_LIBRARY_NAME}>

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

# Runtime (pack Python DLL in windows)
# TODO: https://cmake.org/cmake/help/latest/command/file.html#get-runtime-dependencies
# TODO: https://gist.github.com/micahsnyder/5d98ac8548b429309ec5a35bca9366da
set(Python3_LIBRARY_DEVELOPMENT "${Python3_LIBRARY_NAME_PATH}")

if(Python3_LIBRARY_NAME_PATH AND PROJECT_OS_FAMILY STREQUAL win32)
	install(FILES
		"${Python3_LIBRARY_NAME_PATH}"
		DESTINATION ${INSTALL_LIB}
		COMPONENT runtime
	)

	set(Python3_LIBRARY_INSTALL "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB}/${Python3_LIBRARY_NAME}")
else()
	set(Python3_LIBRARY_INSTALL "${Python3_LIBRARY_NAME_PATH}")
endif()

#
# Configuration
#

# Development
loader_configuration_begin(py_loader)
loader_configuration_deps(python "${Python3_LIBRARY_DEVELOPMENT}")
loader_configuartion_end_development()

# Install
loader_configuration_begin(py_loader)
loader_configuration_deps(python "${Python3_LIBRARY_INSTALL}")
loader_configuartion_end_install()
