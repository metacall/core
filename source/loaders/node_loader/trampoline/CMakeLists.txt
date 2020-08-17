# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE)
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

	if(OPTION_BUILD_GUIX)
		add_custom_target(${target} ALL
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND npm config set msvs_version ${NODE_LOADER_TRAMPOLINE_MSVS_VERSION}
			# TODO: Implement build with gyp
		)
	else()
		add_custom_target(${target} ALL
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND npm config set msvs_version ${NODE_LOADER_TRAMPOLINE_MSVS_VERSION}
			COMMAND npm install --msvs_version=${NODE_LOADER_TRAMPOLINE_MSVS_VERSION}
			COMMAND node-gyp ${NODE_LOADER_TRAMPOLINE_BUILD_TYPE} rebuild --verbose
			COMMAND ${CMAKE_COMMAND} -E  make_directory ${LOADER_LIBRARY_PATH}
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/build/${CMAKE_BUILD_TYPE}/trampoline.node ${LOADER_LIBRARY_PATH}
		)
	endif()

	set(target_binding "${target}_binding")

	include_external_msproject(${target_binding} ${CMAKE_CURRENT_BINARY_DIR}/build/trampoline.vcxproj)

	set_target_properties(${target_binding}
		PROPERTIES
		${DEFAULT_PROJECT_OPTIONS}
		FOLDER "${IDE_FOLDER}"
	)

	add_dependencies(${target_binding} ${target})
else()
	if(OPTION_BUILD_GUIX)
		find_package(NodeJSAddonAPI)

		if(NOT NODEJS_ADDON_API_FOUND)
			message(STATUS "NodeJS Addon API not found")
			return()
		endif()

		# Generate additional gyp file for the addon
		set(NODEJS_ADDON_INCLUDES
			"{
				'target_defaults': {
					'cflags': [
						'-fPIC'
					],
					'include_dirs': [
						'@NODEJS_ADDON_API_INCLUDE_DIR@/external-napi'
					]
				}
			}"
		)

		string(REPLACE "@NODEJS_ADDON_API_INCLUDE_DIR@" "${NODEJS_ADDON_API_INCLUDE_DIR}" NODEJS_ADDON_INCLUDES ${NODEJS_ADDON_INCLUDES})

		file(WRITE binding.gypi "${NODEJS_ADDON_INCLUDES}")

		add_custom_target(${target} ALL
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND gyp binding.gyp -f make --depth . -Ibinding.gypi # TODO: ${NODE_LOADER_TRAMPOLINE_BUILD_TYPE}
			COMMAND make -f binding.Makefile
			COMMAND ${CMAKE_COMMAND} -E  make_directory ${LOADER_LIBRARY_PATH}
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/out/Default/lib.target/libtrampoline.so ${LOADER_LIBRARY_PATH}/trampoline.node
		)
	else()
		add_custom_target(${target} ALL
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND npm install
			COMMAND node-gyp ${NODE_LOADER_TRAMPOLINE_BUILD_TYPE} rebuild --verbose
			COMMAND ${CMAKE_COMMAND} -E  make_directory ${LOADER_LIBRARY_PATH}
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/build/${CMAKE_BUILD_TYPE}/obj.target/trampoline.so ${LOADER_LIBRARY_PATH}/trampoline.node
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

# Library
install(FILES
	${LOADER_LIBRARY_PATH}/trampoline.node
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)
