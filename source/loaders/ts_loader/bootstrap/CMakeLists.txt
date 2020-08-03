# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE OR NOT OPTION_BUILD_LOADERS_TS)
	return()
endif()

#
# Plugin name and options
#

# Target name
set(target ts_loader_bootstrap)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target} bootstrap")

#
# Create target
#

if(MSVC)
	set(BOOTSTRAP_OUTPUT_PATH "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
	set(BOOTSTRAP_OUTPUT_PATH "${PROJECT_BINARY_DIR}")
endif()

add_custom_target(${target} ALL
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/bootstrap.ts ${BOOTSTRAP_OUTPUT_PATH}
)

if(NOT OPTION_BUILD_GUIX)
	add_custom_target(${target}_depends ALL
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/package.json ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/package-lock.json ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND npm --prefix ${CMAKE_CURRENT_BINARY_DIR} install ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/node_modules/typescript ${BOOTSTRAP_OUTPUT_PATH}/node_modules/typescript
		COMMAND ${CMAKE_COMMAND} -E echo "${target} dependencies copied from ${CMAKE_CURRENT_BINARY_DIR}/node_modules to ${BOOTSTRAP_OUTPUT_PATH}/node_modules"
	)

	set_target_properties(${target}_depends
		PROPERTIES
		${DEFAULT_PROJECT_OPTIONS}
		FOLDER "${IDE_FOLDER}"
	)

	add_dependencies(${target} ${target}_depends)

	install(DIRECTORY
		${CMAKE_CURRENT_BINARY_DIR}/node_modules/typescript
		DESTINATION ${INSTALL_LIB}
		COMPONENT runtime
	)
else()
	# Guix stores typescript dependency previously to the build in the output directory
	install(DIRECTORY
		${BOOTSTRAP_OUTPUT_PATH}/node_modules/typescript
		DESTINATION ${INSTALL_LIB}/node_modules
		COMPONENT runtime
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
	${CMAKE_CURRENT_SOURCE_DIR}/lib/bootstrap.ts
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)
