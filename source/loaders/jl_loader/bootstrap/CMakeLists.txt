# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_JL)
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target jl_loader_bootstrap)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target} bootstrap")

#
# Create target
#

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/lib")

set(sources
	${source_path}/bootstrap.jl
)

# Group source files
set(source_group "Source Files")
source_group_by_path(${source_path}  "\\\\.jl$"
	${source_group} ${sources})

add_custom_target(${target}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E  make_directory ${PROJECT_OUTPUT_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/bootstrap.jl ${PROJECT_OUTPUT_DIR}
	SOURCES ${sources}
	DEPENDS ${sources}
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
# Deployment
#

# Library
install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/lib/bootstrap.jl
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)
