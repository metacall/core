# Check if this loader is enabled

if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_CS OR NOT OPTION_BUILD_LOADERS_CS_IMPL)
	return()
endif()

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
	COMMAND dotnet migrate ${CMAKE_CURRENT_SOURCE_DIR}/source/
	COMMAND dotnet restore ${CMAKE_CURRENT_SOURCE_DIR}/source/
	COMMAND dotnet publish ${CMAKE_CURRENT_SOURCE_DIR}/source/ -o ${CMAKE_BINARY_DIR}
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
	${CMAKE_BINARY_DIR}/CSLoader.dll
	${CMAKE_BINARY_DIR}/System.Runtime.Loader.dll
	DESTINATION ${INSTALL_LIB}
)
