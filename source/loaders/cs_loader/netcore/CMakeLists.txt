# Check if this loader is enabled

if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_CS)
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

if(DOTNET_VERSION VERSION_EQUAL "2.0" OR DOTNET_VERSION VERSION_GREATER "2.0")
	add_custom_target(${target} ALL
		COMMAND dotnet restore ${CMAKE_CURRENT_SOURCE_DIR}/source/project.csproj
		COMMAND dotnet publish ${CMAKE_CURRENT_SOURCE_DIR}/source/project.csproj -o ${CMAKE_BINARY_DIR}
	)
else()
	if(DOTNET_MIGRATE)
		add_custom_target(${target} ALL
			COMMAND dotnet migrate ${CMAKE_CURRENT_SOURCE_DIR}/source/project.json
			COMMAND dotnet restore ${CMAKE_CURRENT_SOURCE_DIR}/source/project.json
			COMMAND dotnet publish ${CMAKE_CURRENT_SOURCE_DIR}/source/project.json -o ${CMAKE_BINARY_DIR}
		)
	else()
		add_custom_target(${target} ALL
			COMMAND dotnet restore ${CMAKE_CURRENT_SOURCE_DIR}/source/project.json
			COMMAND dotnet publish ${CMAKE_CURRENT_SOURCE_DIR}/source/project.json -o ${CMAKE_BINARY_DIR}
		)
	endif()
endif()

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

if(DOTNET_VERSION VERSION_EQUAL "2.0" OR DOTNET_VERSION VERSION_GREATER "2.0")
	set(DOTNET_INSTALL_DEPENDENCIES
		${CMAKE_BINARY_DIR}/Microsoft.CodeAnalysis.dll
		${CMAKE_BINARY_DIR}/Microsoft.CodeAnalysis.CSharp.dll)
else()
	set(DOTNET_INSTALL_DEPENDENCIES ${CMAKE_BINARY_DIR}/System.Runtime.Loader.dll)
endif()

# Library
install(FILES
	${CMAKE_BINARY_DIR}/CSLoader.dll
	${DOTNET_INSTALL_DEPENDENCIES}
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)
