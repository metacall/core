# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE OR DISABLE_BUILD_LOADERS_NODE_TRAMPOLINE)
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target node_loader_trampoline)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target} implementation")

#
# Create target
#

if(MSVC)
	set(LOADER_LIBRARY_PATH "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
	set(LOADER_LIBRARY_PATH "${PROJECT_BINARY_DIR}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(NODE_LOADER_TRAMPOLINE_BUILD_TYPE "--debug")
else()
	set(NODE_LOADER_TRAMPOLINE_BUILD_TYPE "--release")
endif()

if(MSVC)
	# TODO: Add multiple msvc versions
	if(MSVC_VERSION EQUAL 1900)
		set(NODE_LOADER_TRAMPOLINE_MSVS_VERSION 2015)
	elseif(MSVC_VERSION GREATER 1900)
		set(NODE_LOADER_TRAMPOLINE_MSVS_VERSION 2017)
	endif()

	add_custom_target(${target} ALL
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND npm config set msvs_version ${NODE_LOADER_TRAMPOLINE_MSVS_VERSION}
		COMMAND npm install --msvs_version=${NODE_LOADER_TRAMPOLINE_MSVS_VERSION}
		COMMAND node-gyp ${NODE_LOADER_TRAMPOLINE_BUILD_TYPE} rebuild --verbose
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/build/${CMAKE_BUILD_TYPE}/trampoline.node ${LOADER_LIBRARY_PATH}
	)

	set(target_binding "${target}_binding")

	include_external_msproject(${target_binding} ${CMAKE_CURRENT_BINARY_DIR}/build/trampoline.vcxproj)

	set_target_properties(${target_binding}
		PROPERTIES
		${DEFAULT_PROJECT_OPTIONS}
		FOLDER "${IDE_FOLDER}"
	)

	add_dependencies(${target_binding} ${target})
else()
	add_custom_target(${target} ALL
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND npm install
		COMMAND node-gyp ${NODE_LOADER_TRAMPOLINE_BUILD_TYPE} rebuild --verbose
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/build/${CMAKE_BUILD_TYPE}/trampoline.node ${LOADER_LIBRARY_PATH}
	)
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

# Library
install(FILES
	${LOADER_LIBRARY_PATH}/trampoline.node
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)
