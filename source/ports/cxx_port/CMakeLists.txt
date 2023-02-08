# Check if port is enabled
if(NOT OPTION_BUILD_PORTS_CXX)
	return()
endif()

#
# Port name and options
#

# Target name
set(target cxx_port)
string(TOLOWER ${META_PROJECT_NAME} target_name)

set(target_export "${META_PROJECT_NAME}-cxx")

# Exit here if required dependencies are not met
message(STATUS "Port ${target}")

# Set API export file and macro
string(TOUPPER ${target_name} target_name_upper)
set(export_file  "include/${target_name}/${target_name}_api.hpp")
set(export_macro "${target_name_upper}_API")

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

set(inline_path "${CMAKE_CURRENT_SOURCE_DIR}/inline/${target_name}")
set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/${target_name}")
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/source")

set(inlines
	${inline_path}/metacall.inl
)

set(headers
	${include_path}/metacall.hpp
)

set(sources
	${source_path}/metacall.cpp
)

# Group source files
set(inline_group "Inline Files")
set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${inline_path} "\\\\.inl$"
	${inline_group} ${inlines})
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$"
	${header_group} ${headers})
source_group_by_path(${source_path} "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$"
	${source_group} ${sources})

#
# Create library
#

# Build library
add_library(${target}
	${inlines}
	${sources}
	${headers}
)

# Create namespaced alias
add_library(${META_PROJECT_NAME}::${target} ALIAS ${target})

# Export library for downstream projects
export(TARGETS ${target} NAMESPACE ${META_PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target_name}/${target_export}-export.cmake)

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
	${CMAKE_CURRENT_SOURCE_DIR}/inline

	PUBLIC
	${DEFAULT_INCLUDE_DIRECTORIES}

	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

#
# Compile definitions
#

target_compile_definitions(${target}
	PRIVATE

	PUBLIC
	$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${target_name_upper}_STATIC_DEFINE>
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
	EXPORT  "${target_export}-export"		COMPONENT dev
	RUNTIME DESTINATION ${INSTALL_BIN}		COMPONENT runtime
	LIBRARY DESTINATION ${INSTALL_SHARED}	COMPONENT runtime
	ARCHIVE DESTINATION ${INSTALL_LIB}		COMPONENT dev
)

# Inline files
install(DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}/inline/${target_name} DESTINATION ${INSTALL_INCLUDE}
	COMPONENT dev
)

# Header files
install(DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}/include/${target_name} DESTINATION ${INSTALL_INCLUDE}
	COMPONENT dev
)

# Generated header files
install(DIRECTORY
	${CMAKE_CURRENT_BINARY_DIR}/include/${target_name} DESTINATION ${INSTALL_INCLUDE}
	COMPONENT dev
)

# CMake config
install(EXPORT	${target_export}-export
	NAMESPACE	${META_PROJECT_NAME}::
	DESTINATION	${INSTALL_CMAKE}/${target_name}
	COMPONENT	dev
)

# TODO

# #
# # Configure test
# #

# set(metacall_cxx_test "${target}_test")
# set(metacall_cxx_test_path "${CMAKE_CURRENT_BINARY_DIR}/${metacall_cxx_test}.cpp")

# #
# # Define test
# #

# add_test(NAME ${metacall_cxx_test}
# 	COMMAND $<TARGET_FILE:${metacall_cxx_test}>
# )

# #
# # Define test labels
# #

# set_property(TEST ${metacall_cxx_test}
# 	PROPERTY LABELS ${metacall_cxx_test}
# )

# include(TestEnvironmentVariables)

# test_environment_variables(${metacall_cxx_test}
# 	""
# 	${TESTS_ENVIRONMENT_VARIABLES}
# )
