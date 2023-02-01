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

# Apparently solves NetCore "error : Unable to load the service index for source https://api.nuget.org/v3/index.json." bug
# https://github.com/NuGet/Home/issues/5358
# https://stackoverflow.com/a/57894492
set(DOTNET_ENV_VAR ${CMAKE_COMMAND} -E env DOTNET_SYSTEM_NET_HTTP_USESOCKETSHTTPHANDLER=0)

if(OPTION_BUILD_GUIX)
	if(DOTNET_VERSION VERSION_EQUAL "2.0" OR DOTNET_VERSION VERSION_GREATER "2.0")
		execute_process(
			COMMAND sh -c "${DOTNET_COMMAND} nuget locals all --list | grep global-packages | awk '{print $NF}'"
			OUTPUT_VARIABLE DOTNET_SOURCE
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		set(DOTNET_SOURCE --source ${DOTNET_SOURCE})
	else()
		set(DOTNET_SOURCE)
		message(WARNING ".NET Core support not implemented in Guix build for versions less than 2.0.")
	endif()

	set(DOTNET_ADDITIONAL_PACKAGES "" CACHE PATH "Define a folder for additional packages in NetCore.")

	if(DOTNET_ADDITIONAL_PACKAGES)
		set(DOTNET_ADDITIONAL_PACKAGES_SOURCE --source ${DOTNET_ADDITIONAL_PACKAGES})
	else()
		set(DOTNET_ADDITIONAL_PACKAGES_SOURCE)
	endif()

	if(DOTNET_CORE_PATH)
		set(DOTNET_CORE_PATH_SOURCE --source ${DOTNET_CORE_PATH})
	else()
		set(DOTNET_CORE_PATH_SOURCE)
	endif()

	# Build without internet access
	add_custom_target(${target} ALL
		COMMAND ${CMAKE_COMMAND} -E $<IF:$<VERSION_LESS:CMAKE_VERSION,3.17>,remove,rm> ${PROJECT_OUTPUT_DIR}/CSLoader.dll
		COMMAND ${DOTNET_COMMAND} restore ${DOTNET_CORE_PATH_SOURCE} ${DOTNET_ADDITIONAL_PACKAGES_SOURCE} ${DOTNET_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/source/project.csproj
		COMMAND ${DOTNET_COMMAND} publish ${DOTNET_CORE_PATH_SOURCE} ${DOTNET_ADDITIONAL_PACKAGES_SOURCE} ${DOTNET_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/source/project.csproj -o ${CMAKE_BINARY_DIR}
	)
else()
	if(DOTNET_VERSION VERSION_EQUAL "2.0" OR DOTNET_VERSION VERSION_GREATER "2.0")
		add_custom_target(${target} ALL
			COMMAND ${CMAKE_COMMAND} -E $<IF:$<VERSION_LESS:CMAKE_VERSION,3.17>,remove,rm> ${PROJECT_OUTPUT_DIR}/CSLoader.dll
			COMMAND ${DOTNET_ENV_VAR} ${DOTNET_COMMAND} restore ${CMAKE_CURRENT_SOURCE_DIR}/source/project.csproj
			COMMAND ${DOTNET_ENV_VAR} ${DOTNET_COMMAND} publish ${CMAKE_CURRENT_SOURCE_DIR}/source/project.csproj -o ${PROJECT_OUTPUT_DIR}
		)
	else()
		if(DOTNET_MIGRATE)
			add_custom_target(${target} ALL
				COMMAND ${CMAKE_COMMAND} -E $<IF:$<VERSION_LESS:CMAKE_VERSION,3.17>,remove,rm> ${PROJECT_OUTPUT_DIR}/CSLoader.dll
				COMMAND ${DOTNET_ENV_VAR} ${DOTNET_COMMAND} migrate ${CMAKE_CURRENT_SOURCE_DIR}/source/project.json
				COMMAND ${DOTNET_ENV_VAR} ${DOTNET_COMMAND} restore ${CMAKE_CURRENT_SOURCE_DIR}/source/project.json
				COMMAND ${DOTNET_ENV_VAR} ${DOTNET_COMMAND} publish ${CMAKE_CURRENT_SOURCE_DIR}/source/project.json -o ${PROJECT_OUTPUT_DIR}
			)
		else()
			add_custom_target(${target} ALL
				COMMAND ${CMAKE_COMMAND} -E $<IF:$<VERSION_LESS:CMAKE_VERSION,3.17>,remove,rm> ${PROJECT_OUTPUT_DIR}/CSLoader.dll
				COMMAND ${DOTNET_ENV_VAR} ${DOTNET_COMMAND} restore ${CMAKE_CURRENT_SOURCE_DIR}/source/project.json
				COMMAND ${DOTNET_ENV_VAR} ${DOTNET_COMMAND} publish ${CMAKE_CURRENT_SOURCE_DIR}/source/project.json -o ${PROJECT_OUTPUT_DIR}
			)
		endif()
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

if(DOTNET_VERSION VERSION_EQUAL "5.0" OR DOTNET_VERSION VERSION_GREATER "5.0")
	set(DOTNET_INSTALL_DEPENDENCIES
		${PROJECT_OUTPUT_DIR}/Microsoft.CodeAnalysis.dll
		${PROJECT_OUTPUT_DIR}/Microsoft.CodeAnalysis.CSharp.dll
	)
elseif(DOTNET_VERSION VERSION_EQUAL "2.0" OR DOTNET_VERSION VERSION_GREATER "2.0")
	set(DOTNET_INSTALL_DEPENDENCIES
		${PROJECT_OUTPUT_DIR}/Microsoft.CodeAnalysis.dll
		${PROJECT_OUTPUT_DIR}/Microsoft.CodeAnalysis.CSharp.dll
		${PROJECT_OUTPUT_DIR}/System.Runtime.CompilerServices.Unsafe.dll
		${PROJECT_OUTPUT_DIR}/System.Text.Encoding.CodePages.dll
	)
else()
	set(DOTNET_INSTALL_DEPENDENCIES ${PROJECT_OUTPUT_DIR}/System.Runtime.Loader.dll)
endif()

# Library
install(FILES
	${PROJECT_OUTPUT_DIR}/CSLoader.dll
	${DOTNET_INSTALL_DEPENDENCIES}
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)
