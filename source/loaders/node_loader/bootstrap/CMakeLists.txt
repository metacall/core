# Check if this loader is enabled
if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_NODE)
	return()
endif()

#
# Plugin name and options
#

if(NOT OPTION_BUILD_GUIX)
	find_package(NPM)

	if(NOT NPM_FOUND)
		message(SEND_ERROR "NPM not found")
		return()
	endif()
endif()

# Target name
set(target node_loader_bootstrap)

# Exit here if required dependencies are not met
message(STATUS "Plugin ${target} bootstrap")

#
# Create target
#

set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/lib")

set(sources
	${source_path}/bootstrap.js
)

# Group source files
set(source_group "Source Files")
source_group_by_path(${source_path}  "\\\\.js$"
	${source_group} ${sources})

add_custom_target(${target}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_OUTPUT_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/bootstrap.js ${PROJECT_OUTPUT_DIR}
	SOURCES ${sources}
	DEPENDS ${sources}
)

if(NOT OPTION_BUILD_GUIX)
	add_custom_target(${target}_depends
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
		COMMAND ${CMAKE_COMMAND} -E echo "Installing ${target} dependencies"
		COMMAND ${NPM_EXECUTABLE} install
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/package.json ${CMAKE_CURRENT_SOURCE_DIR}/lib/package-lock.json
	)

	set_target_properties(${target}_depends
		PROPERTIES
		${DEFAULT_PROJECT_OPTIONS}
		FOLDER "${IDE_FOLDER}"
	)

	add_custom_target(${target}_copy_depends
		COMMAND ${CMAKE_COMMAND} -E echo "Copying ${target} dependencies"
		COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_OUTPUT_DIR}/node_modules
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/lib/node_modules/ ${PROJECT_OUTPUT_DIR}/node_modules
		COMMAND ${CMAKE_COMMAND} -E echo "${target} dependencies copied from ${CMAKE_CURRENT_SOURCE_DIR}/node_modules to ${PROJECT_OUTPUT_DIR}/node_modules"
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/package.json ${CMAKE_CURRENT_SOURCE_DIR}/lib/package-lock.json
	)

	set_target_properties(${target}_copy_depends
		PROPERTIES
		${DEFAULT_PROJECT_OPTIONS}
		FOLDER "${IDE_FOLDER}"
	)

	add_dependencies(${target}_copy_depends ${target}_depends)
	add_dependencies(${target} ${target}_depends ${target}_copy_depends)

	install(DIRECTORY
		${PROJECT_OUTPUT_DIR}/node_modules/espree
		${PROJECT_OUTPUT_DIR}/node_modules/acorn
		${PROJECT_OUTPUT_DIR}/node_modules/acorn-jsx
		${PROJECT_OUTPUT_DIR}/node_modules/eslint-visitor-keys
		DESTINATION ${INSTALL_LIB}/node_modules
		COMPONENT runtime
	)
else()
	# Guix stores espree dependency previously to the build in the output directory
	install(DIRECTORY
		${PROJECT_OUTPUT_DIR}/node_modules/espree
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
	${CMAKE_CURRENT_SOURCE_DIR}/lib/bootstrap.js
	DESTINATION ${INSTALL_LIB}
	COMPONENT runtime
)
