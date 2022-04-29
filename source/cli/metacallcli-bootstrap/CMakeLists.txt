# Check if this loader dependencies are enabled
# TODO: Add OPTION_BUILD_LOADERS_NODE or any other else when needed
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_C OR NOT OPTION_BUILD_LOADERS_NODE)
	return()
endif()

# Check if building this refactor is enabled
if(NOT OPTION_BUILD_METACALL_BOOTSTRAP)
	return()
endif()

# Executable name and options
#

# Target name
set(target metacallcli-bootstrap) # TODO: Rename to metacallcli when the refactor is done

# Exit here if required dependencies are not met
message(STATUS "CLI ${target}")

# Add cli_plugins
add_subdirectory(cli_plugins) #add plugin directory

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

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include/metacallcli") #include folder not "metacallcli-bootstrap"${target}
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}/source")

#TODO: reorganise directory structure
set(headers
	${include_path}/cli_plugin.hpp
	#${include_path}/cli_option.hpp
	${include_path}/cli.hpp
)

set(sources
	${source_path}/cli_plugin.cpp
	${source_path}/cli_options.cpp
	${source_path}/main.cpp
	${source_path}/cli.cpp
)

#
# Create executable
#

# Build executable
add_executable(${target}
	MACOSX_BUNDLE
	${sources}
)

# Create namespaced alias
add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})

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
	${DEFAULT_INCLUDE_DIRECTORIES}

	PUBLIC

	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	$<INSTALL_INTERFACE:include>
)


# Add target dependencies
add_dependencies(${target}
	cli_plugins
)

#
# Libraries
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LIBRARIES}
	${META_PROJECT_NAME}::metacall
	${META_PROJECT_NAME}::reflect
	${META_PROJECT_NAME}::dynlink
	${META_PROJECT_NAME}::environment
	${META_PROJECT_NAME}::portability
	${META_PROJECT_NAME}::preprocessor
	${META_PROJECT_NAME}::log
	${META_PROJECT_NAME}::threading
	${META_PROJECT_NAME}::format
)

#
# Compile definitions
#

target_compile_definitions(${target}
	PRIVATE
	${DEFAULT_COMPILE_DEFINITIONS}
)

#
# Compile options
#

target_compile_options(${target}
	PRIVATE
	${DEFAULT_COMPILE_OPTIONS}
)

#
# Linker options
#

target_link_libraries(${target}
	PRIVATE
	${DEFAULT_LINKER_OPTIONS}
)

#
# Deployment
#

# Executable
install(TARGETS ${target}
	RUNTIME DESTINATION ${INSTALL_BIN} COMPONENT cli
	BUNDLE  DESTINATION ${INSTALL_BIN} COMPONENT cli
)

# TODO: Add all tests

set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
