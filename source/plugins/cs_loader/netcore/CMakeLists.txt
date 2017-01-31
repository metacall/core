# Check if this plugin is enabled

if(NOT OPTION_BUILD_PLUGINS OR NOT OPTION_BUILD_PLUGINS_CS OR NOT OPTION_BUILD_PLUGINS_CS_IMP)
	return()
endif()

option(OPTION_BUILD_PLUGINS_CS_IMP		"Build Netcore library."	ON)
#
# External dependencies
#

find_package(DotNET)

if(NOT DOTNET_FOUND)
	message(STATUS "DotNET command not found")
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target cs_loader_impl)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target} implementation")

#
# Create target
#

add_custom_target(${target} ALL
	COMMAND dotnet restore ${CMAKE_CURRENT_SOURCE_DIR}/source/
	COMMAND dotnet publish ${CMAKE_CURRENT_SOURCE_DIR}/source/ -o ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/
)

#
# Project options
#

set_target_properties(${target}
	PROPERTIES
	${DEFAULT_PROJECT_OPTIONS}
	FOLDER "${IDE_FOLDER}"
)
